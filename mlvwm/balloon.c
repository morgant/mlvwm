/****************************************************************************/
/* This module is mostly all new                                            */
/* by TakaC Hasegawa (tak@bioele.nuee.nagoya-u.ac.jp)                       */
/* Copyright 1996 TakaC Hasegawa. No restrictions are placed on this code,  */
/* as long as the copyright notice is preserved                             */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "mlvwm.h"
#include "screen.h"
#include "misc.h"
#include "event.h"

#include <X11/extensions/shape.h>

static char *WinLabel[] = {
	"Name:",
	"Class:", "Resource:",
	"X:", "Y:",
	"Width:", "Height:",
	NULL };

void CreateBalloonWindow( void )
{
	unsigned long valuemask;
	XSetWindowAttributes attributes;

	valuemask = CWBackPixel | CWCursor | CWEventMask;
	attributes.background_pixel = WhitePixel( dpy, Scr.screen );
	attributes.cursor = Scr.MlvwmCursors[DEFAULT];
	attributes.event_mask = ExposureMask| LeaveWindowMask;
	Scr.Balloon = XCreateWindow( dpy, Scr.Root, 0, 0, 30, 30 , 0,
					CopyFromParent, InputOutput,
					CopyFromParent,
					valuemask, &attributes );
}

void
DrawBackGround( int fx, int fy, int xoff, int yoff, int width, int height )
{
	XPoint tri[3];

	XDrawArc( dpy, Scr.Balloon, Scr.BlackGC,
			 xoff+0, yoff+0, 20, 20, 90*64, 90*64 );
	XDrawArc( dpy, Scr.Balloon, Scr.BlackGC,
			 xoff+width-21, yoff+0, 20, 20, 0, 90*64 );
	XDrawArc( dpy, Scr.Balloon, Scr.BlackGC,
			 xoff+width-21, yoff+height-21, 20, 20, 270*64, 90*64 );
	XDrawArc( dpy, Scr.Balloon, Scr.BlackGC,
			 xoff+0, yoff+height-21, 20, 20, 180*64, 90*64 );
	XDrawLine( dpy, Scr.Balloon, Scr.BlackGC,
			  xoff+10, yoff+0, xoff+width-11, yoff+0 );
	XDrawLine( dpy, Scr.Balloon, Scr.BlackGC,
			  xoff+10, yoff+height-1, xoff+width-11, yoff+height-1);
	XDrawLine( dpy, Scr.Balloon, Scr.BlackGC,
			  xoff+0, yoff+10, xoff+0, yoff+height-11 );
	XDrawLine( dpy, Scr.Balloon, Scr.BlackGC,
			  xoff+width-1, yoff+10, xoff+width-1, yoff+height-11);

	tri[1].x = fx;		tri[1].y = fy;
	if( fx==0 && fy==height-1 ){
		tri[0].x = fx+20;	tri[0].y = fy-20;
		tri[2].x = fx+20;	tri[2].y = fy-10;
	}
	if( fx==width-1 && fy==height+20-1 ){
		tri[0].x = fx-20;	tri[0].y = fy-20;
		tri[2].x = fx-10;	tri[2].y = fy-20;
	}
	if( fx==width+20-1 && fy==0 ){
		tri[0].x = fx-20;	tri[0].y = fy+20;
		tri[2].x = fx-20;	tri[2].y = fy+10;
	}
	if( fx==0 && fy==0 ){
		tri[0].x = fx+20;	tri[0].y = fy+20;
		tri[2].x = fx+10;	tri[2].y = fy+20;
	}
	XDrawLines( dpy, Scr.Balloon, Scr.BlackGC, tri, 3, CoordModeOrigin );
	if( fx==0 && fy==height-1 ){
		tri[0].y++;		tri[2].y--;
	}
	if( fx==width-1 && fy==height+20-1 )		tri[0].x++;
	if( fx==width+20-1 && fy==0 )		tri[0].y--;
	if( fx==0 && fy==0 ){
		tri[0].x--;		tri[2].x++;
	}
	XDrawLine( dpy, Scr.Balloon, Scr.WhiteGC,
			  tri[0].x, tri[0].y, tri[2].x, tri[2].y );
}

void ShapeBalloon( int fx, int fy, int xoff, int yoff, int woff, int hoff, int width, int height )
{
	XPoint tri[3];
	Pixmap mask;
	GC ShapeGC;

	mask = XCreatePixmap( dpy, Scr.Root, width+woff, height+hoff, 1 );
	ShapeGC = XCreateGC( dpy, mask, 0, 0 );

#if defined(sun) && !defined(__NetBSD__)
	XSetForeground( dpy, ShapeGC, WhitePixel( dpy, Scr.screen ));
#else
	XSetForeground( dpy, ShapeGC, BlackPixel( dpy, Scr.screen ));
#endif
	XFillRectangle( dpy, mask, ShapeGC, 0, 0, width+woff, height+hoff );
#if defined(sun) && !defined(__NetBSD__)
	XSetForeground( dpy, ShapeGC, BlackPixel( dpy, Scr.screen ));
	XSetBackground( dpy, ShapeGC, WhitePixel( dpy, Scr.screen ));
#else
	XSetForeground( dpy, ShapeGC, WhitePixel( dpy, Scr.screen ));
	XSetBackground( dpy, ShapeGC, BlackPixel( dpy, Scr.screen ));
#endif

	XFillRectangle(dpy, mask, ShapeGC, xoff+10, yoff+0, width-20, 10);
	XFillRectangle(dpy, mask, ShapeGC, xoff+10, yoff+height-10, width-20, 10);
	XFillRectangle( dpy, mask, ShapeGC, xoff+0, yoff+10, width, height-20 );

	XFillArc( dpy, mask, ShapeGC, xoff-1, yoff-1, 22, 22, 90*64, 90*64 );
	XFillArc( dpy, mask, ShapeGC, xoff+width-22, yoff-1, 22, 22, 0, 90*64 );
	XFillArc( dpy, mask, ShapeGC,
			 xoff+width-22, yoff+height-22, 22, 22, 270*64, 90*64 );
	XFillArc( dpy, mask, ShapeGC,
			 xoff-1, yoff+height-22, 22, 22, 180*64, 90*64 );

	if( fx==0 && fy==height-1 ){
		tri[0].x = fx+20;	tri[0].y = fy-21;
		tri[1].x = fx-1;	tri[1].y = fy+1;
		tri[2].x = fx+20;	tri[2].y = fy-9;
	}
	if( fx==width-1 && fy==height+20-1 ){
		tri[0].x = fx-21;	tri[0].y = fy-20;
		tri[1].x = fx+1;	tri[1].y = fy+1;
		tri[2].x = fx-9;	tri[2].y = fy-20;
	}
	if( fx==width+20-1 && fy==0 ){
		tri[0].x = fx-20;	tri[0].y = fy+21;
		tri[1].x = fx+1;	tri[1].y = fy-1;
		tri[2].x = fx-20;	tri[2].y = fy+9;
	}
	if( fx==0 && fy==0 ){
		tri[0].x = fx+21;	tri[0].y = fy+20;
		tri[1].x = fx-1;	tri[1].y = fy-1;
		tri[2].x = fx+9;	tri[2].y = fy+20;
	}
	XFillPolygon( dpy, mask, ShapeGC, tri, 3, Nonconvex, CoordModeOrigin );

	XShapeCombineMask( dpy, Scr.Balloon, ShapeBounding,
                      0, 0, mask, ShapeSet);

	XFreePixmap( dpy, mask );
	XFreeGC( dpy, ShapeGC );
}

void BalloonSize( int *lh, int *lw, int *offset, int *w, int *h, MlvwmWindow *mw )
{
	int lp;
	char contents[256];
	int contents_width=0, strw, strh;
	int xinc, yinc;

	*w = 0;  /* Balloon Window Width */
	*h = 0;  /* Balloon Window Height */
	*lw = 0; /* Label Width */
	*lh = 0; /* Line Height */

	for( lp=0; WinLabel[lp]; lp++ ){
		StrWidthHeight( BALLOONFONT, &strw, &strh,
			   offset, WinLabel[lp], strlen(WinLabel[lp]) );
		*lw = max( *lw, strw );
		*lh = max( strh, *lh );
	}

	StrWidthHeight( BALLOONFONT, &strw, &strh,
				   offset, mw->name, strlen(mw->name) );
	*lh = max( strh, *lh );
	contents_width = max( contents_width, strw );

	if( &(mw->class) ){
		StrWidthHeight( BALLOONFONT, &strw, &strh,
					   offset, mw->class.res_class,
					   strlen( mw->class.res_class) );
		contents_width = max( strw, contents_width );
		*lh = max( strh, *lh );
		StrWidthHeight( BALLOONFONT, &strw, &strh,
					   offset, mw->class.res_name,
					   strlen(mw->class.res_name) );
		contents_width = max( strw, contents_width );
		*lh = max( strh, *lh );
	}

	sprintf( contents, "%d", mw->frame_x );
	StrWidthHeight(BALLOONFONT,&strw,&strh,offset,contents,strlen(contents));
	contents_width = max( strw, contents_width );
	*lh = max( strh, *lh );

	sprintf( contents, "%d", mw->frame_y );
	StrWidthHeight(BALLOONFONT,&strw,&strh,offset,contents,strlen(contents));
	contents_width = max( strw, contents_width );
	*lh = max( strh, *lh );

	if( mw->hints.flags & PResizeInc){
		xinc = mw->hints.width_inc;
		yinc = mw->hints.height_inc;
	}else
		xinc = yinc = 1;

	sprintf( contents, "%d", (mw->attr.width-mw->hints.base_width)/xinc );
	StrWidthHeight(BALLOONFONT,&strw,&strh,offset,contents,strlen(contents));
	contents_width = max( strw, contents_width );
	*lh = max( strh, *lh );

	sprintf( contents, "%d", (mw->attr.height-mw->hints.base_height)/yinc );
	StrWidthHeight(BALLOONFONT,&strw,&strh,offset,contents,strlen(contents));
	contents_width = max( strw, contents_width );
	*lh = max( strh, *lh );

	*w = contents_width + *lw + 20 + 20; /* 20 is a margin */
	*h = *lh * lp + 20;
}

void DrawWindowBalloon( int xoff, int yoff, int lh, int offset, int lw, MlvwmWindow *mw )
{
	int lp;
	char contents[255];
	int xinc, yinc;

	for( lp=0; WinLabel[lp]!=NULL; lp++ ){
		XDRAWSTRING( dpy, Scr.Balloon, BALLOONFONT,
				Scr.BlackGC, xoff+10, yoff+lh*lp+lh/2+10-offset,
				WinLabel[lp], strlen(WinLabel[lp]) );
	}
	XDRAWSTRING( dpy, Scr.Balloon, BALLOONFONT,
			Scr.BlackGC, xoff+lw+30, yoff+lh/2+10-offset,
			mw->name, strlen(mw->name) );
	XDRAWSTRING( dpy, Scr.Balloon, BALLOONFONT,
			Scr.BlackGC, xoff+lw+30, yoff+lh+lh/2+10-offset,
			mw->class.res_class, strlen(mw->class.res_class) );
	XDRAWSTRING( dpy, Scr.Balloon, BALLOONFONT,
			Scr.BlackGC, xoff+lw+30, yoff+lh*2+lh/2+10-offset,
			mw->class.res_name, strlen(mw->class.res_name) );
	sprintf( contents, "%d", mw->frame_x );
	XDRAWSTRING( dpy, Scr.Balloon, BALLOONFONT,
			Scr.BlackGC, xoff+lw+30, yoff+lh*3+lh/2+10-offset,
			contents, strlen(contents) );
	sprintf( contents, "%d", mw->frame_y );
	XDRAWSTRING( dpy, Scr.Balloon, BALLOONFONT,
			Scr.BlackGC, xoff+lw+30, yoff+lh*4+lh/2+10-offset,
			contents, strlen(contents) );

        if( mw->hints.flags & PResizeInc){
                xinc = mw->hints.width_inc;
                yinc = mw->hints.height_inc;
        }else
                xinc = yinc = 1;

	sprintf( contents, "%d", (mw->attr.width-mw->hints.base_width)/xinc );
	XDRAWSTRING( dpy, Scr.Balloon, BALLOONFONT,
			Scr.BlackGC, xoff+lw+30, yoff+lh*5+lh/2+10-offset,
			contents, strlen(contents) );
	sprintf( contents, "%d", (mw->attr.height-mw->hints.base_height)/yinc );
	XDRAWSTRING( dpy, Scr.Balloon, BALLOONFONT,
			Scr.BlackGC, xoff+lw+30, yoff+lh*6+lh/2+10-offset,
			contents, strlen(contents) );
}

void BalloonHelp( void )
{
	int mx, my, JunkX, JunkY;
	int winx, winy, fx, fy;
	int height=0, label_width=0, one_line=0;
	int width, offset, xoff=0, yoff=0, winwoff=0, winhoff=0;
	unsigned int JunkMask;
	Window JunkRoot, JunkChild;
	XEvent ev;
	Bool IsEnd=False;
	MlvwmWindow *tmp_win;

	XQueryPointer( dpy, Scr.Root, &JunkRoot, &JunkChild,
				  &JunkX, &JunkY, &mx, &my, &JunkMask);
	if( XFindContext ( dpy, JunkChild, MlvwmContext, (caddr_t *)&tmp_win )
	   == XCNOENT ){
		return;
	}

printf("flags %x\n", tmp_win->flags );
	if( Scr.Balloon==None )		CreateBalloonWindow();

	BalloonSize( &one_line, &label_width, &offset, &width, &height, tmp_win );

	winx = mx;
	winy = my-height;
	xoff = 20;
	winwoff = 20;
	fx = 0;
	fy = height-1;
	if( winx+width+20>Scr.MyDisplayWidth ){
		xoff = 0;
		if( winy<0 ){
			winx = mx-width-20;
			winy = my;
			fx = width+20-1;
			fy = 0;
		}
		else{
			winx = mx-width;
			winy = my-height-20;
			winhoff = 20;
			fx = width-1;
			fy = height+20-1;
		}
	}
	else{
		if( winy<0 ){
			winy = my;
			xoff = 0;
			yoff = 20;
			winwoff = 0;
			winhoff = 20;
			fx = 0;
			fy = 0;
		}
	}

	GrabEvent( DEFAULT );

	XMoveResizeWindow( dpy, Scr.Balloon, winx, winy,
					  width+winwoff, height+winhoff );
	ShapeBalloon( fx, fy, xoff, yoff, winwoff, winhoff, width, height );
	XMapRaised( dpy, Scr.Balloon );

	UnGrabEvent();
	do{
		XNextEvent( dpy, &ev );
		switch( ev.type ){
		  case Expose:
			if( ev.xany.window==Scr.Balloon ){
				DrawBackGround( fx, fy, xoff, yoff, width, height );
				DrawWindowBalloon( xoff, yoff, 
				  one_line, offset, label_width, tmp_win );
				XSync( dpy, 0 );
			}
			else
				HandleEvents( ev );
			break;
		  case EnterNotify:
		  case DestroyNotify:	
		  case MapRequest:
		  case UnmapNotify:
		  case ButtonPress:
		  case ConfigureRequest:
		  case ClientMessage:
		  case PropertyNotify:
		  case KeyPress:
		  case ColormapNotify:
			IsEnd = True;
	          default:
			HandleEvents( ev );
			break;
		}
	}
	while( !IsEnd );

	XUnmapWindow( dpy, Scr.Balloon );
}

