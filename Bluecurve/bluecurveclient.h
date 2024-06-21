/*
 *	$Id: bluecurveclient.h,v 1.2 2004/01/26 12:52:48 than Exp $
 *
 *	BlueCurve KWin client
 *
 *	Copyright (C) 1999, 2001 Daniel Duley <mosfet@kde.org>
 *	Matthias Ettrich <ettrich@kde.org>
 *	Karol Szwed <gallium@kde.org>
 *  Than Ngo than@redhat.com
 *
 *	Draws mini titlebars for tool windows.
 *	Many features are now customizable.
 */

#ifndef _BLUECURVE_H
#define _BLUECURVE_H

#include <tqbutton.h>
#include <tqbitmap.h>
#include <tqdatetime.h>
#include <kpixmap.h>
#include <kdecoration.h>
#include <kdecorationfactory.h>


class TQSpacerItem;
class TQBoxLayout;
class TQGridLayout;
class TQHBoxLayout;

namespace BlueCurve {

class BlueCurveClient;

class BlueCurveHandler: public KDecorationFactory
{
	public:
		BlueCurveHandler();
		~BlueCurveHandler();
		KDecoration* createDecoration( KDecorationBridge* );
		bool reset(unsigned long changed);
		//virtual TQValueList< BorderSize > borderSizes() const;

	private:
		void readConfig();
		void createPixmaps();
		void freePixmaps();
		void drawButtonBackground(KPixmap *pix, 
			const TQColorGroup &g,
			bool sunken,
			bool active);
		void recolor( TQImage &img, const TQColor& color );
};

enum ButtonPos { ButtonLeft = 0, ButtonMid, ButtonRight, LeftButtonRight };

class BlueCurveButton : public TQButton, public KDecorationDefines
{
	public:
		BlueCurveButton( BlueCurveClient *parent=0, const char *name=0,
			bool largeButton=true, int pos=ButtonMid,
			bool isOnAllDesktopsButton=false, const unsigned char *bitmap=NULL,
			const TQString& tip=NULL, const int realizeBtns=LeftButton );
		~BlueCurveButton(); 

		int last_button;
		void turnOn( bool isOn );
		void setBitmap(const unsigned char *bitmap);
		void setTipText(const TQString &tip);
		TQSize sizeHint() const;
		void reset();
		int pos;

	protected:
		void resizeEvent( TQResizeEvent* e);
		void showEvent(TQShowEvent *ev);
		void doShape();
		void enterEvent(TQEvent *);
		void leaveEvent(TQEvent *);
		void mousePressEvent( TQMouseEvent* e );
		void mouseReleaseEvent( TQMouseEvent* e );
		void drawButton(TQPainter *p);
		void drawButtonLabel(TQPainter*) {;}

		TQBitmap* deco;
		bool large;
		bool isLeft;
		bool isOnAllDesktops;
		bool isMouseOver;
		BlueCurveClient* client;

		int realizeButtons;
};


class BlueCurveClient : public KDecoration
{
	Q_OBJECT

	public:
		BlueCurveClient( KDecorationBridge* bridge, KDecorationFactory* factory );
		~BlueCurveClient() {;}

		virtual void init();

	protected:
		virtual void resizeEvent( TQResizeEvent* );
		virtual void paintEvent( TQPaintEvent* );
		virtual void showEvent( TQShowEvent* );
		virtual void mouseDoubleClickEvent( TQMouseEvent * );

		virtual void doShape();
		virtual void borders( int&, int&, int&, int& ) const;
		virtual void resize(const TQSize&);
		virtual void captionChange();
		virtual void maximizeChange();
		virtual void shadeChange();
		virtual bool isTool() const;

		virtual void activeChange();
		virtual void iconChange();
		virtual void desktopChange();
		virtual TQSize minimumSize() const;
		virtual Position mousePosition(const TQPoint &) const;

	protected slots:
		void slotMaximize();
		void menuButtonPressed();
		void menuButtonReleased();

	private:
		bool eventFilter( TQObject* o, TQEvent* e );
		void calcHiddenButtons();
		void addClientButtons( const TQString& s, bool isLeft=true );

		enum Buttons{ BtnHelp=0, BtnMax, BtnIconify, BtnClose,
			BtnMenu, BtnOnAllDesktops, BtnCount };
		BlueCurveButton* button[ BlueCurveClient::BtnCount ];

		int           lastButtonWidth;
		int           titleHeight;
		bool          largeButtons;
		TQGridLayout*  g;
		TQHBoxLayout*  hb;
		TQSpacerItem*  titlebar;
		TQSpacerItem*  spacer;
		bool          m_closing;
};

}

#endif
// vim: ts=4
