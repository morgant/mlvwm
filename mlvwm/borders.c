/****************************************************************************/
/* This module is based on fvwm, but has been siginificantly modified       */
/* by TakaC Hasegawa (tac.hasegawa@gmail.com)                               */
/****************************************************************************/
/****************************************************************************
 * This module is based on Twm, but has been siginificantly modified 
 * by Rob Nation (nation@rocket.sanders.lockheed.com)
 ****************************************************************************/
/*****************************************************************************/
/**       Copyright 1988 by Evans & Sutherland Computer Corporation,        **/
/**                          Salt Lake City, Utah                           **/
/**  Portions Copyright 1989 by the Massachusetts Institute of Technology   **/
/**                        Cambridge, Massachusetts                         **/
/**                                                                         **/
/**                           All Rights Reserved                           **/
/**                                                                         **/
/**    Permission to use, copy, modify, and distribute this software and    **/
/**    its documentation  for  any  purpose  and  without  fee is hereby    **/
/**    granted, provided that the above copyright notice appear  in  all    **/
/**    copies and that both  that  copyright  notice  and  this  permis-    **/
/**    sion  notice appear in supporting  documentation,  and  that  the    **/
/**    names of Evans & Sutherland and M.I.T. not be used in advertising    **/
/**    in publicity pertaining to distribution of the  software  without    **/
/**    specific, written prior permission.                                  **/
/**                                                                         **/
/**    EVANS & SUTHERLAND AND M.I.T. DISCLAIM ALL WARRANTIES WITH REGARD    **/
/**    TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES  OF  MERCHANT-    **/
/**    ABILITY  AND  FITNESS,  IN  NO  EVENT SHALL EVANS & SUTHERLAND OR    **/
/**    M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL  DAM-    **/
/**    AGES OR  ANY DAMAGES WHATSOEVER  RESULTING FROM LOSS OF USE, DATA    **/
/**    OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER    **/
/**    TORTIOUS ACTION, ARISING OUT OF OR IN  CONNECTION  WITH  THE  USE    **/
/**    OR PERFORMANCE OF THIS SOFTWARE.                                     **/
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mlvwm.h"
#include "screen.h"
#include "misc.h"
#include "borders.h"

#include <X11/extensions/shape.h>

static unsigned char mesh_bits_v[] = { 0x04, 0x01 };
static unsigned char mesh_bits_h[] = { 0x01, 0x00, 0x02, 0x00 };

void DrawShadowBox( int x, int y, int w, int h, Window win, int d, GC hilight, GC shadow, char mode  )
{
	int lp;

	w--;
	h--;
	for( lp=0; lp<d; lp++ ){
		if( mode&SHADOW_BOTTOM )
            XDrawLine( dpy, win, shadow, x+lp+1, y+h-lp, x+w-lp, y+h-lp );
		if( mode&SHADOW_LEFT )
            XDrawLine( dpy, win, hilight, x+lp, y+lp, x+lp, y+h-lp-1 );
		if( mode&SHADOW_TOP )
            XDrawLine( dpy, win, hilight, x+lp, y+lp, x+w-lp-1, y+lp );
		if( mode&SHADOW_RIGHT )
            XDrawLine( dpy, win, shadow, x+w-lp, y+lp+1, x+w-lp, y+h-lp-1 );
	}
}

void SetShape( MlvwmWindow *tmp_win, int w)
{
  XRectangle rect[7];
  int shape_x, shape_y, lp, point=0;

  if( tmp_win->flags&(TITLE|SBARV|SBARH|RESIZER) ){
	  shape_x = (Scr.flags&SYSTEM8?5:0);
	  shape_y = tmp_win->flags & TITLE ? TITLE_HEIGHT+1 : 0;
  }
  else{
	  shape_x = 6;
	  shape_y = 6;
  }
  XShapeCombineShape (dpy, tmp_win->frame, ShapeBounding,
                      shape_x, shape_y, tmp_win->w,
                      ShapeBounding, ShapeSet);
  if ( tmp_win->flags&TITLE ){
      rect[0].x = -1;      rect[0].y = -1;
      rect[0].width = w-1;      rect[0].height = TITLE_HEIGHT+2;
	  point++;
	  for( lp=0; lp<3; lp++ ){
		  rect[lp+1].x = tmp_win->frame_w-2+lp;	rect[lp+1].y = lp;
		  rect[lp+1].width = 1;	rect[lp+1].height = TITLE_HEIGHT+1-lp;
		  point++;
	  }
	  if( tmp_win->flags&SHADE )
		  for( lp=0; lp<3; lp++ ){
			  rect[lp+4].x = lp;	rect[lp+4].y = tmp_win->frame_h-2+lp;
			  rect[lp+4].width = tmp_win->frame_w+1-lp;	rect[lp+4].height = 1;
			  point++;
		  }
      XShapeCombineRectangles(dpy,tmp_win->frame,ShapeBounding,
                              0,0,rect,point,ShapeUnion,Unsorted);
  }
  if ( tmp_win->flags&RESIZER ){
      rect[0].x = tmp_win->frame_w-SBAR_WH-2-1-(Scr.flags&SYSTEM8?4:0);
      rect[0].y = tmp_win->frame_h-SBAR_WH-2-1-(Scr.flags&SYSTEM8?4:0);
      rect[0].width = SBAR_WH+2+(Scr.flags&SYSTEM8?4:0);
      rect[0].height = SBAR_WH+2+(Scr.flags&SYSTEM8?4:0);
      XShapeCombineRectangles(dpy,tmp_win->frame,ShapeBounding,
                              0,0,rect,1,ShapeUnion,Unsorted);
  }
}

void SetUpFrame( MlvwmWindow *t, int x, int y, int w, int h, Bool sendEvent )
{
	XWindowChanges xwcv;
	unsigned long xwcm;
	int title_height, sbar_v, sbar_h, resize_r;
	int sbar_vh, sbar_hw;

	title_height = t->flags & TITLE ? TITLE_HEIGHT : 0;
	sbar_v = t->flags & SBARV ? SBAR_WH : 0;
	sbar_h = t->flags & SBARH ? SBAR_WH : 0;
	resize_r = t->flags & RESIZER ?
		SBAR_WH : ( sbar_v!=0 && sbar_h!=0 ? SBAR_WH : 0 );

	if( t->flags&TITLE )
		XResizeWindow( dpy, t->title_w,
					  w-2-(t->flags&SBARV?0:1)-(Scr.flags&SYSTEM8?4:0),
					  TITLE_HEIGHT );
	if( t->flags&MINMAXR ){
		if( Scr.flags&SYSTEM8 )
			XMoveWindow(dpy, t->minmax_b,
                        w-BOXSIZE-7-(t->flags&SHADER?BOXSIZE+4:0),
						(TITLE_HEIGHT-BOXSIZE)/2);
		else
			XMoveWindow(dpy, t->minmax_b,
						w-(BOXSIZE*2-2),
						(TITLE_HEIGHT-BOXSIZE)/2);
	}
	if( t->flags&SHADER )
        XMoveWindow(dpy, t->shade_b, w-BOXSIZE-8,
					(TITLE_HEIGHT-BOXSIZE)/2);
	if( !(t->flags&SHADE) ){
		if( t->flags & ( TITLE | SBARV | SBARH | RESIZER ) ){
			t->attr.width = w-sbar_v-(t->flags&SBARV ? 2 : 1 )-1;
			t->attr.height = h-sbar_h-title_height-2-
				(t->flags&TITLE ? 1 : 0)-
					(t->flags&SBARH ? 1 : 0);
			if( Scr.flags&SYSTEM8 ){
				t->attr.width -= 12;
				t->attr.height -= 6;
			}
			XMoveResizeWindow( dpy, t->Parent,
							  Scr.flags&SYSTEM8?5:-1,
							  t->flags & TITLE ? TITLE_HEIGHT : -1,
							  t->attr.width, t->attr.height );
		}
		else{
			t->attr.width = w-12;
			t->attr.height = h-12;
			XMoveResizeWindow( dpy, t->Parent, 5, 5,
							  t->attr.width, t->attr.height );
		}
		if( t->flags&RESIZER )
			XMoveWindow( dpy, t->resize_b, w-SBAR_WH-3-(Scr.flags&SYSTEM8?4:0),
						h-SBAR_WH-3-(Scr.flags&SYSTEM8?4:0) );
 /* Vertical Scroll bar Height */
		sbar_vh = t->attr.height-
			(t->flags&RESIZER && !(t->flags&SBARH)?SBAR_WH+1:0);
 /* Horizontal Scroll width */
		sbar_hw = t->attr.width-
			(t->flags&RESIZER && !(t->flags&SBARV)?SBAR_WH+1:0);

		if( t->flags&SBARV ){
			int anker_position;

			XMoveResizeWindow( dpy, t->scroll_v[0],
							  t->attr.width+(Scr.flags&SYSTEM8?6:0),
							  t->flags & TITLE ? TITLE_HEIGHT : -1,
							  SBAR_WH, sbar_vh );
			XMoveWindow( dpy, t->scroll_v[2], 0, sbar_vh-SBAR_WH );
			if( t->flags&SCROLL ){
				if( t->frame_h<t->win_h )
					anker_position = (double)((sbar_vh-3*SBAR_WH-2)*t->win_y)/
						(double)(t->frame_h-t->win_h)
							+ SBAR_WH+1;
				else
					anker_position = SBAR_WH+2;
				if( anker_position>sbar_vh-3*SBAR_WH-2+SBAR_WH+1 ){
					anker_position = sbar_vh-3*SBAR_WH-2+SBAR_WH+1;
					t->win_y = t->frame_h-t->win_h;
				}
				XMoveWindow( dpy, t->scroll_v[3], 0, anker_position );
			}
		}
		if( t->flags&SBARH ){
			int anker_position;

			XMoveResizeWindow( dpy,t->scroll_h[0], Scr.flags&SYSTEM8?5:-1,
							  h-1-SBAR_WH-2-(Scr.flags&SYSTEM8?6:0),
							  sbar_hw, SBAR_WH );
			XMoveWindow( dpy, t->scroll_h[2], sbar_hw-SBAR_WH, 0 );
			if( t->flags&SCROLL ){
				if( t->frame_w<t->win_w )
					anker_position = (double)( (sbar_hw-3*SBAR_WH-2)*t->win_x)/
						(double)(t->frame_w-t->win_w)
							+ SBAR_WH+1;
				else
					anker_position = SBAR_WH+2;
				if( anker_position>sbar_hw-3*SBAR_WH-2+SBAR_WH+1 ){
					anker_position = sbar_hw-3*SBAR_WH-2+SBAR_WH+1;
					t->win_x = t->frame_w-t->win_w;
				}
				XMoveWindow( dpy, t->scroll_h[3], anker_position, 0 );
			}
		}
	}
	if( t->flags & SCROLL ){
		if( t->flags & SBARH ){
			t->attr.width = w>t->win_w ? t->attr.width :
				t->win_w-sbar_v-(t->flags&SBARV ? 2 : 1 )-1
					-(Scr.flags&SYSTEM8?12:0);
			t->win_x = w>t->win_w ? 0 : t->win_x;
		}
		if( t->flags & SBARV ){
			t->attr.height = h>t->win_h ? t->attr.height : 
				t->win_h-sbar_h-title_height-2-
					(t->flags&TITLE ? 1 : 0)-(t->flags&SBARH ? 1 : 0)
					-(Scr.flags&SYSTEM8?6:0);
			t->win_y = h>t->win_h ? 0 : t->win_y;
		}
	}
	else{
		t->win_x = 0;
		t->win_y = 0;
	}
	XMoveResizeWindow( dpy, t->w, t->win_x, t->win_y,
					  t->attr.width, t->attr.height );

	xwcm = (CWWidth | CWHeight | CWX | CWY );
	xwcv.x = x;
	xwcv.y = y;
	xwcv.width = w;
	xwcv.height = h;
	XConfigureWindow( dpy, t->frame, xwcm, &xwcv );

	if( (!(t->wShaped) || t->flags&SHADE) &&
	   t->flags & ( TITLE | SBARV | SBARH | RESIZER )){
		int lp;
		XRectangle rect[5];

		rect[0].x = -1;		rect[0].y = -1;
		rect[0].width = t->frame_w-1;	rect[0].height = t->frame_h-1;

		for( lp=0; lp<2; lp++ ){
			rect[lp+1].x = t->frame_w-2+lp;	rect[lp+1].y = lp;
			rect[lp+1].width = 1;	rect[lp+1].height = t->frame_h-lp;
		}
		for( lp=0; lp<2; lp++ ){
			rect[lp+3].x = lp;			rect[lp+3].y = t->frame_h-2+lp;
			rect[lp+3].width = t->frame_w-lp;	rect[lp+3].height = 1;
		}
		XShapeCombineRectangles(dpy,t->frame,ShapeBounding,
								0,0,rect,5,ShapeSet,YSorted);
	}
	if( t->wShaped )		SetShape( t, w );
	if (sendEvent && !(t->flags&SHADE) ){
		XEvent client_event;

		client_event.type = ConfigureNotify;
		client_event.xconfigure.display = dpy;
		client_event.xconfigure.event = t->w;
		client_event.xconfigure.window = t->w;
      
		if( t->flags&(TITLE|SBARV|SBARH|RESIZER)){
			client_event.xconfigure.x = x + 1 + t->win_x;
			client_event.xconfigure.y = y + title_height + 1 + t->win_y;
			if( Scr.flags&SYSTEM8 )
				client_event.xconfigure.x += 6;
		}
		else{
			client_event.xconfigure.x = x + 6;
			client_event.xconfigure.y = y + 6;
		}
		client_event.xconfigure.width = t->attr.width;
		client_event.xconfigure.height = t->attr.height;
		client_event.xconfigure.border_width =0; /* Modifed */
		client_event.xconfigure.above = t->frame;
		client_event.xconfigure.override_redirect = False;
		XSendEvent(dpy, t->w, False, StructureNotifyMask, &client_event);
    }
}

void FillGradation( Window win )
{
	XClearWindow( dpy, win );
	XDrawRectangle( dpy, win, Scr.BlackGC,
				   1, 1, BOXSIZE-3, BOXSIZE-3 );
	DrawShadowBox( 0, 0, BOXSIZE, BOXSIZE, win, 1,
				  Scr.Gray3GC, Scr.WhiteGC, SHADOW_ALL );
	DrawShadowBox( 4, 4, BOXSIZE-6, BOXSIZE-6, win, BOXSIZE/2-2,
				  Scr.WhiteGC, Scr.WhiteGC, SHADOW_BOTTOM|SHADOW_RIGHT );
	DrawShadowBox( 2, 2, BOXSIZE-6, BOXSIZE-6, win, BOXSIZE/2-2,
				  Scr.Gray3GC, Scr.WhiteGC,  SHADOW_TOP|SHADOW_LEFT );
	DrawShadowBox( 2, 2, 3, 3, win, 2,  Scr.Gray2GC, Scr.WhiteGC,
				  SHADOW_TOP|SHADOW_LEFT );
	DrawShadowBox( 2, 2, BOXSIZE-4, BOXSIZE-4, win, 1,
				  Scr.WhiteGC, Scr.Gray3GC, SHADOW_ALL );
}

void DrawCloseBox( MlvwmWindow *t, Bool on )
{
	unsigned long mask, valuemask;
	XSetWindowAttributes attributes_box;
	XGCValues xgcv;

	mask = GCForeground;
	if( on || Scr.flags&SYSTEM8 ){
		if( !XGetGCValues( dpy,
						  (Scr.flags&SYSTEM8?Scr.Gray4GC:Scr.Gray3GC),
						  mask, &xgcv ) ){
			fprintf( stderr, "Sorry Can not get GC Values 3\n");
			xgcv.foreground = WhitePixel( dpy, Scr.screen );
		}
		attributes_box.background_pixel = xgcv.foreground;
	}
	else
		attributes_box.background_pixel = WhitePixel( dpy, Scr.screen );
	if( on ) attributes_box.cursor = Scr.MlvwmCursors[DESTROY];
	else attributes_box.cursor = Scr.MlvwmCursors[DEFAULT];
	valuemask = CWBackPixel | CWCursor;
	XChangeWindowAttributes( dpy, t->close_b, valuemask, &attributes_box );
	XClearWindow( dpy, t->close_b );
	if( on ){
		if( Scr.flags&SYSTEM8 ){
			XFillRectangle( dpy, t->title_w, Scr.Gray4GC,
						   4, (TITLE_HEIGHT-BOXSIZE)/2-2,
                           BOXSIZE+2, BOXSIZE+2 );
			FillGradation( t->close_b );
		}
		else{
			XFillRectangle( dpy, t->title_w, Scr.Gray4GC,
						   BOXSIZE-4, (TITLE_HEIGHT-BOXSIZE)/2-1,
						   BOXSIZE+2, BOXSIZE+1 );
			DrawShadowBox( 0, 0, BOXSIZE, BOXSIZE, t->close_b, 1,
						  Scr.BlackGC, Scr.WhiteGC, SHADOW_ALL );
			DrawShadowBox( 1, 1, BOXSIZE-2, BOXSIZE-2, t->close_b, 1,
						  Scr.WhiteGC, Scr.BlackGC, SHADOW_ALL );
		}
	}
}

void DrawMinMax( MlvwmWindow *t, Bool on )
{
	unsigned long mask, valuemask;
	XSetWindowAttributes attributes_box;
	XGCValues xgcv;

	mask = GCForeground;
	if( on || Scr.flags&SYSTEM8 ){
		if( !XGetGCValues( dpy,
						  (Scr.flags&SYSTEM8?Scr.Gray4GC:Scr.Gray3GC),
						  mask, &xgcv ) ){
			fprintf( stderr, "Sorry Can not get GC Values 3\n");
			xgcv.foreground = WhitePixel( dpy, Scr.screen );
		}
		attributes_box.background_pixel = xgcv.foreground;
	}
	else
		attributes_box.background_pixel = WhitePixel( dpy, Scr.screen );

	if( on ) attributes_box.cursor = Scr.MlvwmCursors[MINMAX_CURSOR];
	else attributes_box.cursor = Scr.MlvwmCursors[DEFAULT];
	valuemask = CWBackPixel | CWCursor;
	XChangeWindowAttributes( dpy, t->minmax_b, valuemask, &attributes_box );
	XClearWindow( dpy, t->minmax_b );
	if( on ){
		if( Scr.flags&SYSTEM8 ){
			XFillRectangle( dpy, t->title_w, Scr.Gray4GC,
                           t->frame_w-BOXSIZE-9-
						   (t->flags&SHADER?BOXSIZE+6:0),
						   (TITLE_HEIGHT-BOXSIZE)/2-1,
						   BOXSIZE+8, BOXSIZE+2 );
			FillGradation( t->minmax_b );
			XDrawRectangle( dpy, t->minmax_b, Scr.BlackGC,
                           1, 1, BOXSIZE-7, BOXSIZE-7 );
		}
		else{
			XFillRectangle( dpy, t->title_w, Scr.Gray4GC,
						   t->frame_w-(BOXSIZE*2-1),(TITLE_HEIGHT-BOXSIZE)/2-1,
						   BOXSIZE+2, BOXSIZE+1 );
			DrawShadowBox( 0, 0, BOXSIZE, BOXSIZE, t->minmax_b, 1,
						  Scr.BlackGC, Scr.WhiteGC, SHADOW_ALL );
			DrawShadowBox( 1, 1, BOXSIZE-2, BOXSIZE-2, t->minmax_b, 1,
						  Scr.WhiteGC, Scr.BlackGC, SHADOW_ALL );
			DrawShadowBox( 1, 1, BOXSIZE-5, BOXSIZE-5, t->minmax_b, 1,
						  Scr.WhiteGC, Scr.BlackGC, SHADOW_ALL );
		}
	}
}

void DrawShadeR( MlvwmWindow *t, Bool on )
{
	unsigned long mask, valuemask;
	XSetWindowAttributes attributes_box;
	XGCValues xgcv;

	mask = GCForeground;
	if( on || Scr.flags&SYSTEM8 ){
		if( !XGetGCValues( dpy,
						  (Scr.flags&SYSTEM8?Scr.Gray4GC:Scr.Gray3GC),
						  mask, &xgcv ) ){
			fprintf( stderr, "Sorry Can not get GC Values 3\n");
			xgcv.foreground = WhitePixel( dpy, Scr.screen );
		}
		attributes_box.background_pixel = xgcv.foreground;
	}
	else
		attributes_box.background_pixel = WhitePixel( dpy, Scr.screen );

	if( on ){
		if( t->flags&SHADE )
			attributes_box.cursor = Scr.MlvwmCursors[SHADER_DOWN_CURSOR];
		else
			attributes_box.cursor = Scr.MlvwmCursors[SHADER_UP_CURSOR];
	}
	else attributes_box.cursor = Scr.MlvwmCursors[DEFAULT];
	valuemask = CWBackPixel | CWCursor;
	XChangeWindowAttributes( dpy, t->shade_b, valuemask, &attributes_box );
	XClearWindow( dpy, t->shade_b );
	if( on ){
		XFillRectangle( dpy, t->title_w, Scr.Gray4GC, t->frame_w-BOXSIZE-15,
					   (TITLE_HEIGHT-BOXSIZE)/2-1, BOXSIZE+8, BOXSIZE+2 );
		FillGradation( t->shade_b );
		XDrawLine( dpy, t->shade_b, Scr.BlackGC,
				  1, BOXSIZE/2-1, BOXSIZE-3, BOXSIZE/2-1 );
		XDrawLine( dpy, t->shade_b, Scr.BlackGC,
				  1, BOXSIZE/2+1, BOXSIZE-3, BOXSIZE/2+1 );
	}
}

void SetTitleBar( MlvwmWindow *t, Bool on_off )
{
	int w, offset;
	int titlelength, drawable;
	int lp;
	unsigned long mask, valuemask;
	XSetWindowAttributes attributes_title, attributes_box;
	XGCValues xgcv;
	GC dispgc;

	mask = GCForeground;
	if( on_off || Scr.flags&SYSTEM8 ){
		if( !XGetGCValues( dpy, Scr.Gray4GC, mask, &xgcv ) ){
			fprintf( stderr, "Sorry Can not get GC Values 4\n");
			xgcv.foreground = WhitePixel( dpy, Scr.screen );
		}
		attributes_title.background_pixel = xgcv.foreground;
		if( !XGetGCValues( dpy, Scr.Gray3GC, mask, &xgcv ) ){
			fprintf( stderr, "Sorry Can not get GC Values 3\n");
			xgcv.foreground = WhitePixel( dpy, Scr.screen );
		}
		if( Scr.flags&SYSTEM8 )
			attributes_box.background_pixel = WhitePixel( dpy, Scr.screen );
		else
			attributes_box.background_pixel = xgcv.foreground;
	}
	else{
		attributes_title.background_pixel = WhitePixel( dpy, Scr.screen );
		attributes_box.background_pixel = WhitePixel( dpy, Scr.screen );
	}
	if( on_off ) attributes_title.cursor = Scr.MlvwmCursors[TITLE_CURSOR];
	else     attributes_title.cursor = Scr.MlvwmCursors[DEFAULT];
	valuemask = CWBackPixel | CWCursor;
	XChangeWindowAttributes( dpy, t->title_w, valuemask, &attributes_title );
	XClearWindow( dpy, t->title_w );

	drawable = t->frame_w-14*4;
	if( t->flags&CLOSER || t->flags&MINMAXR )
		drawable-=BOXSIZE*2;
	if( t->flags&SHADER && t->flags&MINMAXR )
		drawable-=BOXSIZE*2;

	titlelength = strlen( t->name )+1;
	do{
		titlelength--;
		StrWidthHeight( WINDOWFONT, &w, NULL, &offset, t->name, titlelength );
	}
	while( w+20>drawable && titlelength>0 );

	if( on_off ){
        for( lp=4; lp<16; lp+=2 ){
			if( Scr.d_depth>1 ){
                if( Scr.flags&SYSTEM8 )
                    DrawShadowBox( 4, lp-1, t->frame_w-14, 2, t->title_w, 1,
								  Scr.WhiteGC, Scr.Gray1GC, SHADOW_ALL );
                else
                    XDrawLine( dpy, t->title_w, Scr.Gray1GC, 0, lp, t->frame_w, lp );
            }
            else
              XDrawLine( dpy, t->title_w, Scr.BlackGC, 0, lp, t->frame_w, lp );
        }

		if( Scr.flags&SYSTEM8 ){
			DrawShadowBox( 0, 0, t->frame_w-2, TITLE_HEIGHT, t->title_w,
						  1, Scr.WhiteGC, Scr.Gray2GC, SHADOW_TOP );
			if( !(t->flags&SHADE) )
				XDrawLine( dpy, t->title_w, Scr.Gray2GC,
						  2, TITLE_HEIGHT-1, t->frame_w-9, TITLE_HEIGHT-1 );
		}
		else
			DrawShadowBox( 0, 0, t->frame_w-2, TITLE_HEIGHT, t->title_w,
						  1, Scr.WhiteGC, Scr.Gray2GC, SHADOW_ALL );
		XFillRectangle( dpy, t->title_w, Scr.Gray4GC,
                       (t->frame_w-w)/2-5, 1, w+10, TITLE_HEIGHT-2 );
		dispgc = Scr.BlackGC;
	}
	else{
		if( Scr.d_depth>1 )			dispgc = Scr.Gray3GC;
		else			dispgc = Scr.BlackGC;
	}
	if( t->flags&CLOSER )		DrawCloseBox( t, on_off );
	if( t->flags&MINMAXR )		DrawMinMax( t, on_off );
	if( t->flags&SHADER )		DrawShadeR( t, on_off );

	XDRAWSTRING( dpy, t->title_w, WINDOWFONT, dispgc, (t->frame_w-w)/2,
				TITLE_HEIGHT/2-offset, t->name, titlelength );
	if( Scr.d_depth<2 && !on_off ){
		xgcv.function = GXor;
		mask = GCFunction;
		XChangeGC( dpy, Scr.BlackGC, mask, &xgcv );
		XSetFillStyle( dpy, Scr.BlackGC, FillTiled );
		XFillRectangle( dpy, t->title_w, Scr.BlackGC, (t->frame_w-w)/2, 1,
					   w, TITLE_HEIGHT-2 );
		XSetFillStyle( dpy, Scr.BlackGC, FillSolid );
		xgcv.function = GXcopy;
		XChangeGC( dpy, Scr.BlackGC, mask, &xgcv );
	}
}

void DrawArrow( Window w, int direction, GC fill, GC outline )
{
	XPoint arrow_p[] = {
		{7,3},{2,8},{5,8},{5,12},{9,12},{9,8},{12,8},{7,3}, {0,0}},
	arrow_p8[] = {{7,6},{4,9},{11,9},{8,6},{7,6}, {0,0}};
	XPoint *use_p;
	int lp, JunkX, point;

	if( Scr.flags&SYSTEM8 )		use_p = arrow_p8;
	else		use_p = arrow_p;

	for( point=0; use_p[point].x!=0; point++ ){
		use_p[point].x = use_p[point].x*SBAR_WH/16.;
		use_p[point].y = use_p[point].y*SBAR_WH/16.;
	}

	if( direction==C_SBAR_LEFT || direction==C_SBAR_RIGHT ){
		for( lp=0; lp<point; lp++ ){
			JunkX = use_p[lp].x;
			if( direction==C_SBAR_LEFT )
				use_p[lp].x = use_p[lp].y;
			else
				use_p[lp].x = SBAR_WH-use_p[lp].y-1;
			use_p[lp].y = JunkX;
		}
	}
	if( direction==C_SBAR_DOWN )
		for( lp=0; lp<point; lp++ )		use_p[lp].y = SBAR_WH-use_p[lp].y-1;

	XFillPolygon( dpy, w, fill, use_p, point, Nonconvex, CoordModeOrigin );
	XDrawLines( dpy, w, outline, use_p, point, CoordModeOrigin );
}

void DrawSbarAnk( MlvwmWindow *t, int context, Bool on_off )
{
	Window	win;
	int size, scale, lp;
	unsigned int mask, valuemask;
	XGCValues xgcv;
	XSetWindowAttributes attributes;

	if( context==C_SBAR_H_AN ){
		 win = t->scroll_h[3];
		size = t->frame_w-t->win_w;
	}
	if( context==C_SBAR_V_AN ){
		 win = t->scroll_v[3];
		size = t->frame_h-t->win_h;
	}

	if( on_off ){
		if( t->flags&SCROLL && size<0 ){
			mask = GCForeground;
			if( !XGetGCValues( dpy, Scr.flags&SYSTEM8?
					Scr.ScrollBlueGC:Scr.Gray4GC,
					mask, &xgcv ) ){
				fprintf( stderr, "Sorry Can not get GC Values 4\n");
				xgcv.foreground = WhitePixel( dpy, Scr.screen );
			}
			attributes.background_pixel = xgcv.foreground;
			attributes.cursor = Scr.MlvwmCursors[DEFAULT];
			valuemask = CWBackPixel | CWCursor;

			valuemask = CWBackPixel;
			XChangeWindowAttributes( dpy, win, valuemask, &attributes );
			XClearWindow( dpy, win );

			DrawShadowBox( 0, 0, SBAR_WH, SBAR_WH, win, 1,
						  Scr.WhiteGC, Scr.Gray2GC, SHADOW_ALL );
			scale = 8*SBAR_WH/16.;
			if( context==C_SBAR_H_AN ){
				for( lp=0; lp<(SBAR_WH-8)/2; lp++ )
					DrawShadowBox( (lp+2)*2, (SBAR_WH-scale)/2,
								  2, scale, win, 1,
								  Scr.WhiteGC, Scr.BlackGC, SHADOW_ALL );
			}
			if( context==C_SBAR_V_AN ){
				for( lp=0; lp<(SBAR_WH-8)/2; lp++ )
					DrawShadowBox( (SBAR_WH-scale)/2, (lp+2)*2,
								  scale, 2, win, 1,
								  Scr.WhiteGC, Scr.BlackGC, SHADOW_ALL );
			}
		}
		else{
			mask = GCForeground;
			if( !XGetGCValues( dpy, Scr.Gray4GC, mask, &xgcv ) ){
				fprintf( stderr, "Sorry Can not get GC Values 4\n");
				xgcv.foreground = WhitePixel( dpy, Scr.screen );
			}
			attributes.background_pixel = xgcv.foreground;
			attributes.cursor = Scr.MlvwmCursors[DEFAULT];
			valuemask = CWBackPixel | CWCursor;

			XChangeWindowAttributes(dpy, win, valuemask, &attributes);
			XClearWindow( dpy, win );
		}
	}
	else{
		attributes.background_pixel = WhitePixel( dpy, Scr.screen );
		attributes.cursor = Scr.MlvwmCursors[DEFAULT];
		valuemask = CWBackPixel | CWCursor;
		XChangeWindowAttributes(dpy, win, valuemask, &attributes);
		XClearWindow( dpy, win );
	}
}

void DrawSbarArrow( MlvwmWindow *t, int context, Bool on_off )
{
	int size;
	unsigned int mask, valuemask;
	Window win;
	XGCValues xgcv;
	XSetWindowAttributes attributes;
	Cursor	cursor;
	GC fill, outline;

	switch( context ){
	case C_SBAR_UP:
		win = t->scroll_v[1];
		size = t->frame_h-t->win_h;
		cursor = Scr.MlvwmCursors[SBARV_CURSOR];
		break;
	case C_SBAR_DOWN:
		win = t->scroll_v[2];
		size = t->frame_h-t->win_h;
		cursor = Scr.MlvwmCursors[SBARV_CURSOR];
		break;
	case C_SBAR_LEFT:
		win = t->scroll_h[1];
		size = t->frame_w-t->win_w;
		cursor = Scr.MlvwmCursors[SBARH_CURSOR];
		break;
	case C_SBAR_RIGHT:
		win = t->scroll_h[2];
		size = t->frame_w-t->win_w;
		cursor = Scr.MlvwmCursors[SBARH_CURSOR];
		break;
	}

	if( on_off ){
		mask = GCForeground;
		if( !XGetGCValues( dpy, Scr.Gray4GC, mask, &xgcv ) ){
			fprintf( stderr, "Sorry Can not get GC Values 4\n");
			xgcv.foreground = WhitePixel( dpy, Scr.screen );
		}
		attributes.background_pixel = xgcv.foreground;
		if( t->flags&SCROLL && size<0 )
			attributes.cursor = cursor;
		else
			attributes.cursor = Scr.MlvwmCursors[DEFAULT];
		valuemask = CWBackPixel | CWCursor;

		XChangeWindowAttributes( dpy, win, valuemask, &attributes );
		XClearWindow( dpy, win );

		if( t->flags&SCROLL && size<0 ) {
			DrawShadowBox( 0, 0, SBAR_WH, SBAR_WH, win, 1,
						  Scr.WhiteGC, Scr.Gray2GC, SHADOW_ALL );
			if( Scr.flags&SYSTEM8 )
				fill = outline = Scr.BlackGC;
			else {
				fill = Scr.Gray3GC;
				outline = Scr.BlackGC;
			}
		}
		else{
			fill = outline = Scr.Gray2GC;
 			if( !(Scr.flags&SYSTEM8) )
				fill = Scr.Gray4GC;
		}
		DrawArrow( win, context, fill, outline);
	}
	else{
		mask = GCForeground;
		attributes.background_pixel = WhitePixel( dpy, Scr.screen );
		attributes.cursor = Scr.MlvwmCursors[DEFAULT];
		valuemask = CWBackPixel | CWCursor;
		XChangeWindowAttributes( dpy, win, valuemask, &attributes );
		XClearWindow( dpy, win );
	}
}

void DrawSbarBar( MlvwmWindow *t, int context, Bool on_off )
{
	unsigned long mask, valuemask;
	XSetWindowAttributes attributes;
	XWindowAttributes winattrs;
	XGCValues xgcv;
	Window win;
	int x, y, width_f, height_f, width, height, size, mesh_w, mesh_h;
	char *mesh;

	if( context==C_SBAR_H ) win = t->scroll_h[0];
	if( context==C_SBAR_V ) win = t->scroll_v[0];

	XSetWindowBackgroundPixmap( dpy, win, None );
	XGetWindowAttributes(dpy, win, &winattrs);
	if( context==C_SBAR_H ){
		size = t->frame_w-t->win_w;
		x = SBAR_WH+1;
		y = 0;
		width_f = winattrs.width;
		height_f = SBAR_WH;
		width = winattrs.width-SBAR_WH*2-2;
		height = SBAR_WH;
		mesh = mesh_bits_h;
		mesh_w = 2;
		mesh_h = 4;
	}
	if( context==C_SBAR_V ){
		size = t->frame_h-t->win_h;
		x = 0;
		y = SBAR_WH+1;
		width_f = SBAR_WH;
		height_f = winattrs.height;
		width = SBAR_WH;
		height = winattrs.height-SBAR_WH*2-2;
		mesh = mesh_bits_v;
		mesh_w = 4;
		mesh_h = 2;
	}

	if( on_off ){
		if( t->flags&SCROLL && size<0 ){
			if( Scr.flags&SYSTEM8 ){
				Pixmap bgpix;
				bgpix = XCreatePixmap( dpy, Scr.Root, width_f,
					height_f, Scr.d_depth );
				XFillRectangle(dpy,bgpix,Scr.Gray3GC,0,0,width_f,height_f);

				DrawShadowBox( x, y, width, height, bgpix, 1,
							  Scr.Gray2GC, Scr.WhiteGC, SHADOW_ALL );
				XSetWindowBackgroundPixmap( dpy, win, bgpix );
				XFreePixmap( dpy, bgpix );
			}
			else{
				XSetWindowBackgroundPixmap( dpy, win,
                     XCreatePixmapFromBitmapData( dpy, Scr.Root,
												 mesh, mesh_w, mesh_h,
												 BlackPixel(dpy,Scr.screen),
												 WhitePixel(dpy,Scr.screen),
												 Scr.d_depth ));
			}
			XClearWindow( dpy, win );
		}
		else{
			mask = GCForeground;
			if( !XGetGCValues( dpy, Scr.Gray4GC, mask, &xgcv ) ){
				fprintf( stderr, "Sorry Can not get GC Values 4\n");
				xgcv.foreground = WhitePixel( dpy, Scr.screen );
			}
			attributes.background_pixel = xgcv.foreground;
			attributes.cursor = Scr.MlvwmCursors[DEFAULT];

			valuemask = CWBackPixel | CWCursor;
			XChangeWindowAttributes( dpy, win, valuemask, &attributes );
			XClearWindow( dpy, win );
		}
		if( context==C_SBAR_V ){
			XDrawLine(dpy, win, Scr.BlackGC, x, y-1, width, y-1);
			XDrawLine(dpy, win, Scr.BlackGC, x, y+height, width, y+height);
		}
		if( context==C_SBAR_H ){
			XDrawLine(dpy, win, Scr.BlackGC, x-1, y, x-1, height);
			XDrawLine(dpy, win, Scr.BlackGC, x+width, y, x+width, height);
		}
	}
	else{
		mask = GCForeground;
		attributes.background_pixel = WhitePixel( dpy, Scr.screen );
		attributes.cursor = Scr.MlvwmCursors[DEFAULT];
		valuemask = CWBackPixel | CWCursor;
		XChangeWindowAttributes( dpy, win, valuemask, &attributes );
		XClearWindow( dpy, win );
	}
}

void DrawFrameShadow( MlvwmWindow *t, Bool on )
{
	XSegment lines[4];
	int lp;
	unsigned long valuemask;
	XSetWindowAttributes attributes;

	if( on ) attributes.cursor = Scr.MlvwmCursors[TITLE_CURSOR];
	else attributes.cursor = Scr.MlvwmCursors[DEFAULT];
	valuemask = CWCursor;
	XChangeWindowAttributes( dpy, t->frame, valuemask, &attributes );
	if( t->flags&( TITLE | SBARV | SBARH | RESIZER )){
		for( lp=0; lp<2; lp++ )
			SetSegment(0,t->frame_w,t->frame_h-lp-1,
					   t->frame_h-lp-1,lines+lp);
		for( lp=0; lp<2; lp++ )
			SetSegment(t->frame_w-lp-1,t->frame_w-lp-1,0,
					   t->frame_h,lines+lp+2);
		XDrawSegments( dpy, t->frame, Scr.BlackGC, lines, 4 );
		if( Scr.flags&SYSTEM8 ){
			if( on ){
				DrawShadowBox( 0, 0, t->frame_w-2, t->frame_h-2, t->frame, 5,
							  Scr.WhiteGC, Scr.WhiteGC,
							  SHADOW_LEFT|SHADOW_BOTTOM|SHADOW_RIGHT );
				DrawShadowBox( 0, 0, t->frame_w-2, t->frame_h-2, t->frame, 2,
							  Scr.WhiteGC, Scr.Gray3GC,
							  SHADOW_LEFT|SHADOW_BOTTOM|SHADOW_RIGHT );
				DrawShadowBox( 2, 0, t->frame_w-6, t->frame_h-4, t->frame, 2,
							  Scr.Gray4GC, Scr.Gray4GC,
							  SHADOW_LEFT|SHADOW_BOTTOM|SHADOW_RIGHT );
				XDrawLine( dpy, t->frame, Scr.Gray2GC,
						  4, 0, 4, t->frame_h-7 );
				DrawShadowBox( t->frame_w-SBAR_WH-8, t->frame_h-SBAR_WH-8,
							  SBAR_WH+1, SBAR_WH+1, t->frame, 1,
							  Scr.WhiteGC, Scr.Gray4GC,
							  SHADOW_LEFT|SHADOW_TOP );
			}
			else{
				DrawShadowBox( 0, 0, t->frame_w-2, t->frame_h-2, t->frame, 5,
							  Scr.Gray4GC, Scr.Gray4GC,
							  SHADOW_ALL );
				DrawShadowBox( t->frame_w-SBAR_WH-8, t->frame_h-SBAR_WH-8,
							  SBAR_WH+1, SBAR_WH+1, t->frame, 1,
							  Scr.Gray4GC, Scr.Gray4GC,
							  SHADOW_LEFT|SHADOW_TOP );
			}
		}
	}
	else{
		DrawShadowBox( 0, 0, t->frame_w, t->frame_h, t->frame, 2,
					  Scr.WhiteGC, Scr.Gray3GC, SHADOW_ALL );
		DrawShadowBox( 3, 3, t->frame_w-6, t->frame_h-6, t->frame, 2,
					  Scr.Gray3GC, Scr.WhiteGC, SHADOW_ALL );
		if( Scr.flags&SYSTEM8 )
			DrawShadowBox( 2, 2, t->frame_w-4, t->frame_h-4, t->frame, 2,
						  Scr.Gray4GC, Scr.Gray4GC, SHADOW_ALL );
	}
}

void DrawResizeBox( MlvwmWindow *t, Bool on_off )
{
	unsigned long mask, valuemask;
	XSetWindowAttributes attributes;
	XGCValues xgcv;
	int point, scale, lp;

	mask = GCForeground;
	if( on_off || Scr.flags&SYSTEM8 ){
		if( !XGetGCValues( dpy, Scr.Gray4GC, mask, &xgcv ) ){
			fprintf( stderr, "Sorry Can not get GC Values 4\n");
			xgcv.foreground = WhitePixel( dpy, Scr.screen );
		}
		attributes.background_pixel = xgcv.foreground;
	}
	else
		attributes.background_pixel = WhitePixel( dpy, Scr.screen );
	if( on_off ) attributes.cursor = Scr.MlvwmCursors[RESIZE];
	else     attributes.cursor = Scr.MlvwmCursors[DEFAULT];
	valuemask = CWBackPixel | CWCursor;

	XChangeWindowAttributes( dpy, t->resize_b, valuemask, &attributes );

	XClearWindow( dpy, t->resize_b );
	if( on_off ){
		if( Scr.flags&SYSTEM8 ){
			for(lp=0; lp<3; lp++ ){
				XDrawLine( dpy, t->resize_b, Scr.WhiteGC,
						  3+lp*2, 8+lp*2, 7+lp*2, 4+lp*2 );
				XDrawLine( dpy, t->resize_b, Scr.BlackGC,
						  4+lp*2, 8+lp*2, 8+lp*2, 4+lp*2 );
			}
		}
		else{
			point = 4*SBAR_WH/16.;
			scale = 9*SBAR_WH/16.;
			DrawShadowBox( point, point,  scale,  scale, t->resize_b,
						  1, Scr.WhiteGC, Scr.BlackGC, SHADOW_ALL );
			DrawShadowBox( point-1, point-1, scale+2, scale+2, t->resize_b,
						  1, Scr.BlackGC, Scr.WhiteGC, SHADOW_ALL );
			
			point = 3*SBAR_WH/16.;
			scale = 6*SBAR_WH/16.;
			XFillRectangle( dpy, t->resize_b, Scr.Gray4GC,
						   point, point, scale, scale );

			point = 3*SBAR_WH/16.;
			scale = 5*SBAR_WH/16.;
			DrawShadowBox( point, point,  scale,  scale, t->resize_b,
						  1, Scr.WhiteGC, Scr.BlackGC, SHADOW_ALL );
			DrawShadowBox( point-1, point-1, scale+2, scale+2, t->resize_b,
						  1, Scr.BlackGC, Scr.WhiteGC, SHADOW_ALL );
		}
	}
	if( Scr.flags&SYSTEM8 && !(t->flags&(SBARV|SBARH)) ){
		DrawShadowBox( 0, 0,  SBAR_WH, SBAR_WH, t->resize_b,
					  1, Scr.BlackGC, Scr.BlackGC,
					  SHADOW_TOP|SHADOW_LEFT );
		if( on_off )
			DrawShadowBox( 1, 1,  SBAR_WH-1, SBAR_WH-1, t->resize_b,
						  1, Scr.WhiteGC, Scr.WhiteGC,
						  SHADOW_TOP|SHADOW_LEFT );
	}
}

void DrawAllDecorations( MlvwmWindow *t, Bool on_off )
{
	if( t->flags&SBARV ){
		DrawSbarBar( t, C_SBAR_V, on_off );
		DrawSbarAnk( t, C_SBAR_V_AN, on_off );
		DrawSbarArrow( t, C_SBAR_UP, on_off );
		DrawSbarArrow( t, C_SBAR_DOWN, on_off );
	}
	if( t->flags&SBARH ){
		DrawSbarBar( t, C_SBAR_H, on_off );
		DrawSbarAnk( t, C_SBAR_H_AN, on_off );
		DrawSbarArrow( t, C_SBAR_LEFT, on_off );
		DrawSbarArrow( t, C_SBAR_RIGHT, on_off );
	}
	if( t->flags&RESIZER )
		DrawResizeBox( t, on_off );
	if( t->flags&TITLE )
		SetTitleBar( t, on_off );
	if( Scr.flags&SYSTEM8 )
		DrawFrameShadow( t, on_off );

	XSync( dpy, 0 );
}

void SetFocus( MlvwmWindow *t )
{
	char *str, action[24], *winname;
	size_t str_size;
	unsigned long mask, valuemask;
	XSetWindowAttributes attributes;
	XGCValues xgcv;

	mask = GCForeground;

	if( Scr.ActiveWin==t )		return;
	if( Scr.ActiveWin ){
		DrawAllDecorations( Scr.ActiveWin, False );

		if( Scr.ActiveWin->flags&(SBARV|SBARH) &&
		   !(Scr.ActiveWin->flags&RESIZER) && !(Scr.flags&SYSTEM8) ){
			attributes.background_pixel = WhitePixel( dpy, Scr.screen );
			valuemask = CWBackPixel;
			XChangeWindowAttributes( dpy, Scr.ActiveWin->frame,
									valuemask, &attributes );
			XClearArea( dpy, Scr.ActiveWin->frame, 
					   Scr.ActiveWin->frame_w-2-SBAR_WH,
					   Scr.ActiveWin->frame_h-2-SBAR_WH,
					   SBAR_WH, SBAR_WH, False );
		}
			
		if( !(Scr.flags&FOLLOWTOMOUSE) ){
			XSync(dpy,0);
			XGrabButton(dpy, AnyButton, AnyModifier, Scr.ActiveWin->frame,
						True, ButtonPressMask, GrabModeSync,GrabModeAsync,None,
						Scr.MlvwmCursors[DEFAULT]);
		}
		winname = WinListName( Scr.ActiveWin );
		sprintf( action, "Select %lX", (unsigned long)Scr.ActiveWin );
		ChangeMenuItemLabel( "ICON", winname, winname,
							action, ~CHECKON, M_AND );
		free( winname );
	}

	Scr.ActiveWin=t;
	MapMenuBar( Scr.ActiveWin );
	if( t==NULL || t->flags&TRANSIENT ){
		ChangeMenuLabel( &(Scr.IconMenu), NULL, Scr.SystemIcon );
		ChangeMenuItemLabel( "ICON", Scr.IconMenu.m_item->label,
							Scr.IconMenu.m_item->label, NULL, STRGRAY, M_COPY);
		ChangeMenuItemLabel( "ICON", "Hide Others", "Hide Others",
							NULL, STRGRAY, M_COPY );
		XSetInputFocus( dpy, Scr.NoFocusWin, RevertToParent, CurrentTime );
		if( t==NULL )
			return;
	}
	ChangeMenuLabel( &(Scr.IconMenu), NULL, 
					t->miniicon==NULL?Scr.SystemIcon:t->miniicon );
	if( !(t->flags&TRANSIENT) ){
		str_size = strlen(t->name)+6;
		str = calloc( str_size, 1 );
		snprintf( str, str_size, "Hide %s", t->name );
		ChangeMenuItemLabel( "ICON", Scr.IconMenu.m_item->label,
							str, NULL, SELECTON, M_COPY );
		ChangeMenuItemLabel( "ICON", "Hide Others", "Hide Others",
							NULL, SELECTON, M_COPY );
		sprintf( action, "Select %lX", (unsigned long)t );
		winname = WinListName( t );
		ChangeMenuItemLabel( "ICON", winname, winname,
							action, SELECTON|CHECKON, M_COPY );
		free( winname );
		free( str );
	}

	DrawAllDecorations( t, True );

	if( t->flags&(SBARV|SBARH) && !(t->flags&RESIZER) ){
		if( !XGetGCValues( dpy, Scr.Gray4GC, mask, &xgcv ) ){
			fprintf( stderr, "Sorry Can not get GC Values 4\n");
			xgcv.foreground = WhitePixel( dpy, Scr.screen );
		}
		attributes.background_pixel = xgcv.foreground;
		valuemask = CWBackPixel;
		XChangeWindowAttributes( dpy, t->frame, valuemask, &attributes );
		XClearArea( dpy, t->frame, 
				   t->frame_w-2-SBAR_WH-1-(Scr.flags&SYSTEM8?6:0),
				   t->frame_h-2-SBAR_WH-1-(Scr.flags&SYSTEM8?6:0),
				   SBAR_WH+(Scr.flags&SYSTEM8?2:0),
				   SBAR_WH+(Scr.flags&SYSTEM8?2:0), False );
	}

	if( !(Scr.flags&FOLLOWTOMOUSE) )
		XUngrabButton( dpy, AnyButton, AnyModifier, t->frame );

	if( !(t->flags&SHADE) )
		XSetInputFocus( dpy, t->w, RevertToParent, CurrentTime );
	else
		XSetInputFocus( dpy, Scr.NoFocusWin, RevertToParent, CurrentTime );
}
