/*
 *	$Id: bluecurveclient.cpp,v 1.4 2006/03/16 18:03:36 than Exp $
 *
 *	BlueCurve KWin client
 *
 *	Copyright (C) 1999, 2001 Daniel Duley <mosfet@kde.org>
 *	Matthias Ettrich <ettrich@kde.org>
 *	Karol Szwed <gallium@kde.org>
 *  Than Ngo <than@redhat.com>
 *
 *  Ported to TDE by neeeeow <https://github.com/neeeeow/twin-style-bluecurve>
 *
 *	Draws mini titlebars for tool windows.
 *	Many features are now customizable.
 */

#include "bluecurveclient.h"

#include <kconfig.h>
#include <kglobal.h>
#include <kpixmapeffect.h>
#include <kimageeffect.h>
#include <kdrawutil.h>
#include <klocale.h>
#include <tqlayout.h>
#include <tqdrawutil.h>
#include <tqbitmap.h>
#include <tqimage.h>
#include <tqtooltip.h>
#include <tqapplication.h>
#include <tqlabel.h>
#include <kdebug.h>


#define BASE_BUTTON_SIZE  17
#define BORDER_WIDTH      6
#define CORNER_RADIUS     12

#define BUTTON_DIAM       12
#define TOP_GRABBAR_WIDTH 2
#define BOTTOM_CORNER     5


namespace BlueCurve
{
#include "bitmaps.h"

KPixmap* titlePix;
KPixmap* titleBuffer;
KPixmap* aUpperGradient;
KPixmap* iUpperGradient;

KPixmap* pinDownPix;
KPixmap* pinUpPix;
KPixmap* ipinDownPix;
KPixmap* ipinUpPix;

KPixmap* btnUpPix;
KPixmap* btnDownPix;
KPixmap* ibtnUpPix;
KPixmap* ibtnDownPix;

TQPixmap* bottomLeftPix;
TQPixmap* bottomRightPix;
TQPixmap* abottomLeftPix;
TQPixmap* abottomRightPix;

BlueCurveHandler* clientHandler;

bool BlueCurve_initialized = false;
bool useGradients;
bool showGrabBar;
bool showTitleBarStipple;
bool largeToolButtons;

static int grabBorderWidth;
static int borderWidth;
static int toolTitleHeight;
static int normalTitleHeight;

BlueCurveHandler::BlueCurveHandler()
{
	readConfig();
	createPixmaps();
	BlueCurve_initialized = true;
}


BlueCurveHandler::~BlueCurveHandler()
{
	BlueCurve_initialized = false;
	freePixmaps();
}


KDecoration* BlueCurveHandler::createDecoration( KDecorationBridge* bridge )
{
	return new BlueCurveClient( bridge, this );
}


bool BlueCurveHandler::reset(unsigned long changed)
{
	BlueCurve_initialized = false;
	freePixmaps();
	readConfig();
	createPixmaps();
	BlueCurve_initialized = true;
	bool needHardReset = true;
	if (changed & SettingColors)
		needHardReset = false;

	if (needHardReset) {
		return true;
	} else {
		resetDecorations(changed);
		return false;
	}

	return true;
}


void BlueCurveHandler::readConfig()
{
	KConfig* conf = KGlobal::config();
	conf->setGroup("BlueCurve");

	showGrabBar = conf->readBoolEntry("ShowGrabBar", true);
	showTitleBarStipple = conf->readBoolEntry("ShowTitleBarStipple", true);
	useGradients = conf->readBoolEntry("UseGradients", true);
	int size = conf->readNumEntry("TitleBarSize", 0);

	if (size < 0) size = 0;
	if (size > 2) size = 2;

	normalTitleHeight = BASE_BUTTON_SIZE + (4*size);
	toolTitleHeight = normalTitleHeight - 4;
	largeToolButtons = (toolTitleHeight >= 16) ? true : false;

	int new_borderWidth;
	switch(options()->preferredBorderSize(this)) {
	case BorderLarge:
		new_borderWidth = 8;
		break;
	case BorderVeryLarge:
		new_borderWidth = 12;
		break;
	case BorderHuge:
		new_borderWidth = 18;
		break;
	case BorderVeryHuge:
		new_borderWidth = 27;
		break;
	case BorderOversized:
		new_borderWidth = 40;
		break;
	case BorderTiny:
	case BorderNormal:
	default:
		new_borderWidth = 4;
	}

	borderWidth = new_borderWidth;
	grabBorderWidth = (borderWidth > 15) ? borderWidth + 15 : 2*borderWidth;
}


// This paints the button pixmaps upon loading the style.
void BlueCurveHandler::createPixmaps()
{
	// Make the titlebar stipple optional
	if (showTitleBarStipple)
	{
		TQPainter p;
		TQPainter maskPainter;
		int x, y;
		titlePix = new KPixmap();
		titlePix->resize(132, normalTitleHeight+2);
		TQBitmap mask(132, normalTitleHeight+2);

		mask.fill(TQt::color0);

		p.begin(titlePix);
		maskPainter.begin(&mask);
		maskPainter.setPen(TQt::color1);

		TQColor lighterColor(options()->color(ColorTitleBar, true).light (150));
		int h, s, v;
		lighterColor.hsv (&h, &s, &v);
		s /= 2;
		s = (s > 255) ? 255 : (int) s;
		TQColor satColor(h, s, v, TQColor::Hsv);

		KPixmapEffect::gradient(*titlePix,
				satColor,
				satColor.dark(150),
				KPixmapEffect::VerticalGradient);

		for(y = 0; y < (normalTitleHeight+2); y++) {
			for(x = (3 - y) % 4; x < 132; x += 4) {
				maskPainter.drawPoint(x, y);
			}
		}

		maskPainter.end();
		p.end();
		titlePix->setMask(mask);
	} else
		titlePix = NULL;

	// Create titlebar gradient images if required
	aUpperGradient = NULL;
	iUpperGradient = NULL;

	// Set the sticky pin pixmaps;
	TQColorGroup g;
	TQPainter p;

	// Active pins
	g = options()->colorGroup( ColorButtonBg, true );

	pinUpPix = new KPixmap();
	pinUpPix->resize(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);
	p.begin( pinUpPix );
	kColorBitmaps( &p, g, 0, 0, BASE_BUTTON_SIZE, BASE_BUTTON_SIZE, true, pinup_white_bits,
		pinup_gray_bits, NULL, NULL, pinup_dgray_bits, NULL );
	p.end();
	pinUpPix->setMask( TQBitmap(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE, pinup_mask_bits, true) );

	pinDownPix = new KPixmap();
	pinDownPix->resize(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);
	p.begin( pinDownPix );
	kColorBitmaps( &p, g, 0, 0, BASE_BUTTON_SIZE, BASE_BUTTON_SIZE, true, pindown_white_bits,
		pindown_gray_bits, NULL, NULL, pindown_dgray_bits, NULL );
	p.end();
	pinDownPix->setMask( TQBitmap(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE, pindown_mask_bits, true) );

	// Inactive pins
	g = options()->colorGroup( ColorButtonBg, false );

	ipinUpPix = new KPixmap();
	ipinUpPix->resize(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);
	p.begin( ipinUpPix );
	kColorBitmaps( &p, g, 0, 0, BASE_BUTTON_SIZE, BASE_BUTTON_SIZE, true, pinup_white_bits,
		pinup_gray_bits, NULL, NULL, pinup_dgray_bits, NULL );
	p.end();
	ipinUpPix->setMask( TQBitmap(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE, pinup_mask_bits, true) );

	ipinDownPix = new KPixmap();
	ipinDownPix->resize(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);
	p.begin( ipinDownPix );
	kColorBitmaps( &p, g, 0, 0, BASE_BUTTON_SIZE, BASE_BUTTON_SIZE, true, pindown_white_bits,
		pindown_gray_bits, NULL, NULL, pindown_dgray_bits, NULL );
	p.end();
	ipinDownPix->setMask( TQBitmap(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE, pindown_mask_bits, true) );

	// Create a title buffer for flicker-free painting
	titleBuffer = new KPixmap();

	// Cache all possible button states

	btnUpPix = new KPixmap();
	btnUpPix->resize(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);
	btnDownPix = new KPixmap();
	btnDownPix->resize(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);
	ibtnUpPix = new KPixmap();
	ibtnUpPix->resize(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);
	ibtnDownPix = new KPixmap();
	ibtnDownPix->resize(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);

	// Draw the button state pixmaps
	g = options()->colorGroup( ColorButtonBg, true );

	drawButtonBackground( btnUpPix, g, false, true );
	drawButtonBackground( btnDownPix, g, true, true );

	g = options()->colorGroup( ColorButtonBg, false );

	drawButtonBackground( ibtnUpPix, g, false, false );
	drawButtonBackground( ibtnDownPix, g, true, false );

	TQImage bottomleft(bottom_left_xpm);
	TQImage bottomright(bottom_right_xpm);
	TQImage abottomleft(bottom_left_xpm);
	TQImage abottomright(bottom_right_xpm);

	recolor(bottomleft, options()->color( ColorTitleBar, false ).light(95));
	recolor(bottomright, options()->color( ColorTitleBar, false ).light(95));
	recolor(abottomleft, options()->color( ColorTitleBar, true ).light(135));
	recolor(abottomright, options()->color( ColorTitleBar, true ).light(135));

	bottomLeftPix 	= new TQPixmap();
	bottomRightPix	= new TQPixmap();
	abottomLeftPix	= new TQPixmap();	
	abottomRightPix	= new TQPixmap();
	bottomLeftPix->convertFromImage(bottomleft);
	bottomRightPix->convertFromImage(bottomright);
	abottomLeftPix->convertFromImage(abottomleft);
	abottomRightPix->convertFromImage(abottomright);
}

// This is the recoloring method from the Keramik widget style,
// copyright (c) 2002 Malte Starostik <malte@kde.org>.
// Modified to work with 8bpp images.
void BlueCurveHandler::recolor( TQImage &img, const TQColor& color )
{
	int hue = -1, sat = 0, val = 228;
	if ( color.isValid() )
		color.hsv( &hue, &sat, &val );
	register int pixels = (img.depth() > 8 ? img.width() * img.height() : img.numColors());
	register TQ_UINT32* data = ( img.depth() > 8 ? reinterpret_cast< TQ_UINT32* >( img.bits() ) :
		reinterpret_cast< TQ_UINT32* >( img.colorTable() ) );
	
	for ( int i = 0; i < pixels; i++ )
	{
		TQColor c( *data );
		int h, s, v;
		c.hsv( &h, &s, &v );
		h = hue;
		s = sat;
		v = v * val / 145;
		c.setHsv( h, TQMIN( s, 255 ), TQMIN( v, 255 ) );
		*data = ( c.rgb() & TQT_RGB_MASK ) | ( *data & ~TQT_RGB_MASK );
		data++;
	}
}


void BlueCurveHandler::freePixmaps()
{
	// Free button pixmaps
	if (btnUpPix)
		delete btnUpPix;
	if(btnDownPix)
		delete btnDownPix;
	if (ibtnUpPix)
		delete ibtnUpPix;
	if (ibtnDownPix)
		delete ibtnDownPix;

	// Title images
	if (titleBuffer)
		delete titleBuffer;
	if (titlePix)
		delete titlePix;
	if (aUpperGradient)
		delete aUpperGradient;
	if (iUpperGradient)
		delete iUpperGradient;

	// Sticky pin images
	if (pinUpPix)
		delete pinUpPix;
	if (ipinUpPix)
		delete ipinUpPix;
	if (pinDownPix)
		delete pinDownPix;
	if (ipinDownPix)
		delete ipinDownPix;
}


void BlueCurveHandler::drawButtonBackground(KPixmap *pix, 
		const TQColorGroup &g, bool sunken, bool active)
{
	TQPainter p;

	bool highcolor = useGradients && (TQPixmap::defaultDepth() > 8);
	TQColor c = g.background();

	// Fill the background with a gradient if possible
	if (highcolor)
	{
		if (active)
		{
			KPixmapEffect::gradient(*pix, c, TQt::white,
				KPixmapEffect::DiagonalGradient);
		} else
		{
			TQColor inactiveTitleColor1(options()->color(ColorTitleBar, false));
			TQColor inactiveTitleColor2(options()->color(ColorTitleBlend, false));
			KPixmapEffect::gradient(*pix,
				inactiveTitleColor2,
				inactiveTitleColor1,
				KPixmapEffect::VerticalGradient);
		}
	} else
		pix->fill(c);

	p.begin(pix);
	p.setPen(sunken ? g.dark() : g.mid());
}


BlueCurveButton::BlueCurveButton(BlueCurveClient *parent, const char *name,
		bool largeButton, int bpos, bool isOnAllDesktopsButton,
		const unsigned char *bitmap, const TQString& tip, const int realizeBtns)
		: TQButton(parent->widget(), name)
{
	realizeButtons = realizeBtns;
	setBackgroundMode( TQWidget::NoBackground );
	setToggleButton( isOnAllDesktopsButton );

	isMouseOver = false;
	deco = NULL;
	large = largeButton;
	isOnAllDesktops = isOnAllDesktopsButton;
	client = parent;

	pos = bpos;

	setFixedSize(BASE_BUTTON_SIZE, BASE_BUTTON_SIZE);

	if (bitmap)
		setBitmap(bitmap);

	TQToolTip::add(this, tip);
}


BlueCurveButton::~BlueCurveButton()
{
	if (deco)
		delete deco;
}


TQSize BlueCurveButton::sizeHint() const
{
	return( TQSize(BASE_BUTTON_SIZE,BASE_BUTTON_SIZE ));
}


void BlueCurveButton::resizeEvent( TQResizeEvent* e)
{
	doShape();
	TQButton::resizeEvent(e);
}

void BlueCurveButton::showEvent(TQShowEvent *)
{
	doShape();
}

void BlueCurveButton::doShape()
{
	// Obtain widget bounds.
	int w  = rect() .width();
	int h  = rect().height();
	int r = BUTTON_DIAM / 2;
	int dm = BUTTON_DIAM;
	TQBitmap mask(size(), true);

	TQPainter p3(&mask);
	TQBrush blackbr(TQt::color1);
	p3.fillRect(0,0,w,h,blackbr);

	p3.setPen(TQt::color1);
	p3.setBrush(TQt::color1);
	if (pos == ButtonLeft) {
		p3.eraseRect(0, -TOP_GRABBAR_WIDTH, r, r);
		p3.drawPie(0, -TOP_GRABBAR_WIDTH, dm-1, dm-1, 90*16, 90*16);
		p3.drawArc(0, -TOP_GRABBAR_WIDTH, dm-1, dm-1, 90*16, 90*16);
	} else if (pos == ButtonRight)
		{
			p3.eraseRect(w-r , -TOP_GRABBAR_WIDTH, r,r);
			p3.drawPie(w-dm, -TOP_GRABBAR_WIDTH, dm-1, dm-1, 0*16, 90*16);
			p3.drawArc(w-dm, -TOP_GRABBAR_WIDTH, dm-1, dm-1, 0*16, 90*16);
		}
	p3.end();
	setMask(mask);
}

void BlueCurveButton::setBitmap(const unsigned char *bitmap)
{
	if (deco)
		delete deco;

	deco = new TQBitmap(14, 14, bitmap, true);
	deco->setMask( *deco );
	repaint( false );
}


void BlueCurveButton::drawButton(TQPainter *p)
{
	if (!BlueCurve_initialized)
		return;

	if (deco)
	{
		// Fill the button background with an appropriate button image
		KPixmap btnbg;

		if (isDown())
			btnbg = client->isActive() ? *btnDownPix : *ibtnDownPix;
		else
			btnbg = client->isActive() ? *btnUpPix : *ibtnUpPix;
      
		if (isMouseOver)
			KPixmapEffect::intensity(btnbg, 0.8);

		// Scale the background image if required
		// This is slow, but we assume this isn't done too often
		if ( !large )
		{
			btnbg.detach();
			btnbg.convertFromImage(btnbg.convertToImage().smoothScale(14, 14));
		}

		p->drawPixmap( 0, 0, btnbg );

	}

	// If we have a decoration bitmap, then draw that
	// otherwise we paint a menu button (with mini icon), or a sticky button.
	if ( deco )
	{
		// Select the appropriate button decoration color
		bool darkDeco = tqGray( KDecoration::options()->color(
				KDecoration::ColorButtonBg,
				client->isActive()).rgb() ) > 127;

		TQColor bgc = KDecoration::options()->color(KDecoration::ColorTitleBar, client->isActive());
		if (isMouseOver)
			p->setPen( darkDeco ? bgc.dark(120) : bgc.light(120) );
		else
			p->setPen( darkDeco ? bgc.dark(150) : bgc.light(150) );

		int xOff = (width()-14)/2;
		int yOff = (height()-14)/2;
		p->drawPixmap(isDown() ? xOff+1: xOff, isDown() ? yOff+1 : yOff, *deco);
	} else
	{
		KPixmap btnpix;

		if (isOnAllDesktops)
		{
			if (client->isActive())
				btnpix = isOn() ? *pinDownPix : *pinUpPix;
			else
				btnpix = isOn() ? *ipinDownPix : *ipinUpPix;
		} else
		{
			btnpix = client->icon().pixmap( TQIconSet::Small, TQIconSet::Normal );
        }
      
		// Intensify the image if required
		if (isMouseOver)
			btnpix = KPixmapEffect::intensity(btnpix, 0.8);
      
		// Smooth scale the pixmap for small titlebars
		// This is slow, but we assume this isn't done too often
		if ( !large )
			btnpix.convertFromImage(btnpix.convertToImage().smoothScale(14, 14));
      
		p->drawPixmap( 0, 0, btnpix );
	}

	TQColorGroup g;
	p->setPen(g.dark());
}


// Make the protected member public
void BlueCurveButton::turnOn( bool isOn )
{
	if ( isToggleButton() )
		setOn( isOn );
}


void BlueCurveButton::enterEvent(TQEvent *e) 
{ 
	isMouseOver=true;
	repaint(false); 
	TQButton::enterEvent(e);
}


void BlueCurveButton::leaveEvent(TQEvent *e)
{ 
	isMouseOver=false;
	repaint(false); 
	TQButton::leaveEvent(e);
}


void BlueCurveButton::mousePressEvent( TQMouseEvent* e )
{
	last_button = e->button();
	TQMouseEvent me( e->type(), e->pos(), e->globalPos(),
			(e->button()&realizeButtons)?LeftButton:NoButton, e->state() );
	TQButton::mousePressEvent( &me );
}


void BlueCurveButton::mouseReleaseEvent( TQMouseEvent* e )
{
	last_button = e->button();
	TQMouseEvent me( e->type(), e->pos(), e->globalPos(),
			(e->button()&realizeButtons)?LeftButton:NoButton, e->state() );
	TQButton::mouseReleaseEvent( &me );
}


void BlueCurveButton::reset()
{
	// repaint the whole thing
	repaint(false);
}


void BlueCurveButton::setTipText(const TQString &tip) {
	if ( KDecoration::options()->showTooltips() ) {
		TQToolTip::remove(this );
		TQToolTip::add(this, tip );
	}
}


BlueCurveClient::BlueCurveClient( KDecorationBridge* bridge, KDecorationFactory* factory )
		: KDecoration (bridge, factory)
{
}


void BlueCurveClient::init()
{
	createMainWidget( WResizeNoErase | WStaticContents | WRepaintNoErase );

	widget()->installEventFilter( this );

	// No flicker thanks
	widget()->setBackgroundMode( TQWidget::NoBackground );

	// Set button pointers to NULL so we can track things
	for(int i=0; i < BlueCurveClient::BtnCount; i++)
		button[i] = NULL;

	// Finally, toolWindows look small
	if ( isTool() ) {
		titleHeight  = toolTitleHeight;
		largeButtons = false;
	} else {
		titleHeight  = normalTitleHeight;
		largeButtons = true;
	}

	// Pack the windowWrapper() window within a grid
	TQVBoxLayout* g = new TQVBoxLayout(widget());
	g->setResizeMode(TQLayout::FreeResize);
	g->addSpacing(TOP_GRABBAR_WIDTH);       // Top grab bar

	// Pack the titlebar HBox with items
	hb = new TQHBoxLayout();
	hb->setSpacing(0);
	hb->setMargin(0);
	hb->setResizeMode( TQLayout::FreeResize );

	hb->addSpacing(2);
	addClientButtons( options()->titleButtonsLeft(), true );
	titlebar = new TQSpacerItem( 10, titleHeight,
		TQSizePolicy::Expanding, TQSizePolicy::Minimum );
	hb->addItem(titlebar);

	//hb->addSpacing(2);
	addClientButtons( options()->titleButtonsRight(), false );
	hb->addSpacing(2);

	g->addLayout( hb );
	g->addSpacing(1); // line under titlebar

	// Add the middle section
	hb = new TQHBoxLayout();
	hb->addSpacing(BORDER_WIDTH);
	if (isPreview())
		hb->addWidget(new TQLabel( i18n( "<center><b>Bluecurve preview</b></center>" ), widget()));
	else
		hb->addWidget(new TQLabel("", widget()));
	hb->addSpacing(BORDER_WIDTH);
	g->addLayout( hb );

	// Determine the size of the lower grab bar
	if ( showGrabBar && (!isTool()) )
		g->addSpacing(BORDER_WIDTH); // bottom handles
	else
		g->addSpacing(4); // bottom handles
}


bool BlueCurveClient::isTool() const
{
	NET::WindowType type = windowType(NET::NormalMask|NET::ToolbarMask|NET::UtilityMask|NET::MenuMask);
	return ((type==NET::Toolbar)||(type==NET::NET::Utility)||(type==NET::Menu));
}


void BlueCurveClient::addClientButtons( const TQString& s, bool isLeft )
{
	int pos;
	// Make sure we place the spacing between the buttons
	bool first_button = true;
	BlueCurveButton* last_button = NULL;

	if (s.length() > 0) {
		for(unsigned int i = 0; i < s.length(); i++) {
			if (i == 0 && isLeft)
				pos = ButtonLeft;
			else
				pos = ButtonMid;

			switch( s[i].latin1() )
			{
				// Menu button
				case 'M':
					if (!button[BtnMenu])
					{
						button[BtnMenu] = new BlueCurveButton(this, "menu",
							largeButtons, pos, false, menu_bits, i18n("Menu"), LeftButton|RightButton);
						connect( button[BtnMenu], SIGNAL(pressed()),
							this, SLOT(menuButtonPressed()) );
						connect( button[BtnMenu], SIGNAL(released()),
							this, SLOT(menuButtonReleased()));
						if (! first_button)
							hb->addSpacing(2);
						else
							first_button = false;
						hb->addWidget( button[BtnMenu] );
						last_button = button[BtnMenu];
					}
					break;

				// Sticky button
				case 'S':
					if (!button[BtnOnAllDesktops])
					{
						button[BtnOnAllDesktops] = new BlueCurveButton(this, "on_all_desktops", 
							largeButtons, pos, true, NULL, i18n("On All Desktops"));
						button[BtnOnAllDesktops]->turnOn( isOnAllDesktops() );
						connect( button[BtnOnAllDesktops], SIGNAL(clicked()), 
							this, SLOT(toggleOnAllDesktops()) );
						hb->addSpacing(2);
						hb->addWidget( button[BtnOnAllDesktops] );
						last_button = button[BtnOnAllDesktops];
					}
					break;

				// Help button
				case 'H':
					if( providesContextHelp() && (!button[BtnHelp]) )
					{
						button[BtnHelp] = new BlueCurveButton(this, "help",
							largeButtons, pos, true, question_bits,
							i18n("Help"));
						connect( button[BtnHelp], SIGNAL( clicked() ),
							this, SLOT( contextHelp() ));
						if (! first_button)
							hb->addSpacing(2);
						else
							first_button = false;
						hb->addWidget( button[BtnHelp] );
						last_button = button[BtnHelp];
					}
					break;

				// Minimize button
				case 'I':
					if ( (!button[BtnIconify]) && isMinimizable())
					{
						button[BtnIconify] = new BlueCurveButton(this, "iconify",
							largeButtons, pos, false, iconify_bits,
							i18n("Minimize"));
						connect( button[BtnIconify], SIGNAL( clicked()),
							this, SLOT(minimize()) );
						if (! first_button)
							hb->addSpacing(2);
						else
							first_button = false;
						hb->addWidget( button[BtnIconify] );
						last_button = button[BtnIconify];
					}
					break;

				// Maximize button
				case 'A':
				if ( (!button[BtnMax]) && isMaximizable())
				{
					button[BtnMax]  = new BlueCurveButton(this, "maximize",
						largeButtons, pos, false, maximize_bits,
						i18n("Maximize"), LeftButton|MidButton|RightButton);
					connect( button[BtnMax], SIGNAL( clicked()),
						this, SLOT(slotMaximize()) );
					if (! first_button)
						hb->addSpacing(2);
					else
						first_button = false;
					hb->addWidget( button[BtnMax] );
					last_button = button[BtnMax];
				}
				break;

				// Close button
				case 'X':
				if (!button[BtnClose])
				{
					button[BtnClose] = new BlueCurveButton(this, "close",
						largeButtons, pos, false, close_bits,
						i18n("Close"));
					connect( button[BtnClose], SIGNAL( clicked()),
						this, SLOT(closeWindow()) );
					if (! first_button)
						hb->addSpacing(2);
					else
						first_button = false;
					hb->addWidget( button[BtnClose] );
					last_button = button[BtnClose];
				}
				break;

				// Spacer item (only for non-tool windows)
				case '_':
				if ( !isTool() )
					hb->addSpacing(2);
			}
		}

		if (last_button) {
			if(not isLeft)
				last_button->pos = ButtonRight;
			else
				last_button->pos = LeftButtonRight;
		}
	}
}


void BlueCurveClient::iconChange()
{
	if (button[BtnMenu] && button[BtnMenu]->isVisible())
		button[BtnMenu]->repaint(false);
}


void BlueCurveClient::desktopChange()
{
	if (button[BtnOnAllDesktops]) {
		button[BtnOnAllDesktops]->turnOn(isOnAllDesktops());
		button[BtnOnAllDesktops]->repaint(false);
		button[BtnOnAllDesktops]->setTipText(isOnAllDesktops() ? i18n("Not On All Desktops") : i18n("On All Desktops"));
	}
}


void BlueCurveClient::slotMaximize()
{
	if (button[BtnMax])
	{
		switch (button[BtnMax]->last_button)
		{
			case MidButton:
				maximize(maximizeMode() ^ MaximizeVertical );
				break;
			case RightButton:
				maximize(maximizeMode() ^ MaximizeHorizontal );
				break;
			default:
				maximize(maximizeMode() == MaximizeFull ? MaximizeRestore : MaximizeFull );
		}
	}
}


void BlueCurveClient::resizeEvent( TQResizeEvent* e)
{
	doShape();
	calcHiddenButtons();
  
	if (widget()->isVisibleToTLW())
	{
		widget()->update(widget()->rect());
		int dx = 0;
		int dy = 0;

		if ( e->oldSize().width() != width() )
			dx = 32 + TQABS( e->oldSize().width() -  width() );

		if ( e->oldSize().height() != height() )
			dy = 8 + TQABS( e->oldSize().height() -  height() );

		if ( dy )
			widget()->update( 0, height() - dy + 1, width(), dy );
		if ( dx )
		{
			widget()->update( width() - dx + 1, 0, dx, height() );
			widget()->update( TQRect( TQPoint(4,4), titlebar->geometry().bottomLeft() - TQPoint(1,0) ) );
			widget()->update( TQRect( titlebar->geometry().topRight(), TQPoint(width() - 4,
				titlebar->geometry().bottom()) ) );
			// Titlebar needs no paint event
			// widget()->repaint(titlebar->geometry(), false);
			TQApplication::postEvent( widget(), new TQPaintEvent(titlebar->geometry(),FALSE) );
		}
	}
}


void BlueCurveClient::captionChange()
{
	widget()->repaint( titlebar->geometry(), false );
}


void BlueCurveClient::paintEvent( TQPaintEvent* )
{
	if (!BlueCurve_initialized)
		return;

	TQColorGroup g;

	bool drawLeftDivider = true; 
	bool drawRightDivider = true; 

	// Obtain widget bounds.
	TQRect r(widget()->rect());
	TQPainter p(widget());

	int x = r.x();
	int y = r.y();
	int x2 = x + r.width() - 1;
	int y2 = y + r.height() - 1;
	int w  = r.width();
	int h  = r.height();
	g = options()->colorGroup(ColorFrame, isActive());
	// Determine where to place the extended left titlebar
	//int leftFrameStart = (h > 42) ? y+titleHeight+26: y+titleHeight;

	// Determine where to make the titlebar color transition
	r = titlebar->geometry();
	//int rightOffset = r.x()+r.width()+1;

	TQColor c2 = options()->color(ColorFrame, isActive() );
	// Fill with frame color behind RHS buttons
	///  p.fillRect( x, y+2, x2, titleHeight+1, c2);

	// Create a disposable pixmap buffer for the titlebar
	// very early before drawing begins so there is no lag
	// during painting pixels.
	titleBuffer->resize( w, titleHeight + TOP_GRABBAR_WIDTH );


	// Draw the title bar.
	r = titlebar->geometry();

	// Obtain titlebar blend colours
	TQColor c1 = options()->color(ColorTitleBar, isActive() );

	TQPainter p2( titleBuffer, this );
	TQColor activeTitleColor1(options()->color(ColorTitleBar,      true));
	TQColor activeTitleColor2(options()->color(ColorTitleBlend,    true));

	TQColor inactiveTitleColor1(options()->color(ColorTitleBar,    false));
	TQColor inactiveTitleColor2(options()->color(ColorTitleBlend,  false));
	bool highcolor = useGradients && (TQPixmap::defaultDepth() > 8);

	if (highcolor)
	{
		static TQSize oldsize(0,0);
		TQSize titleBufferSize(w, titleHeight + TOP_GRABBAR_WIDTH);
		if (oldsize != titleBufferSize)
		{
			oldsize = titleBufferSize;
			if (aUpperGradient)
			{
				delete aUpperGradient;
				aUpperGradient = NULL;
			}
			if (iUpperGradient)
			{
				delete iUpperGradient;
				iUpperGradient = NULL;
			}
	      
			// Create the titlebar gradients
			if (activeTitleColor1 != activeTitleColor2)
			{
				aUpperGradient = new KPixmap(oldsize);
				KPixmapEffect::gradient(*aUpperGradient,
					activeTitleColor2,
					activeTitleColor1,
					KPixmapEffect::VerticalGradient);
			}

			if (inactiveTitleColor1 != inactiveTitleColor2)
			{
				iUpperGradient = new KPixmap(oldsize);
				KPixmapEffect::gradient(*iUpperGradient,
					inactiveTitleColor2,
					inactiveTitleColor1,
					KPixmapEffect::VerticalGradient);
			}
		}
	}
  
	KPixmap* upperGradient = isActive() ? aUpperGradient : iUpperGradient;
	// Draw the titlebar gradient

	if (upperGradient)
		p2.drawPixmap(0, TOP_GRABBAR_WIDTH, *upperGradient);
	else
		p2.fillRect(0, TOP_GRABBAR_WIDTH, w, titleHeight, c1);

	TQFont fnt = options()->font(true, true);

	if ( isTool() )
		fnt.setPointSize( fnt.pointSize()-2 );  // Shrink font by 2pt

	p2.setFont( fnt );

	// Draw the titlebar stipple if active and available
	if (isActive() && titlePix)
	{
		TQFontMetrics fm(fnt);
		int captionWidth = fm.width(caption()) + 1;
		p2.drawTiledPixmap( r.x() + 2 + 2 + captionWidth, TOP_GRABBAR_WIDTH,
			r.width() - 2 - 4 - captionWidth, 
			titleHeight+1, *titlePix );
	}

	if (isActive())
	{
		p2.setPen( options()->color(ColorTitleBlend, isActive()).dark());
		p2.drawText(r.x() + 2 + 1, TOP_GRABBAR_WIDTH + 1,
			r.width() - 2 - 1, r.height(),
			AlignLeft | AlignVCenter, caption() );
	}

	p2.setPen( options()->color(ColorFont, isActive()) );
	p2.drawText(r.x() + 2, TOP_GRABBAR_WIDTH,
		r.width() - 2, r.height(),
		AlignLeft | AlignVCenter, caption() );

	// Main Title Bar background area
	p2.setPen(TQt::white);
	p2.drawLine(x + 1, y + 1, x2 - 1, y + 1);
	// This is kind of broken...
	// We fill in the inner part of the circle here.  This is dependent on BUTTON_DIAM
	p2.drawLine(x + 1, y + 1, x + 1, y + TOP_GRABBAR_WIDTH + titleHeight);
	p2.drawLine(x + 2, y + 2, x + 3, y + 2);
	p2.drawLine(x + 2, y + 2, x + 2, y + 3);
	p2.drawLine(x + w - 2 , y + 1, x + w - 2, y + TOP_GRABBAR_WIDTH + titleHeight);
	p2.drawLine(x + w - 3, y + 2, x + w - 3, y + 5);
	p2.drawLine(x + w - 4, y + 2, x + w - 3, y + 2);

	if (isActive())
	{
		TQColor lighterColor (options()->color(ColorTitleBar, true).light (150));
		p2.setPen (lighterColor);
		p2.drawLine (r.x(), 2, r.x() + r.width(), 2);
		int h, s, v;
		lighterColor.hsv (&h, &s, &v);
		s /= 2;
		s = (s > 255) ? 255 : (int) s;

		TQColor satColor(h, s, v, TQColor::Hsv);
		p2.setPen (satColor);
		p2.drawLine (r.x(), 1, r.x() + r.width() - 2, 1);
	}

	p2.setPen(TQt::white);
	if (isActive())
	{
		for (int i = 0; i < BtnCount; i ++)
		{
			if (button[i] == NULL)
				continue;
			if (!button[i]->isVisible())
			{
				if (button[i]->pos == ButtonRight)
					drawRightDivider = false;
				// FIXME: Should be LeftButtonLeft if we had it
				if (button[i]->pos == LeftButtonRight)
					drawLeftDivider = false;
				continue;
			}
			TQRect buttonSize = button[i]->geometry ();
			p2.setPen(TQt::white);
			p2.drawLine (buttonSize.x() - 1, TOP_GRABBAR_WIDTH,
			buttonSize.x() - 1, TOP_GRABBAR_WIDTH + titleHeight);
			if (button[i]->pos == ButtonRight)
				continue;
			else if (button[i]->pos == LeftButtonRight)
				p2.setPen(g.mid().light(120));
			else
				p2.setPen(g.dark());
			p2.drawLine (buttonSize.x() + buttonSize.width(), TOP_GRABBAR_WIDTH - 1,
				buttonSize.x() + buttonSize.width(), TOP_GRABBAR_WIDTH + titleHeight);
		}
	}


	// Top Left Button Area
	if (drawLeftDivider)
	{
		if (isActive ())
			p2.setPen(options()->color(ColorTitleBar, true).dark (150));
		else
			p2.setPen(g.mid());
		p2.drawLine (r.x() , y + 1, r.x() , y + titleHeight + TOP_GRABBAR_WIDTH);
	}



	// Top Right Button Area
	if (drawRightDivider)
	{
		if (isActive ())
			p2.setPen(options()->color(ColorTitleBar, true).dark (150));
		else
			p2.setPen(g.mid());
		p2.drawLine (r.x() + r.width() - 2, y + 1,
			r.x() + r.width() - 2 , y + titleHeight + TOP_GRABBAR_WIDTH);
	}

	// Black outer line
	p2.setPen(TQt::black);
	p2.drawRect(0,0,w,h);
	p2.drawArc(x, y, BUTTON_DIAM, BUTTON_DIAM, 90*16, 90*16);
	p2.drawArc(x + w - BUTTON_DIAM , y, BUTTON_DIAM, BUTTON_DIAM, 0*16, 90*16);
	p2.end();

	int sideStart = titleHeight + TOP_GRABBAR_WIDTH + 1;

	// Draw the left and right sides
  
	// Fill the left side first
	qDrawShadePanel(&p,
		// We compensate for the top and bottom parts of the bevel
		// by drawing 1 pixel below and above the frame part
		x + 1, y + (sideStart - 1),
		BORDER_WIDTH - 1, h - (sideStart + 2),
		g, false, 1, &g.brush(TQColorGroup::Background));

	// Right Side
	qDrawShadePanel(&p,
		x2 - (BORDER_WIDTH - 2), y + (sideStart - 1),
		BORDER_WIDTH - 2, h - (sideStart + 2),
		g, false, 1, &g.brush(TQColorGroup::Background));

	p.setPen(g.dark());
	p.drawLine(x2 - (BORDER_WIDTH - 1), sideStart, x2 - (BORDER_WIDTH - 1), h - sideStart);

	// Draw the bottom
	qDrawShadePanel(&p,
		x, y2 - (BORDER_WIDTH - 2),
		w, (BORDER_WIDTH - 2),
		g, false, 1, &g.brush(TQColorGroup::Background));
	p.setPen(g.dark());
	p.drawLine(x, y2 - (BORDER_WIDTH - 1), x2, y2 - (BORDER_WIDTH - 1));

	// Line above the app and below the title bar
	p.setPen(g.dark());
	p.drawLine(x, y + titleHeight + TOP_GRABBAR_WIDTH,
		x2, y + titleHeight + TOP_GRABBAR_WIDTH);

	bitBlt( widget(), 0, 0, titleBuffer );

	// Draw an outer black frame
	p.setPen(TQt::black);
	p.drawRect(0,0,w,h);

	// Put on the bottom corners
	p.drawPixmap(0, h - bottomLeftPix->height(),
		isActive() ? *abottomLeftPix : *bottomLeftPix);
	p.drawPixmap(w - bottomRightPix->width(), h - bottomRightPix->height(), 
		isActive() ? *abottomRightPix : *bottomRightPix);
	p.end();
}


void BlueCurveClient::shadeChange()
{ ; }


void BlueCurveClient::doShape()
{
	// Obtain widget bounds.
	TQRect r(widget()->rect());
	int x = 0;
	int y = 0;
	//int x2 = width() - 1;
	//int y2 = height() - 1;
	int w  = width();
	int h  = height();

	int rad = BUTTON_DIAM / 2;
	int dm = BUTTON_DIAM;

	TQBitmap mask(w+1, h+1, true);

	TQPainter p(&mask);

	p.fillRect(x, y, w+1, h+1, TQt::color1);

	p.eraseRect(x, y, rad, rad);
	p.eraseRect(w-rad+1, 0, rad, rad);

	p.eraseRect(x, h-BOTTOM_CORNER, BOTTOM_CORNER, BOTTOM_CORNER);
	p.eraseRect(w-BOTTOM_CORNER, h-BOTTOM_CORNER, BOTTOM_CORNER, BOTTOM_CORNER);

	p.setPen(TQt::color1);
	p.setBrush(TQt::color1);

	p.drawPie(x, y, dm, dm, 90*16, 90*16);
	p.drawArc(x, y, dm, dm, 90*16, 90*16);

	p.drawPie(w-dm, 0, dm, dm, 0*16, 90*16);
	p.drawArc(w-dm, 0, dm, dm, 0*16, 90*16);

	p.drawPixmap(x, h - bottomLeftPix->height(), *bottomLeftPix->mask());

	p.drawPixmap(w-bottomRightPix->width(), h - bottomRightPix->height(), 
		*bottomRightPix->mask());
	p.fillRect(x+BOTTOM_CORNER, h - bottomLeftPix->height(),
		bottomLeftPix->width()-BOTTOM_CORNER,
		bottomLeftPix->height()-BOTTOM_CORNER,
		TQt::color1);

	p.fillRect(w-bottomRightPix->width(), h - bottomRightPix->height(), 
		bottomRightPix->width()-BOTTOM_CORNER,
		bottomRightPix->height()-BOTTOM_CORNER,
		TQt::color1);

	p.end();
	setMask(mask);
}


void BlueCurveClient::showEvent(TQShowEvent *)
{
	calcHiddenButtons();
	doShape();
	widget()->show();
}


void BlueCurveClient::mouseDoubleClickEvent( TQMouseEvent * e )
{
	if ( titlebar->geometry().contains( e->pos() ) )
		titlebarDblClickOperation();
}


void BlueCurveClient::maximizeChange()
{
	if (button[BtnMax]) {
		button[BtnMax]->setBitmap((maximizeMode()==MaximizeFull) ? minmax_bits : maximize_bits);
		button[BtnMax]->setTipText((maximizeMode()==MaximizeFull) ? i18n("Restore") : i18n("Maximize"));
	}
}


void BlueCurveClient::borders( int& left, int& right, int& top, int& bottom ) const
{
	left = right = borderWidth;
	top = titleHeight + 4;
	bottom = (showGrabBar && isResizable()) ? grabBorderWidth : borderWidth;
}


void BlueCurveClient::activeChange()
{
	for(int i=BlueCurveClient::BtnHelp; i < BlueCurveClient::BtnCount; i++)
		if(button[i])
			button[i]->repaint(false);
		widget()->repaint(false);
}


void BlueCurveClient::resize( const TQSize& s )
{
	widget()->resize( s );
}


TQSize BlueCurveClient::minimumSize() const
{
	return widget()->minimumSize();
}


// The hiding button while shrinking, show button while expanding magic
void BlueCurveClient::calcHiddenButtons()
{
	// Hide buttons in this order:
	// Sticky, Help, Maximize, Minimize, Close, Menu.
	BlueCurveButton* btnArray[] = { button[BtnOnAllDesktops], button[BtnHelp],
		button[BtnMax], button[BtnIconify], button[BtnClose],
		button[BtnMenu] };

	int minwidth  = 160; // Start hiding at this width
	int btn_width = 16;
	int current_width = width();
	int count = 0;
	int i;

	// Find out how many buttons we need to hide.
	while (current_width < minwidth)
	{
		current_width += btn_width;
		count++;
	}

	// Bound the number of buttons to hide
	if (count > 6)
		count = 6;

	// Hide the required buttons...
	for(i = 0; i < count; i++)
	{
		if ( btnArray[i] && btnArray[i]->isVisible() )
			btnArray[i]->hide();
    }

	// Show the rest of the buttons...
	for(i = count; i < 6; i++)
	{
		if (btnArray[i] && (!btnArray[i]->isVisible()) )
			btnArray[i]->show();
	}
}


KDecoration::Position BlueCurveClient::mousePosition( const TQPoint& p ) const
{
	Position m = PositionCenter;

	// Modify the mouse position if we are using a grab bar.
	if (showGrabBar && (!isTool()) )
		if (p.y() < (height() - 8))
			m = KDecoration::mousePosition(p);
		else
		{
			if (p.x() >= (width() - 20))
				m = PositionBottomRight;
			else if (p.x() <= 20)
				m = PositionBottomLeft;
			else
				m = PositionBottom;
		}
	else
		m = KDecoration::mousePosition(p);

	return m;
}


// Make sure the menu button follows double click conventions set in kcontrol
void BlueCurveClient::menuButtonPressed()
{
	static TQTime t;
	static BlueCurveClient* lastClient = NULL;
	bool dbl = ( lastClient == this && t.elapsed() <= TQApplication::doubleClickInterval());
	lastClient = this;
	t.start();

	if (dbl)
	{
		m_closing = true;
		return;
	}

	TQPoint menupoint ( button[BtnMenu]->rect().bottomLeft().x()-1,
					button[BtnMenu]->rect().bottomLeft().y()+2 );
	KDecorationFactory* f = factory();
	showWindowMenu( button[BtnMenu]->mapToGlobal( menupoint ));
	if( !f->exists( this )) // 'this' was destroyed
		return;

	button[BtnMenu]->setDown(false);
}


void BlueCurveClient::menuButtonReleased()
{
	if (m_closing)
		closeWindow();
}


bool BlueCurveClient::eventFilter( TQObject* o, TQEvent* e )
{
	if ( o != widget() )
		return false;

	switch( e->type())
	{
		case TQEvent::Resize:
			resizeEvent(static_cast< TQResizeEvent* >( e ) );
			return true;
		case TQEvent::Paint:
			paintEvent(static_cast< TQPaintEvent* >( e ) );
			return true;
		case TQEvent::MouseButtonDblClick:
			mouseDoubleClickEvent(static_cast< TQMouseEvent* >( e ) );
			return true;
		case TQEvent::MouseButtonPress:
			processMousePressEvent(static_cast< TQMouseEvent* >( e ) );
			return true;
		case TQEvent::Show:
			showEvent(static_cast<TQShowEvent *>(e));
			return true;
		default:
			break;
	}
	return false;
}

} // namespace

// Extended KWin plugin interface
extern "C"  KDecorationFactory *create_factory()
{
	return new BlueCurve::BlueCurveHandler();
}

#include "bluecurveclient.moc"

// vim: ts=4
