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

#include <qbutton.h>
#include <qbitmap.h>
#include <qdatetime.h>
#include <kpixmap.h>
#include <kdecoration.h>
#include <kdecorationfactory.h>


class QSpacerItem;
class QBoxLayout;
class QGridLayout;
class QHBoxLayout;

namespace BlueCurve {

class BlueCurveClient;

class BlueCurveHandler: public KDecorationFactory
{
	public:
		BlueCurveHandler();
		~BlueCurveHandler();
		KDecoration* createDecoration( KDecorationBridge* );
		bool reset(unsigned long changed);
		//virtual QValueList< BorderSize > borderSizes() const;

	private:
		void readConfig();
		void createPixmaps();
		void freePixmaps();
		void drawButtonBackground(KPixmap *pix, 
			const QColorGroup &g,
			bool sunken,
			bool active);
		void recolor( QImage &img, const QColor& color );
};

enum ButtonPos { ButtonLeft = 0, ButtonMid, ButtonRight, LeftButtonRight };

class BlueCurveButton : public QButton, public KDecorationDefines
{
	public:
		BlueCurveButton( BlueCurveClient *parent=0, const char *name=0,
			bool largeButton=true, int pos=ButtonMid,
			bool isOnAllDesktopsButton=false, const unsigned char *bitmap=NULL,
			const QString& tip=NULL, const int realizeBtns=LeftButton );
		~BlueCurveButton(); 

		int last_button;
		void turnOn( bool isOn );
		void setBitmap(const unsigned char *bitmap);
		void setTipText(const QString &tip);
		QSize sizeHint() const;
		void reset();
		int pos;

	protected:
		void resizeEvent( QResizeEvent* e);
		void showEvent(QShowEvent *ev);
		void doShape();
		void enterEvent(QEvent *);
		void leaveEvent(QEvent *);
		void mousePressEvent( QMouseEvent* e );
		void mouseReleaseEvent( QMouseEvent* e );
		void drawButton(QPainter *p);
		void drawButtonLabel(QPainter*) {;}

		QBitmap* deco;
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
		virtual void resizeEvent( QResizeEvent* );
		virtual void paintEvent( QPaintEvent* );
		virtual void showEvent( QShowEvent* );
		virtual void mouseDoubleClickEvent( QMouseEvent * );

		virtual void doShape();
		virtual void borders( int&, int&, int&, int& ) const;
		virtual void resize(const QSize&);
		virtual void captionChange();
		virtual void maximizeChange();
		virtual void shadeChange();
		virtual bool isTool() const;

		virtual void activeChange();
		virtual void iconChange();
		virtual void desktopChange();
		virtual QSize minimumSize() const;
		virtual Position mousePosition(const QPoint &) const;

	protected slots:
		void slotMaximize();
		void menuButtonPressed();
		void menuButtonReleased();

	private:
		bool eventFilter( QObject* o, QEvent* e );
		void calcHiddenButtons();
		void addClientButtons( const QString& s, bool isLeft=true );

		enum Buttons{ BtnHelp=0, BtnMax, BtnIconify, BtnClose,
			BtnMenu, BtnOnAllDesktops, BtnCount };
		BlueCurveButton* button[ BlueCurveClient::BtnCount ];

		int           lastButtonWidth;
		int           titleHeight;
		bool          largeButtons;
		QGridLayout*  g;
		QHBoxLayout*  hb;
		QSpacerItem*  titlebar;
		QSpacerItem*  spacer;
		bool          m_closing;
};

}

#endif
// vim: ts=4
