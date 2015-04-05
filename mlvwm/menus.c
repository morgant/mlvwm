/****************************************************************************/
/* This module is all original code                                         */
/* by TakaC Hasegawa (tak@bioele.nuee.nagoya-u.ac.jp                        */
/* Copyright 1996, TakaC Hasegawa                                           */
/*     You may use this code for any purpose, as long as the original       */
/*     copyright remains in the source code and all documentation           */
/****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mlvwm.h"
#include "screen.h"
#include "menus.h"
#include "event.h"
#include "functions.h"
#include "misc.h"
#include "add_window.h"
#include "borders.h"

#include <X11/Xatom.h>
#include <X11/Xresource.h>

#include <X11/extensions/shape.h>

void RedrawMenu( MenuLabel *m, Bool onoroff )
{
	GC tmp_f_GC, tmp_b_GC;
	int width, offset;
	unsigned long gcm;
	XGCValues gcv;
	Pixel fore_pixel, back_pixel;

	if( Scr.flags&SYSTEM8 ){
		gcm = GCForeground;
		if( !XGetGCValues( dpy, Scr.MenuBlueGC, gcm, &gcv ) ){
			fprintf( stderr, "Sorry Can not get GC Values Blue\n");
			gcv.foreground = WhitePixel( dpy, Scr.screen );
		}
		fore_pixel = gcv.foreground;
		if( !XGetGCValues( dpy, Scr.MenuSelectBlueGC, gcm, &gcv ) ){
			fprintf( stderr, "Sorry Can not get GC Values SelectBlue\n");
			gcv.foreground = WhitePixel( dpy, Scr.screen );
		}
		back_pixel = gcv.foreground;
	}
	else{
		fore_pixel = WhitePixel( dpy, Scr.screen );
		back_pixel = BlackPixel( dpy, Scr.screen );
	}

	if( onoroff )
		XSetWindowBackground( dpy, m->LabelWin, back_pixel );
	else
		XSetWindowBackground( dpy, m->LabelWin, fore_pixel );
	XClearWindow( dpy, m->LabelWin );

	if( m->LabelStr )
		StrWidthHeight( MENUBARFONT, &width, NULL, &offset,
					   m->LabelStr, strlen(m->LabelStr));

	if( onoroff ){
		tmp_f_GC = Scr.WhiteGC;
		tmp_b_GC = Scr.BlackGC;
	}
	else{
		tmp_f_GC = Scr.BlackGC;
		tmp_b_GC = Scr.WhiteGC;
	}
	if( !(m->flags&ACTIVE) && Scr.d_depth>1 )
		tmp_f_GC = Scr.Gray3GC;
	if( !m->xpm ){
		if( m->LabelStr ){
			XDRAWSTRING( dpy, m->LabelWin, MENUBARFONT, tmp_f_GC,
						(m->LabelWidth-width)/2,
						(MENUB_H-2)/2-offset,
						m->LabelStr, strlen(m->LabelStr) );
/* Mask String for Mono Display */
			if( !(m->flags&ACTIVE) && Scr.d_depth<2 ){
				if( onoroff )		gcv.function = GXand;
				else				gcv.function = GXor;
				gcm = GCFunction;
				XChangeGC( dpy, Scr.BlackGC, gcm, &gcv );
				XSetFillStyle( dpy, Scr.BlackGC, FillTiled );
				XFillRectangle( dpy, m->LabelWin, Scr.BlackGC, 0, 0,
							   m->LabelWidth, MENUB_H);
				XSetFillStyle( dpy, Scr.BlackGC, FillSolid );
				gcv.function = GXcopy;
				XChangeGC( dpy, Scr.BlackGC, gcm, &gcv );
			}
		}
	}
	else{
		if( m->xpm->kind==PIXMAP ){
			XSetClipMask( dpy, tmp_f_GC, m->xpm->mask );
			XSetClipOrigin( dpy, tmp_f_GC,
						   (m->LabelWidth-m->xpm->width)/2,
						   (MENUB_H-2-m->xpm->height)/2 );
			XCopyArea( dpy, m->xpm->icon, m->LabelWin, tmp_f_GC, 0, 0,
					  m->xpm->width, m->xpm->height,
					  (m->LabelWidth-m->xpm->width)/2,
					  (MENUB_H-2-m->xpm->height)/2  );
			XSetClipMask( dpy, tmp_f_GC, None );
		}
		else{
			XCopyArea( dpy, onoroff?m->xpm->mask:m->xpm->icon, m->LabelWin,
					  tmp_f_GC, 0, 0,
					  m->xpm->width, m->xpm->height,
					  (m->LabelWidth-m->xpm->width)/2,
					  (MENUB_H-2-m->xpm->height)/2  );
		}
	}
}

void RedrawMenuBar( void )
{
	int lp;

	if( Scr.flags&SYSTEM8 )
		DrawShadowBox( 0, 0, Scr.MyDisplayWidth, MENUB_H, Scr.MenuBar, 1,
					  Scr.WhiteGC, Scr.Gray1GC, SHADOW_ALL );
	XFillRectangle( dpy, Scr.MenuBar, Scr.BlackGC, 0, 0, 7, 7 );
	XFillRectangle( dpy, Scr.MenuBar,
				   Scr.BlackGC, Scr.MyDisplayWidth-7, 0, 7, 7 );
	if( Scr.flags&SYSTEM8 && !(Scr.flags&STARTING) ){
		for( lp=0; lp<2; lp++ ){
			XDrawArc( dpy, Scr.MenuBar, Scr.WhiteGC, 0, 0,
					 14-lp, 14-lp, 180*64, -(90*64) );
			XDrawArc( dpy, Scr.MenuBar, Scr.Gray1GC, Scr.MyDisplayWidth-15, 0,
					 14-lp, 14-lp, 0, 90*64 );
		}
		XFillArc( dpy, Scr.MenuBar, Scr.MenuBlueGC, 0, 0,
				 14, 14, 180*64, -(90*64) );
		XFillArc( dpy, Scr.MenuBar, Scr.MenuBlueGC, Scr.MyDisplayWidth-15, 0,
				 14, 14, 0, 90*64 );
	}
	else{
		XFillArc( dpy, Scr.MenuBar, Scr.WhiteGC, 0, 0,
				 14, 14, 180*64, -(90*64) );
		XFillArc( dpy, Scr.MenuBar, Scr.WhiteGC, Scr.MyDisplayWidth-15, 0,
				 14, 14, 0, 90*64 );
	}
	if( !(Scr.flags&SYSTEM8) )
		XDrawLine( dpy, Scr.MenuBar, Scr.BlackGC,
				  0, MENUB_H-1, Scr.MyDisplayWidth, MENUB_H-1 );
}

void DrawStringMenuBar( char *str )
{
	static int width=0;
	int offset;

	if( width!=0 ){
		XFillRectangle( dpy, Scr.MenuBar, Scr.WhiteGC,
					   (Scr.MyDisplayWidth-width)/2, 0, width, MENUB_H-1 );
	}
	if( strlen( str )!=0 ){
		StrWidthHeight( MENUBARFONT, &width, NULL, &offset,
					   str, strlen(str));
		XDRAWSTRING( dpy, Scr.MenuBar, MENUBARFONT, Scr.BlackGC,
					  (Scr.MyDisplayWidth-width)/2,
					  MENUB_H/2-offset, str, strlen(str) );
	}
	else
		XClearWindow( dpy, Scr.MenuBar );
	RedrawMenuBar();
	XSync( dpy, 0 );
}

/* X 標準の関数で代用可能 */
Bool isRect( int px, int py, int x, int y, int width, int height )
{
	if( px>x && px<x+width && py>y && py<y+height )		return True;
	else										return False;
}

void DrawCheckMark( MenuLabel *ml, GC gc, int topy )
{
	XPoint check[] = {{3,5},{7,9},{16,0},{16,1},{7,10},{3,6}};
	int lp;

	for( lp=0; lp<6; lp++ )
		check[lp].y += (topy+ml->ItemHeight/2-5);
	XDrawLines( dpy, ml->PullWin, gc, check, 6, CoordModeOrigin );
}

void DrawSubMenuMark( MenuLabel *ml, GC gc, int topy )
{
	XPoint mark[] = {{15, -7}, {15, 7}, {7,0}};
	int lp;

	for( lp=0; lp<3; lp++ ){
		mark[lp].x = ml->MenuWidth-mark[lp].x;
		mark[lp].y += (topy+ml->ItemHeight/2);
	}
	XFillPolygon( dpy, ml->PullWin, gc, mark, 3, Convex, CoordModeOrigin );
}

MenuLabel *DrawMenuItem( MenuLabel *ml, int sel, Bool on )
{
	GC r_tmpGC, s_tmpGC;
	int label_p, top_y, lp, width, offset;
	Pixmap icon;
	MenuItem *mi;
	unsigned long gcm;
	XGCValues gcv;
	char dots[] = { 2, 2 };

	for( lp=0, mi = ml->m_item; lp<sel-1; lp++, mi=mi->next );
	if( !(mi->mode&SELECTON) && on )		return NULL;

	top_y = ml->ItemHeight*(sel-1);
	label_p = ml->ItemHeight==0?20:20+ml->ItemHeight+5;

	if( on ){
		r_tmpGC = Scr.flags&SYSTEM8?Scr.MenuSelectBlueGC:Scr.BlackGC;
		s_tmpGC = Scr.WhiteGC;
	}
	else{
		r_tmpGC = Scr.flags&SYSTEM8?Scr.MenuBlueGC:Scr.WhiteGC;
		s_tmpGC = Scr.BlackGC;
	}

	if( mi->mode&STRGRAY && Scr.d_depth>2 )
		s_tmpGC = Scr.Gray3GC;
	XFillRectangle( dpy, ml->PullWin, r_tmpGC,
				   0+(Scr.flags&SYSTEM8?1:0), top_y,
				   ml->MenuWidth-(Scr.flags&SYSTEM8?2:0), ml->ItemHeight );

/** Draw Check Mark. **/
	if( mi->mode & CHECKON )
		DrawCheckMark( ml, s_tmpGC, top_y );
/** Draw Submenu Mark **/
	if( mi->submenu )
		DrawSubMenuMark( ml, s_tmpGC, top_y );
/** Draw Icon. **/
	if( mi->xpm ){
		if( mi->mode&STRGRAY || mi->mode&ICONGRAY)
			icon = mi->xpm->lighticon;
		else
			icon = mi->xpm->icon;
		if( mi->xpm->kind==PIXMAP ){
			XSetClipMask( dpy, s_tmpGC, mi->xpm->mask );
			XSetClipOrigin( dpy, s_tmpGC, 
						   20+(ml->IconWidth-mi->xpm->width)/2,
						   top_y+(ml->ItemHeight-mi->xpm->height)/2);
			XCopyArea( dpy, icon, ml->PullWin, s_tmpGC, 0, 0,
					  mi->xpm->width, mi->xpm->height,
					  20+(ml->IconWidth-mi->xpm->width)/2,
					  top_y+(ml->ItemHeight-mi->xpm->height)/2);
			XSetClipMask( dpy, s_tmpGC, None );
		}
		else{
			XCopyArea( dpy, on?mi->xpm->mask:mi->xpm->icon,
					  ml->PullWin, s_tmpGC, 0, 0,
					  mi->xpm->width, mi->xpm->height,
					  20+(ml->IconWidth-mi->xpm->width)/2,
					  top_y+(ml->ItemHeight-mi->xpm->height)/2);
			if( mi->mode&ICONGRAY  ){
				if( on )		gcv.function = GXand;
				else			gcv.function = GXor;
				gcm = GCFunction;
				XChangeGC( dpy, Scr.BlackGC, gcm, &gcv );
				XSetFillStyle( dpy, Scr.BlackGC, FillTiled );
				XFillRectangle( dpy, ml->PullWin, Scr.BlackGC, 21, top_y,
							   mi->xpm->width, ml->ItemHeight );
				XSetFillStyle( dpy, Scr.BlackGC, FillSolid );
				gcv.function = GXcopy;
				XChangeGC( dpy, Scr.BlackGC, gcm, &gcv );
			}
		}
	}
/** Draw Label String. **/
	if( mi->label ){
		StrWidthHeight( MENUFONT, &width, NULL, &offset,
					   mi->label, strlen(mi->label));
		XDRAWSTRING( dpy, ml->PullWin, MENUFONT, s_tmpGC,
					(mi->xpm!=NULL?label_p:20),
					top_y+ml->ItemHeight/2-offset,
					mi->label, strlen(mi->label) );
/* Mask String for Mono Display */
		if( mi->mode&STRGRAY && Scr.d_depth<2 ){
			if( on )		gcv.function = GXand;
			else			gcv.function = GXor;
			gcm = GCFunction;
			XChangeGC( dpy, Scr.BlackGC, gcm, &gcv );
			XSetFillStyle( dpy, Scr.BlackGC, FillTiled );
			XFillRectangle( dpy, ml->PullWin, Scr.BlackGC, 0, top_y,
						   ml->MenuWidth, ml->ItemHeight );
			XSetFillStyle( dpy, Scr.BlackGC, FillSolid );
			gcv.function = GXcopy;
			XChangeGC( dpy, Scr.BlackGC, gcm, &gcv );
		}
	}
	if( mi->label && mi->label[0]=='\0' ){
		if( Scr.d_depth>1 ){
			if( Scr.flags&SYSTEM8 ){
				XDrawLine( dpy, ml->PullWin, Scr.Gray2GC,
						  2, top_y+ml->ItemHeight/2,
						  ml->MenuWidth-3, top_y+ml->ItemHeight/2);
				XDrawLine( dpy, ml->PullWin, Scr.WhiteGC,
						  3, top_y+ml->ItemHeight/2+1,
						  ml->MenuWidth-2, top_y+ml->ItemHeight/2+1);
			}
			else
				XDrawLine( dpy, ml->PullWin, Scr.Gray2GC,
						  0, top_y+ml->ItemHeight/2,
						  ml->MenuWidth, top_y+ml->ItemHeight/2);
		}
		else{
			gcm = GCLineStyle;
			gcv.line_style = LineOnOffDash;
			XChangeGC( dpy, Scr.BlackGC, gcm, &gcv );
			XSetDashes( dpy, Scr.BlackGC, 0, dots, 2 );
			XDrawLine( dpy, ml->PullWin, Scr.BlackGC,
					  0, top_y+ml->ItemHeight/2,
					  ml->MenuWidth, top_y+ml->ItemHeight/2);
			gcm = GCLineStyle;
			gcv.line_style = FillSolid;
			XChangeGC( dpy, Scr.BlackGC, gcm, &gcv );
		}
	}
	
	return mi->submenu;
}

void SortMenuItem( MenuLabel *m, MenuItem *start )
{
	MenuItem *fence, *lastSwap, *i, *j, *k;

	for(fence = NULL; fence != start; fence = lastSwap )
		for(j = (lastSwap = i = start)->next;
			(k = j->next) != fence;
			i = j, j = k ){
			if ( strcmp( j->label, k->label ) > 0 ){
				i->next = k;
				j->next = k->next;
				lastSwap = k->next = j;
			}
		}
}

/*****************************************************************
MenuLabel *m  Menu Label of Target
int left      left side position of menu
int right     right side position of menu
int y         Height of menu
Bool side     True  left(Tear off right)
              False right(Tear off left)
*****************************************************************/
Bool MapMenu( MenuLabel *m, int left, int right, int y, Bool side )
{
	int lp, items=0;
	int label_w, local_mw=0, height;
	MenuItem *mi;

	if( m->LabelWin!=None )	RedrawMenu( m, True );

	if( m == &Scr.IconMenu && Scr.iconAnchor->next )
		SortMenuItem( &Scr.IconMenu, Scr.iconAnchor );

	m->MenuWidth = 0;
	m->MenuHeight = 0;
	m->ItemHeight = 0;
	m->IconWidth = 0;
	m->SelectNum = -1;

	for( mi = m->m_item; mi; mi=mi->next ){
		StrWidthHeight( MENUFONT, &label_w, &height, NULL,
					   mi->label, strlen(mi->label));
		m->ItemHeight = max( height, m->ItemHeight );
		if( mi->iconname && !(mi->xpm) )
			mi->xpm = ReadIcon( mi->iconname, NULL, False );
		if( mi->xpm ){
			m->ItemHeight = max( mi->xpm->height, m->ItemHeight );
			m->IconWidth = max( mi->xpm->width, m->IconWidth );
			local_mw = 20+mi->xpm->width+5+label_w+10;
		}
		else
			local_mw = 20+label_w+10;
		if( mi->submenu )			local_mw += 15;
		m->MenuWidth = max( local_mw, m->MenuWidth );
		if( (mi->label && mi->label[0]) || mi->next )
			items++;
	}

	m->ItemHeight+=2;
	m->MenuHeight = m->ItemHeight*items;

	if( side )
		m->MenuX = left;
	else
		m->MenuX = right-m->MenuWidth;

	if( m->MenuX+m->MenuWidth > Scr.MyDisplayWidth ){
		m->MenuX = right-m->MenuWidth;
		side = False;
	}
	if( m->MenuX < 0 ){
		m->MenuX = 0;
		side = True;
	}
	m->MenuY = y+m->MenuHeight<Scr.MyDisplayHeight ?
		y : Scr.MyDisplayHeight-m->MenuHeight;
	m->MenuY = m->MenuY<MENUB_H-1 ? MENUB_H-1 : m->MenuY;
	if( Scr.flags&SYSTEM8 ){
		m->MenuWidth+=2;
		m->MenuHeight+=1;
	}
	XMoveResizeWindow( dpy, m->PullWin, m->MenuX, m->MenuY,
					  m->MenuWidth+2, m->MenuHeight+2 );
	{
		XRectangle rect[7];
		int point=0;

		rect[point].x = -1;		rect[point].y = -1;
		rect[point].width = m->MenuWidth+1;
		rect[point].height = m->MenuHeight+1;
		point++;
		for( lp=1; lp<4; lp++ ){
			rect[point].x = m->MenuWidth+lp-1;	rect[point].y = lp-1;
			rect[point].width = 1;	rect[point].height = m->MenuHeight+2-lp+1;
			point++;
		}
		for( lp=1; lp<4; lp++ ){
			rect[point].x = lp-1;	rect[point].y = m->MenuHeight+lp-1;
			rect[point].width = m->MenuWidth+2-lp+1;	rect[point].height = 1;
			point++;
		}
		XShapeCombineRectangles(dpy,m->PullWin,ShapeBounding,
								0,0,rect,point,ShapeSet,Unsorted);
	}

	XMapRaised( dpy, m->PullWin );

	return side;
}

void UnmapMenu( MenuLabel *m, int mode )
{
	if( mode&UNMAP_WINDOW ){
		XUnmapWindow( dpy, m->PullWin );
		m->SelectNum = -1;
	}
	if( mode&UNMAP_LABEL )
		RedrawMenu( m, False );
	XSync( dpy, 0 );
}

void ExecMenu( MenuLabel *m, int selected )
{
	int lp;
	MenuItem *tmp_item;

	for( lp=0, tmp_item = m->m_item; lp<selected-1;
		lp++,tmp_item=tmp_item->next );
	if( !(tmp_item->mode&SELECTON) || !(tmp_item->action ))
		return;

	for( lp=0; lp<Scr.flush_times; lp++ ){
		DrawMenuItem( m, selected, False );		XFlush( dpy );
		sleep_a_little( Scr.flush_time );
		DrawMenuItem( m, selected, True );		XFlush( dpy );
		sleep_a_little( Scr.flush_time );
	}

	DrawMenuItem( m, selected, True );
	XFlush( dpy );
	UnmapMenu( m, UNMAP_WINDOW );
	ExecuteFunction( tmp_item->action );
	UnmapMenu( m, UNMAP_LABEL );
}

void DrawMenuItemAll( MenuLabel *ml )
{
	int lp;
	MenuItem *tmp_item;

	for( lp=0, tmp_item=ml->m_item; tmp_item; lp++,tmp_item=tmp_item->next )
		DrawMenuItem( ml, lp+1, lp+1==ml->SelectNum?True:False );
	for( lp=0; lp<2; lp++ ){
		XDrawLine( dpy, ml->PullWin, Scr.BlackGC,
				  lp, ml->MenuHeight+lp, ml->MenuWidth+1, ml->MenuHeight+lp );
		XDrawLine( dpy, ml->PullWin, Scr.BlackGC,
				  ml->MenuWidth+lp, lp, ml->MenuWidth+lp, ml->MenuHeight);
	}
	if( Scr.flags&SYSTEM8 ){
		XDrawLine( dpy, ml->PullWin, Scr.WhiteGC,
				  0, 0, 0, ml->MenuHeight );
		XDrawLine( dpy, ml->PullWin, Scr.Gray2GC,
				  0, ml->MenuHeight-1, ml->MenuWidth-1, ml->MenuHeight-1 );
		XDrawLine( dpy, ml->PullWin, Scr.Gray2GC,
				  ml->MenuWidth-1, 0, ml->MenuWidth-1, ml->MenuHeight-1 );
	}
}

Bool ChoiseMenu( MenuLabel *m, Window *entwin, int ignore, Bool side )
{
	Bool isEnd = False, finishall=False, Release=False, ChildSide=True;
	XEvent Event;
	MenuLabel *mapped=NULL, *tmp_menu;

	while( !isEnd && !finishall){
		XMaskEvent( dpy, ExposureMask | ButtonReleaseMask | ButtonPressMask|
						EnterWindowMask | PointerMotionMask | ButtonMotionMask,
						&Event );
//		XNextEvent( dpy, &Event );
		switch( Event.type ){
		case Expose:
			if( XFindContext( dpy, Event.xany.window, MenuContext,
							 (caddr_t *)&tmp_menu )!=XCNOENT ){
				if( Event.xexpose.count==0 )	DrawMenuItemAll( tmp_menu );
			}
			else	handle_expose( &Event );
			break;
		case EnterNotify:
			if( XFindContext( dpy, Event.xcrossing.window, MenuContext,
							 (caddr_t *)&tmp_menu )!=XCNOENT ){
				if( tmp_menu==mapped ){
					finishall = ChoiseMenu( mapped, entwin, ignore, ChildSide );
					if( *entwin!=m->PullWin )	isEnd = True;
				}
				else if( tmp_menu!=m || Event.xcrossing.window==m->LabelWin ){
					isEnd = True;
					*entwin = Event.xcrossing.window;
					if( m->SelectNum != -1 ){
						DrawMenuItem( m, m->SelectNum, False );
						m->SelectNum = -1;
						if( mapped ){
							UnmapMenu( mapped, UNMAP_ALL );
							mapped = NULL;
						}
					}
				}
			}
			if( Event.xcrossing.window == Scr.MenuBar &&
				Event.xcrossing.detail!=NotifyNonlinearVirtual ){
				isEnd = True;
				*entwin = Scr.MenuBar;
			}
			break;
		case ButtonRelease:
			if( ignore ){
				isEnd = True;
				finishall = True;
				Release = True;
			}
			else	ignore++;
			break;
		case MotionNotify:
			if(isRect( Event.xbutton.x, Event.xbutton.y, 0, 0,
						m->MenuWidth, m->MenuHeight ) &&
					Event.xany.window == m->PullWin ){
				if( m->SelectNum != (Event.xbutton.y-3)/m->ItemHeight+1 ){
					if( m->SelectNum != -1 ){
						DrawMenuItem( m, m->SelectNum, False);
						if( mapped ){
							UnmapMenu( mapped, UNMAP_ALL );
							mapped = NULL;
						}
					}
					m->SelectNum = (Event.xbutton.y-3)/m->ItemHeight+1;
					mapped=DrawMenuItem( m, m->SelectNum, True );
					if( mapped ){
						ChildSide = MapMenu( mapped,
								m->MenuX+m->MenuWidth-5, m->MenuX+5,
								(m->MenuY==MENUB_H-1 && m->SelectNum==1) ?
								m->MenuY+m->ItemHeight/2 :
								m->MenuY+m->ItemHeight*(m->SelectNum-1),
								side );
					}
				}
			}
			else if( m->SelectNum != -1 ){
				DrawMenuItem( m, m->SelectNum, False );
				m->SelectNum = -1;
				if( mapped ){
					UnmapMenu( mapped, UNMAP_ALL );
					mapped = NULL;
				}
			}
			break;
		default:
			break;
		}
	}

	if( mapped ){
		UnmapMenu( mapped, UNMAP_ALL );
		DrawMenuItemAll( m );
	}
	if( Release && m->SelectNum!=-1 ){
		UnGrabEvent();
		ExecMenu( m, m->SelectNum );
		if( !GrabEvent( DEFAULT ) ){
			XBell( dpy, 30 );
			return finishall;
		}
	}

	return finishall;
}

void press_menu( MenuLabel *m )
{
	Bool isEnd = False, Side=True;
	XEvent Event;
	MenuLabel *mapped, *tmp_menu;
	int x, y, JunkX, JunkY;
	unsigned int JunkMask;
	Window JunkRoot, JunkChild, entwin;
	int ignore;

	if( m!=NULL )
		Side=MapMenu( m, m->LabelX, m->LabelX+m->LabelWidth, MENUB_H-1, True );

    if( !GrabEvent( DEFAULT ) ){
        XBell( dpy, 30 );
        return;
    }

	if( Scr.flags&ONECLICKMENU )	ignore=0;
	else							ignore=1;

	mapped = m;
	while( !isEnd ){
		XMaskEvent( dpy, ExposureMask | ButtonReleaseMask | ButtonPressMask|
						EnterWindowMask | PointerMotionMask | ButtonMotionMask,
						&Event );
//		XNextEvent( dpy, &Event );
		switch( Event.type ){
		case Expose:
			if( mapped && Event.xany.window==mapped->PullWin &&
			   Event.xexpose.count==0 )
				DrawMenuItemAll( mapped );
			else
				handle_expose( &Event );
			break;
		case ButtonRelease:
			if( ignore )	isEnd = True;
			else			ignore++;
			break;
		case EnterNotify:
			if( XFindContext( dpy, Event.xcrossing.window,
							 MenuContext, (caddr_t *)&tmp_menu )
			   !=XCNOENT ){
				if( mapped != tmp_menu ){
					if( mapped )	UnmapMenu( mapped, UNMAP_ALL );
					mapped = tmp_menu;
					Side = MapMenu( mapped, mapped->LabelX,
							mapped->LabelX+mapped->LabelWidth,
							MENUB_H-1, True );
				}
				else if( Event.xcrossing.window==mapped->PullWin ){
					isEnd = ChoiseMenu( mapped, &entwin, ignore, Side );
					if( entwin == Scr.MenuBar ){
						UnmapMenu( mapped, UNMAP_ALL );
						mapped = NULL;
					}
				}
			}
			if( Event.xcrossing.window==Scr.MenuBar &&
			   Event.xcrossing.detail!=NotifyVirtual ){
				if( mapped!=NULL ){
					XQueryPointer( dpy, mapped->PullWin,
								  &JunkRoot, &JunkChild,
								  &x, &y,&JunkX, &JunkY,&JunkMask);
					if( !isRect( x, y, 0, 0,
								mapped->MenuWidth, mapped->MenuHeight ) && 
					   !isRect( x-mapped->LabelX, y, 0, 0,
							   mapped->LabelWidth, MENUB_H )){
						UnmapMenu( mapped, UNMAP_ALL );
						mapped = NULL;
					}
				}
			}
			break;
		default:
			break;
		}
	}
	if( mapped )	UnmapMenu( mapped, UNMAP_ALL );
    UnGrabEvent();
}

Bool SwallowMenu( MenuLabel *ml )
{
	XEvent ev;
	XClassHint class;
	XSizeHints hints;
	MlvwmWindow *tmp_win;
	Bool done=True;
	char *name, *comm, *fullpath;
	long supplied=0;
	Atom *protocols = NULL, *ap;
	int n, lp;
	XWindowAttributes attr;
	XTextProperty text_prop;
#ifdef USE_LOCALE
	int num;
	char **list;
#endif

	stripquote( ml->action, &comm );
	if( comm==NULL ){
		ml->flags &= ~SWALLOW;

		return False;
	}
	if( (fullpath=LookUpFiles( getenv("PATH"), comm, X_OK ))==NULL ){
		ml->flags &= ~SWALLOW;
		DrawErrMsgOnMenu( "Swallow Falied(Can't Exec) ", comm );
		return False;
	}
	free( fullpath );
	free( comm );
	ExecuteFunction( ml->action );

	do{
		XMaskEvent( dpy, SubstructureRedirectMask |
				   StructureNotifyMask | SubstructureNotifyMask, &ev );
		if( ev.type==MapRequest ){
			if( XGetWMName( dpy, ev.xmaprequest.window, &text_prop) != 0 ){
#ifdef USE_LOCALE
				if(text_prop.value)
					text_prop.nitems = strlen(text_prop.value);
				if(XmbTextPropertyToTextList(dpy, &text_prop, &list, &num)
				   >= Success
				   && num > 0 && *list)
					name = *list;
				else
					name = NoName;
#else
				name = (char *)text_prop.value;
#endif
			}
			else
				name = NoName;

			XGetClassHint(dpy, ev.xmaprequest.window, &class);	
			if( !strcmp( name, ml->name ) ||
			   !strcmp( class.res_name, ml->name ) ||
			   !strcmp( class.res_class, ml->name ) ){
				done = False;
				XGetWindowAttributes( dpy, ev.xmaprequest.window, &attr );

				if(!XGetWMNormalHints( dpy, ev.xmaprequest.window, &hints,
									  &supplied ))
					hints.flags = 0;
				if((USSize | PSize ) & hints.flags ){
					ml->LabelWidth = hints.width;
					ml->LabelHeight = hints.height;
				}
				else{
					ml->LabelWidth = attr.width;
					ml->LabelHeight = attr.height;
				}
				ml->LabelWin = ev.xmaprequest.window;
				ml->flags &= ~CANDELETE;
				if(XGetWMProtocols (dpy, ml->LabelWin, &protocols, &n)){
					for( lp=0, ap=protocols; lp<n; lp++, ap++)
						if(*ap == (Atom)_XA_WM_DELETE_WINDOW)
							ml->flags |= CANDELETE;
				}
				if (protocols) XFree ((char *) protocols);
			}
			else
				HandleEvents( ev );
			if( name!=NoName )		XFree( name );
		}
		else
			HandleEvents( ev );
		if( ev.type==MapNotify &&
		   XFindContext(dpy, ev.xany.window, MlvwmContext, (caddr_t *)&tmp_win)
		   !=XCNOENT)
			DrawStringMenuBar( tmp_win->name );
	}
	while( done );
	return True;
}

void CreateMenuLabel( MenuLabel *ml )
{
	unsigned long valuemask;
	XSetWindowAttributes attributes;
	int width;

	if( ml->flags&SWALLOW ){
		if( SwallowMenu( ml ) ){
			XSetWindowBorderWidth( dpy, ml->LabelWin, 0 );
//			XResizeWindow( dpy, ml->LabelWin, ml->LabelWidth, MENUB_H-2 );
			XReparentWindow( dpy, ml->LabelWin, Scr.MenuBar, 0, 0 );
			ml->LabelWidth += 6;
		}
	}
	else{
		if( ml->xpm ){
			ml->LabelWidth = ml->xpm->width+16;
			ml->LabelHeight = ml->xpm->height;
		}
		else{
			ml->LabelHeight = 0;
			if( ml->LabelStr ){
				StrWidthHeight( MENUBARFONT, &width, NULL, NULL,
					   ml->LabelStr, strlen(ml->LabelStr));
				ml->LabelWidth = width+16;
			}
			else
				ml->LabelWidth = 16;
		}
		valuemask = CWCursor | CWEventMask | CWBackPixel;
		attributes.background_pixel = WhitePixel( dpy, Scr.screen );
		attributes.cursor = Scr.MlvwmCursors[DEFAULT];
		attributes.event_mask = (ButtonPressMask | ButtonReleaseMask |
								 EnterWindowMask | ExposureMask |
								 OwnerGrabButtonMask | PointerMotionMask);
		ml->LabelWin = XCreateWindow( dpy, Scr.MenuBar, 0, 1,
					  ml->LabelWidth, MENUB_H-2, 0,
					  CopyFromParent, InputOutput,
					  CopyFromParent,
					  valuemask, &attributes );
		XSaveContext( dpy, ml->LabelWin, MenuContext, (caddr_t)ml );

		valuemask = CWCursor | CWEventMask | CWBackPixel;
		attributes.background_pixel = WhitePixel( dpy, Scr.screen );
		attributes.cursor = Scr.MlvwmCursors[DEFAULT];
		attributes.event_mask = (SubstructureRedirectMask | ExposureMask | 
								 SubstructureNotifyMask | PointerMotionMask |
								 EnterWindowMask | LeaveWindowMask );
		ml->PullWin = XCreateWindow( dpy, Scr.Root, 0, 0, 10, 10 , 1,
									CopyFromParent, InputOutput,
									CopyFromParent,
									valuemask, &attributes );
		XSaveContext( dpy, ml->PullWin, MenuContext, (caddr_t)ml );
	}
}

void AddMenuItem( MenuLabel *ml, char *label, char *action, char *icon, Icon *miniicon, MenuLabel *submenu, int mode )
{
	MenuItem *tmpitem, *newitem;

	newitem = calloc( 1, sizeof( MenuItem ) );

	if( ml->m_item == NULL )
		ml->m_item = newitem;
	else{
		for( tmpitem=ml->m_item; tmpitem->next; tmpitem=tmpitem->next );
		tmpitem->next = newitem;
	}
	newitem->label = strdup( label );
	newitem->action = action ? strdup( action ) : action ;
	newitem->iconname = icon ? strdup( icon ) : icon ;
	newitem->mode = mode;
	newitem->submenu = submenu;
	newitem->xpm = miniicon;
}

void DelMenuItem( MenuLabel *ml, char *action )
{
	MenuItem *mi, *prev;

	for( mi=ml->m_item, prev=NULL;
		strcmp( mi->action, action )!=0 && mi;
		prev=mi, mi=mi->next );
	if( !mi )	return;

	free( mi->label );
	if( mi->action )	free( mi->action );
	if( mi->iconname )	free( mi->iconname );
	if( prev )		prev->next = mi->next;
	free( mi );
}

void CreateMenuItems( void )
{
	MenuLabel *tmpmenu;

	Scr.IconMenu.flags = ACTIVE | STICKLABEL;
	Scr.IconMenu.xpm = Scr.SystemIcon;
	Scr.IconMenu.name = strdup( "ICON" );
	Scr.IconMenu.LabelStr = NULL;
	CreateMenuLabel( &Scr.IconMenu );

	for(tmpmenu=Scr.MenuLabelRoot;tmpmenu!=NULL;tmpmenu=tmpmenu->next)
		CreateMenuLabel( tmpmenu );
}

char *SetItemGrayString( char *str, char **action, int *mode, char **icon, MenuLabel **submenu  )
{
	*mode |= STRGRAY;

	return str+4;
}

char *SetItemBlackString( char *str, char **action, int *mode, char **icon, MenuLabel **submenu  )
{
	*mode &= ~STRGRAY;

	return str+5;
}

char *SetItemCheckMark( char *str, char **action, int *mode, char **icon, MenuLabel **submenu  )
{
	*mode |= CHECKON;

	return str+5;
}

char *SetItemNonCheckMark( char *str, char **action, int *mode, char **icon, MenuLabel **submenu  )
{
	*mode &= ~CHECKON;

	return str+8;
}

char *SetItemSelect( char *str, char **action, int *mode, char **icon, MenuLabel **submenu  )
{
	*mode |= SELECTON;

	return str+6;
}

char *SetItemNonSelect( char *str, char **action, int *mode, char **icon, MenuLabel **submenu  )
{
	*mode &= ~SELECTON;

	return str+9;
}

char *SetItemIcon( char *str, char **action, int *mode, char **icon, MenuLabel **submenu  )
{
	char *name, *stop, tmp[5], *path;

	if( *icon )
		free( *icon );

	if( (stop = strchr( str, ',' ))!=NULL )
		*stop = '\0';
	name = calloc( strlen(str)-4, 1 );
	sscanf( str, "%s%s", tmp, name );

	*icon = name;
	if((path = LookUpFiles( Scr.IconPath, name, R_OK ))==NULL )
		DrawErrMsgOnMenu( "Can't Find file ", name );
	else
		free( path );

	if( stop!=NULL )
		stop++;
	else
		stop = str+strlen(str);

	return stop;
}

char *SetItemAction( char *str, char **action, int *mode, char **icon, MenuLabel **submenu  )
{
	char *stop, *top;

	if( *action )		free( *action );
	top = SkipSpace( str+6 );
	if( (stop = strchr( str, ',' ))!=NULL )
		*stop = '\0';
	else
		str[strlen(str)-1]='\0';
	*action = strdup( top );

	if( stop!=NULL )
		stop++;
	else
		stop = str+strlen(str);

	return stop;
}

char *SetSubMenu( char *str, char **action, int *mode, char **icon, MenuLabel **submenu )
{
	char *stop, *top;
	MenuLabel *tmp_l;

	top = SkipSpace( str+7 );
	if( (stop = strchr( str, ',' ))!=NULL )
		*stop = '\0';
	else
		str[strlen(str)-1]='\0';

	for( tmp_l=Scr.MenuLabelRoot;
		tmp_l && strcmp( top, tmp_l->name ); tmp_l=tmp_l->next );

	if( tmp_l )
		*submenu = tmp_l;
	else
		DrawErrMsgOnMenu( "Configuration Error(SubMenu)! ", str );

	if( stop!=NULL )
		stop++;
	else
		stop = str+strlen(str);

	return stop;
}

menu_item_func MenuItemFunc[]={
	{"Gray", SetItemGrayString },
	{"Black", SetItemBlackString },
	{"Check", SetItemCheckMark },
	{"NonCheck", SetItemNonCheckMark },
	{"Select", SetItemSelect },
	{"NonSelect", SetItemNonSelect },
	{"SubMenu", SetSubMenu },
	{"Icon", SetItemIcon },
	{"Action", SetItemAction },
	{NULL, 0}
};


char *SetLabelIconName( MenuLabel *ml, char *str )
{
	char *name, *stop, tmp[5];

	if( (stop = strchr( str, ',' ))!=NULL )
		*stop = '\0';
	name = calloc( strlen(str)-4, 1 );
	sscanf( str, "%s%s", tmp, name );
	ml->xpm = ReadIcon( name, NULL, True );
	free( name );

	if( stop!=NULL )
		stop++;
	else
		stop = str+strlen(str);

	return stop;
}

char *SetLabelStick( MenuLabel *ml, char *str )
{
	ml->flags |= STICKLABEL;

	return str+5;
}

char *SetLabelNonStick( MenuLabel *ml, char *str )
{
	ml->flags &= ~STICKLABEL;

	return str+8;
}

char *SetLabelRight( MenuLabel *ml, char *str )
{
	ml->flags &= ~LEFTSIDE;

	return str+5;
}

char *SetLabelLeft( MenuLabel *ml, char *str )
{
	ml->flags |= LEFTSIDE;

	return str+5;
}

char *SetMenuLabelLabel( MenuLabel *ml, char *str )
{
	char *top;

	top = stripquote( str+6, &(ml->LabelStr) );

	return top;
}

char *SetSwallowAction( MenuLabel *ml, char *str )
{
	char *stop, *top;

	top = SkipSpace( str+6 );
	if( (stop = strchr( str, ',' ))!=NULL )
		*stop = '\0';
	else
		str[strlen(str)-1]='\0';
	ml->action = strdup( top );

	if( stop!=NULL )
		stop++;
	else
		stop = str+strlen(str);

	return stop;
}

menu_func MenuLabelFunc[]={
	{"Action", SetSwallowAction},
	{"Icon", SetLabelIconName},
	{"Label", SetMenuLabelLabel },
	{"Left", SetLabelLeft},
	{"Right", SetLabelRight},
	{"Stick", SetLabelStick},
	{"NonStick", SetLabelNonStick},
	{NULL, 0}
};

void SetSwallow( char *line, FILE *fp )
{
	char str[256], *top;
	MenuLabel *tmpmenu, *last_m;
	int lp;

	tmpmenu = calloc( 1, sizeof( MenuLabel ) );

	if( Scr.MenuLabelRoot==NULL )		Scr.MenuLabelRoot = tmpmenu;
	else{
		for( last_m=Scr.MenuLabelRoot;
			last_m->next!=NULL;
			last_m=(MenuLabel *)last_m->next );
		last_m->next = tmpmenu;
	}
	top = stripquote( line, &(tmpmenu->name) );
	tmpmenu->flags = SWALLOW | ACTIVE | STICKLABEL;

	while( *top!='\n' && *top!='\0' ){
		for( ; !isalpha( *top ) && *top!='\n' && *top!='\0'; top++ );
		if( *top=='\n' || *top=='\0' )		break;
		for( lp=0; MenuLabelFunc[lp].label!=NULL; lp++ ){
			if( !strncmp( top, MenuLabelFunc[lp].label,
						 strlen(MenuLabelFunc[lp].label) )){
				top=MenuLabelFunc[lp].action( tmpmenu, top );
				break;
			}
		}
		if( !MenuLabelFunc[lp].label ){
			DrawErrMsgOnMenu( "Configuration Error(Swallow)! ", str );
			for( ; *top!=',' && *top!='\n' && *top!='\0'; top++ );
		}
	}
}

void SetMenu( char *line, FILE *fp )
{
	char str[256], *top, tmp[5];
	MenuLabel *tmpmenu, *last_m, *submenu;
	int mode;
	char *action, *label;
	char *icon;
	int lp;

	tmpmenu = calloc( 1, sizeof( MenuLabel ) );

	if( Scr.MenuLabelRoot==NULL )		Scr.MenuLabelRoot = tmpmenu;
	else{
		for( last_m=Scr.MenuLabelRoot;
			last_m->next!=NULL;
			last_m=(MenuLabel *)last_m->next );
		last_m->next = tmpmenu;
	}

	if( (top = strchr( line, ',' ))!=NULL )
		*top = '\0';
	tmpmenu->name = calloc( strlen(line)-4, 1 );
	sscanf( line, "%s%s", tmp, tmpmenu->name );
	if( top )		top++;
	else		top = line+strlen( line );

	tmpmenu->flags = LEFTSIDE;

	while( *top!='\n' && *top!='\0' ){
		for( ; !isalpha( *top ) && *top!='\n' && *top!='\0'; top++ );
		if( *top=='\n' || *top=='\0' )		break;
		for( lp=0; MenuLabelFunc[lp].label!=NULL; lp++ ){
			if( !strncmp( top, MenuLabelFunc[lp].label,
					 strlen(MenuLabelFunc[lp].label) )){
				top=MenuLabelFunc[lp].action( tmpmenu, top );
				break;
			}
		}
		if( !MenuLabelFunc[lp].label ){
			DrawErrMsgOnMenu( "Configuration Error(Menu)! ", str );
			for( ; *top!=',' && *top!='\n' && *top!='\0'; top++ );
		}
	}

	while( fgetline( str, 256, fp )!=NULL && strncmp( str, "END", 3) ){
		if( str[0]=='#' )		continue;
		mode = SELECTON;
		label = NULL;
		action = NULL;
		icon = NULL;
		submenu = NULL;

		top = stripquote( str, &label );
		if( !label ){
			DrawErrMsgOnMenu( "Configuration Error! ", str );
			continue;
		}
		while( *top!='\n' && *top!='\0' ){
			for( ; !isalpha( *top ) && *top!='\n' && *top!='\0'; top++ );
			if( *top=='\n' || *top=='\0' )		break;
			for( lp=0; MenuItemFunc[lp].label!=NULL; lp++ ){
				if( !strncmp( top, MenuItemFunc[lp].label,
							 strlen(MenuItemFunc[lp].label) )){
					top = MenuItemFunc[lp].action( top, &action,
												  &mode, &icon, &submenu );
					break;
				}
			}
			if( !MenuItemFunc[lp].label ){
				DrawErrMsgOnMenu( "Configuration Error(MenuItem)! ", str );
				for( ; *top!=',' && *top!='\n' && *top!='\0'; top++ );
			}
		}
		AddMenuItem( tmpmenu, label, action, icon, NULL, submenu, mode );
		if( label )		free( label );
		if( action )	free( action );
		if( icon )		free( icon );
		if( mode&SELECTON )	tmpmenu->flags |= ACTIVE;
	}
}

void SetMenuBar( char *line, FILE *fp )
{
	char str[256], *top, tmp[8];
	Menu *tmp_m, *last_m;
	MenuLink **tmp_link;
	MenuLabel *tmp_l;

	tmp_m = calloc( 1, sizeof( Menu ) );

	if( Scr.MenuRoot==NULL )		Scr.MenuRoot = tmp_m;
	else{
		for( last_m=Scr.MenuRoot;
			last_m->next!=NULL;
			last_m=last_m->next );
		last_m->next = tmp_m;
	}
	if( (top = strchr( line, ',' ))!=NULL )
		*top = '\0';
	tmp_m->name = calloc( strlen(line)-7, 1 );
	sscanf( line, "%s%s", tmp, tmp_m->name );

	if( top )		top++;
	else		top = line+strlen( line );
	tmp_link = &tmp_m->link;

	for( tmp_l=Scr.MenuLabelRoot; tmp_l; tmp_l=tmp_l->next ){
		if( tmp_l->flags&STICKLABEL ){
			*tmp_link = calloc( 1, sizeof( MenuLink ) );
			(*tmp_link)->link = tmp_l;
			tmp_link = &(*tmp_link)->next;
		}
	}
	while( fgetline( str, 256, fp )!=NULL && strncmp( str, "END", 3) ){
		if( str[0]=='#' )		continue;
		str[ strlen(str)-1] = '\0';
		for( tmp_l=Scr.MenuLabelRoot;
			tmp_l && strcmp( str, tmp_l->name ); tmp_l=tmp_l->next );
		if( tmp_l ){
			*tmp_link = calloc( 1, sizeof( MenuLink ) );
			(*tmp_link)->link = tmp_l;
			tmp_link = &(*tmp_link)->next;
		}
		else
			DrawErrMsgOnMenu( "MenuBar Falied(Can't find) ", str );
	}
}

Window PixmapWin (char **data_xpm, Window root, int x, int y)
{
	static XpmAttributes attributes;
	
	Pixmap pixmap;
	Pixmap shapemask;
	Window w;
	XSetWindowAttributes attr;

	XpmCreatePixmapFromData( dpy, root, data_xpm,
							&pixmap, &shapemask, &attributes );
	x = x<0? x + Scr.MyDisplayWidth  - attributes.width  : x-1;
	y = y<0? y + Scr.MyDisplayHeight - attributes.height : y-1;
	
	attr.background_pixel = BlackPixel( dpy, DefaultScreen(dpy) );
	attr.override_redirect = True;
	w = XCreateWindow( dpy, root, x, y,
					  attributes.width, attributes.height, 1,
					  CopyFromParent, InputOutput, CopyFromParent,
					  CWBackPixel | CWOverrideRedirect, &attr );
	XShapeCombineMask( dpy, w, ShapeBounding, 0, 0, shapemask, ShapeSet );
	
	XFreePixmap( dpy, pixmap );
	XFreePixmap( dpy, shapemask );
	return w;
}

void CreateMenuBar( void )
{
	static char *lbot_xpm[] = {
		"7 7 2 1",
		"  c none s none",
		"# c black",
		"#      ",
		"#      ",
		"#      ",
		"##     ",
		"###    ",
		"####   ",
		"#######"
	};
	static char *rbot_xpm[] = {
		"7 7 2 1",
		"  c none s none",
		"# c black",
		"      #",
		"      #",
		"      #",
		"     ##",
		"    ###",
		"   ####",
		"#######"
	};
	unsigned long valuemask;
	XSetWindowAttributes attributes;

	valuemask = CWCursor | CWBackPixel | CWEventMask;
	attributes.cursor = Scr.MlvwmCursors[DEFAULT];
	attributes.background_pixel = WhitePixel( dpy, Scr.screen );
	attributes.event_mask = (SubstructureRedirectMask | ButtonPressMask |
							 EnterWindowMask | ExposureMask |
							 OwnerGrabButtonMask | ButtonReleaseMask );
	Scr.MenuBar = XCreateWindow( dpy, Scr.Root, 0, 0,
								Scr.MyDisplayWidth, MENUB_H ,0,
								CopyFromParent, InputOutput, CopyFromParent,
								valuemask, &attributes );
	XMapWindow( dpy, Scr.MenuBar );
	Scr.lbCorner = PixmapWin( lbot_xpm, Scr.Root,  0, -1 );
	XMapWindow( dpy, Scr.lbCorner );

	Scr.rbCorner = PixmapWin( rbot_xpm, Scr.Root, -1, -1 );
	XMapWindow( dpy, Scr.rbCorner );
	RedrawMenuBar( );
}

void MapMenuBar( MlvwmWindow *win )
{
	int left, right;
	MenuLabel *tmpml;
	MenuLink *mlink;
	styles *style;
	static Bool Mapped=False;

	if( Scr.flags&STARTING )	return;
	left = 10;
	right = Scr.MyDisplayWidth-Scr.IconMenu.LabelWidth-10;

	XMoveWindow( dpy, Scr.IconMenu.LabelWin, right, 1 );
	Scr.IconMenu.LabelX = right;
	right--;

	if( Scr.MenuRoot==NULL ){
		if( Mapped )	return;

		for( tmpml=Scr.MenuLabelRoot; tmpml; tmpml=tmpml->next ){
			if( tmpml->flags&LEFTSIDE ){
				tmpml->LabelX = left;
				left += tmpml->LabelWidth+1;
			}
			else{
				tmpml->LabelX = right-tmpml->LabelWidth-1;
				right = tmpml->LabelX;
			}
			if( tmpml->LabelHeight )
				XMoveWindow( dpy, tmpml->LabelWin,
					tmpml->LabelX+3, (MENUB_H-tmpml->LabelHeight)/2 );
			else
				XMoveWindow( dpy, tmpml->LabelWin, tmpml->LabelX, 1 );
		}
		XMapSubwindows( dpy, Scr.MenuBar );
		Mapped = True;

		return;
	}
	if((!win || (style = lookupstyles( win->name, &win->class ))==NULL) ||
	   !(style->menubar) )
		if( (style = lookupstyles( "*", NULL ))==NULL )
			return;
	if( !(style->menubar) )		return;
	if( Scr.ActiveMenu==style->menubar )		return;

	XGrabServer (dpy);
	for( mlink=style->menubar->link; mlink; mlink=mlink->next ){
		if( mlink->link->flags&LEFTSIDE ){
			mlink->link->LabelX = left;
			left += mlink->link->LabelWidth+1;
		}
		else{
			mlink->link->LabelX = right-mlink->link->LabelWidth-1;
			right = mlink->link->LabelX;
		}
		if( mlink->link->flags&SWALLOW )
			XMoveWindow( dpy, mlink->link->LabelWin,
				mlink->link->LabelX+3, (MENUB_H-mlink->link->LabelHeight)/2 );
		else
			XMoveWindow( dpy, mlink->link->LabelWin, mlink->link->LabelX, 1 );
	}

	if( Scr.ActiveMenu )
		for( mlink=Scr.ActiveMenu->link; mlink; mlink=mlink->next )
			if( !(mlink->link->flags&STICKLABEL) )
				XUnmapWindow( dpy, mlink->link->LabelWin );

	XMapWindow( dpy, Scr.IconMenu.LabelWin );
	for( mlink=style->menubar->link; mlink; mlink=mlink->next ){
		XMapWindow( dpy, mlink->link->LabelWin );
		RedrawMenu( mlink->link, False );
	}
	Scr.ActiveMenu = style->menubar;
	XSync( dpy, 0 );
	XUngrabServer (dpy);
}

void ChangeMenuLabel( MenuLabel *ml, char *newl, Icon *miniicon )
{	
	if( newl==NULL && ml->xpm==miniicon )	return;
	if( ml->LabelStr && !strcmp(ml->LabelStr, newl )){
		if( ml->LabelStr )
			free( ml->LabelStr );
		ml->LabelStr = newl;
	}
	ml->xpm = miniicon;
	RedrawMenu( ml, False );
}

void ChangeMenuItemLabel( char *name, char *oldl, char *newl, char *action, int mm, int sm )
{
	MenuItem *tmpitem;
	MenuLabel *tmplabel;

	if( oldl==NULL || *oldl=='\0' ) return;
	if( !strcmp( name, "ICON" ) )		tmplabel = &(Scr.IconMenu);
	else{
		for( tmplabel = Scr.MenuLabelRoot;
			tmplabel && strcmp( name, tmplabel->name );
			tmplabel=tmplabel->next );
		if( tmplabel==NULL )	return;
	}
	for( tmpitem = tmplabel->m_item;
		tmpitem && (strcmp( oldl, tmpitem->label ) ||
		(action==NULL?False:strcmp(tmpitem->action, action)));
		tmpitem=tmpitem->next );
	if( tmpitem==NULL )	return;
	switch( sm ){
	  case M_COPY:		tmpitem->mode = mm;		break;
	  case M_AND:		tmpitem->mode &= mm;	break;
	  case M_OR:		tmpitem->mode |= mm;	break;
	}
	if( oldl==newl || !strcmp( oldl, newl ) )	return;
	if( tmpitem->label!=NULL )
		free( tmpitem->label );
	tmpitem->label = strdup( newl );
}

void FreeMenuItems( MenuLabel *ml, Bool FreeIcon )
{
	MenuItem *tmpitem, *next;

	tmpitem = ml->m_item;
	while( tmpitem!=NULL ){
		next = tmpitem->next;
		if( tmpitem->label )		free( tmpitem->label );
		if( tmpitem->action )		free( tmpitem->action );
		if( FreeIcon && tmpitem->xpm ){
			XFreePixmap( dpy, tmpitem->xpm->icon );
			XFreePixmap( dpy, tmpitem->xpm->lighticon );
			if( tmpitem->xpm->mask!=None )
				XFreePixmap( dpy, tmpitem->xpm->mask );
			free( tmpitem->xpm );
			free( tmpitem->iconname );
		}
		free( tmpitem );
		tmpitem = next;
	}
}

void FreeMenuLabels( MenuLabel *ml, Bool all )
{
	Bool cont=True;
	XWindowAttributes winattrs;
	unsigned long eventMask;
	XEvent ev;

	FreeMenuItems( ml, all );
	free( ml->name );
	if( ml->LabelStr )	free( ml->LabelStr );
	if( ml->xpm ){
		XFreePixmap( dpy, ml->xpm->icon );
		XFreePixmap( dpy, ml->xpm->lighticon );
		if( ml->xpm->mask!=None )		XFreePixmap( dpy, ml->xpm->mask );
		free( ml->xpm );
	}
	if( !(ml->flags&SWALLOW) ){
		if( ml->action )
			free( ml->action );
		if( ml->LabelWin!=None )
			XDestroyWindow( dpy, ml->LabelWin );
		if( ml->PullWin!=None )
			XDestroyWindow( dpy, ml->PullWin );
	}
	else{
		if( XGetWindowAttributes(dpy, ml->LabelWin, &winattrs) ){
			eventMask = winattrs.your_event_mask;
			XMoveWindow( dpy, ml->LabelWin, ml->LabelX, -MENUB_H );
			XMapWindow( dpy, ml->LabelWin );
			XSelectInput(dpy, ml->LabelWin, eventMask | StructureNotifyMask);

			if( ml->flags&CANDELETE )
				send_clientmessage( ml->LabelWin, _XA_WM_DELETE_WINDOW,
								   CurrentTime);
			else
				XKillClient( dpy, ml->LabelWin );
			do{
				XMaskEvent( dpy, StructureNotifyMask|SubstructureNotifyMask,
						   &ev );
				if( (ev.type==UnmapNotify || ev.type==DestroyNotify) &&
				   (ev.xany.window==ml->LabelWin ||
					ev.xunmap.window==ml->LabelWin) ){
					cont = False;
					XUnmapWindow( dpy, ev.xany.window );
				}
				else
					HandleEvents( ev );
			}
			while( cont );
		}
		free( ml->action );
	}
	if( all )		free( ml );
}

void FreeMenu( void )
{
	Menu *tmpm, *nextm;
	MenuLink *tmplink, *nextlink;
	MenuLabel *tmpml, *nextl;

	tmpm = Scr.MenuRoot;
	while( tmpm ){
		nextm = tmpm->next;
		tmplink = tmpm->link;
		while( tmplink ){
			nextlink = tmplink->next;
			free( tmplink );
			tmplink = nextlink;
		}
		free( tmpm->name );
		free( tmpm );
		tmpm = nextm;
	}

	tmpml = Scr.MenuLabelRoot;
	while( tmpml ){
		nextl = tmpml->next;
		FreeMenuLabels( tmpml, True );
		tmpml = nextl;
	}
	ChangeMenuLabel( &(Scr.IconMenu), NULL, Scr.SystemIcon );
	FreeMenuLabels( &Scr.IconMenu, False );
	XUnmapWindow( dpy, Scr.MenuBar );
}

void FreeShortCut( void )
{
	ShortCut *now, *next;

	now = Scr.ShortCutRoot;
	for( ;now!=NULL; now = next ){
		next = now->next;
		free( now->action );
		free( now );
	}
}

void KeepOnTop( void )
{
	MlvwmWindow *tmp;

	if( Scr.flags&STARTING )	return;
	for (tmp = Scr.MlvwmRoot.next; tmp != NULL; tmp = tmp->next)
		if( tmp->flags&ONTOP )
			RaiseMlvwmWindow( tmp );
	XRaiseWindow( dpy, Scr.MenuBar );
	XRaiseWindow( dpy, Scr.lbCorner );
	XRaiseWindow( dpy, Scr.rbCorner );
}

void CreateSimpleMenu( void )
{
	Scr.MenuLabelRoot = calloc( 1, sizeof( MenuLabel ) );
	Scr.MenuLabelRoot->flags = LEFTSIDE | ACTIVE;
	Scr.MenuLabelRoot->name = strdup( "Default" );
	Scr.MenuLabelRoot->LabelStr = strdup( "File" );

	Scr.MenuLabelRoot->m_item = calloc( 1, sizeof( MenuItem ) );
	Scr.MenuLabelRoot->m_item->mode = SELECTON;
	Scr.MenuLabelRoot->m_item->label = strdup( "Quit" );
	Scr.MenuLabelRoot->m_item->action = strdup( "Exit" );

	Scr.MenuRoot = calloc( 1, sizeof(Menu) );
	Scr.MenuRoot->link = calloc( 1, sizeof(MenuLink) );
	Scr.MenuRoot->link->link = Scr.MenuLabelRoot;
	if( !Scr.style_list ){
		Scr.style_list = calloc( 1, sizeof( styles ) );
		Scr.style_list->name = strdup("*");
		Scr.style_list->flags = NORMALWIN;
		Scr.style_list->maxmizescale = 90;
		Scr.style_list->menubar = Scr.MenuRoot;
	}
}
