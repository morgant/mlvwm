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
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <X11/Xatom.h>

#include "mlvwm.h"
#include "screen.h"
#include "menus.h"
#include "borders.h"
#include "add_window.h"
#include "functions.h"
#include "misc.h"
#include "balloon.h"

#include <X11/extensions/shape.h>

#define MAX_NAME_LEN 200L               /* truncate to this many */

extern int ShapeEventBase;

extern void handle_expose( XEvent * );

MlvwmWindow *ColormapWindow = &Scr.MlvwmRoot;

void send_clientmessage( Window w, Atom a, Time timestamp)
{
	MlvwmWindow *tmp_win;
	XClientMessageEvent ev;

	if( XFindContext ( dpy, w, MlvwmContext, (caddr_t *)&tmp_win )
	   == XCNOENT )
		tmp_win = NULL;

	ev.type = ClientMessage;
	ev.window = w;
	ev.message_type = _XA_WM_PROTOCOLS;
	ev.format = 32;
	ev.data.l[0] = a;
	ev.data.l[1] = timestamp;
	XSendEvent (dpy, w, False, 0L, (XEvent *) &ev);
}

void InstallWindowColormaps (MlvwmWindow *tmp)
{
	XWindowAttributes attributes;
	static Colormap last_cmap;

	ColormapWindow = tmp;
	/* Save the colormap to be loaded for when force loading of
	 * root colormap(s) ends.
	 */
	Scr.pushed_window = tmp;
	/* Don't load any new colormap if root colormap(s) has been
	 * force loaded.
	 */
	if (Scr.root_pushes)      return;

	XGetWindowAttributes(dpy,tmp->w,&attributes);
  
	if(last_cmap != attributes.colormap){
		last_cmap = attributes.colormap;
		XInstallColormap(dpy,attributes.colormap);    
	}
}

void InstallRootColormap( void )
{
    MlvwmWindow *tmp;
    if (Scr.root_pushes == 0){
		tmp = Scr.pushed_window;
		InstallWindowColormaps(&Scr.MlvwmRoot);
		Scr.pushed_window = tmp;
	}
    Scr.root_pushes++;
    return;
}

void UninstallRootColormap( void )
{
	if (Scr.root_pushes)
		Scr.root_pushes--;
  
	if (!Scr.root_pushes) 
		InstallWindowColormaps(Scr.pushed_window);

	return;
}

Bool GrabEvent( int cursor )
{
	int i=0, val=0;
	unsigned int mask;

	XSync( dpy, 0 );
	if(Scr.PreviousActive == NULL)
		Scr.PreviousActive = Scr.ActiveWin;
	XSetInputFocus( dpy, Scr.NoFocusWin, RevertToParent, CurrentTime );
	Scr.ActiveWin = NULL;
	mask = ButtonPressMask | ButtonReleaseMask
		| ButtonMotionMask | PointerMotionMask
			| PointerMotionHintMask;
	while((i<1000)&&(val=XGrabPointer(dpy, Scr.Root, True, mask,
								  GrabModeAsync, GrabModeAsync, Scr.Root,
								  Scr.MlvwmCursors[cursor], CurrentTime)!=
					 GrabSuccess)){
		i++;
		sleep_a_little(1000);
    }
	XSync( dpy, 0 );
	if(val!=GrabSuccess)
		return False;
	else
		return True;
}

void UnGrabEvent( void )
{
	XSync( dpy, 0 );
	XUngrabPointer( dpy, CurrentTime );

	if(Scr.PreviousActive != NULL) {
		SetFocus( Scr.PreviousActive );
		Scr.PreviousActive = NULL;
    }
	XSync( dpy, 0 );
}

void RestoreWithdrawnLocation( MlvwmWindow *tmp, Bool restart )
{
	XWindowChanges xwc;
	Window root, child;
	int rx, ry;
	unsigned int width, height, bw, depth;
	unsigned int mask;

	if(!tmp)    return;

	if (XGetGeometry (dpy, tmp->w, &root, &xwc.x, &xwc.y, 
					  &width, &height, &bw, &depth)){
		XTranslateCoordinates(dpy,tmp->frame,Scr.Root,xwc.x,xwc.y,
							  &rx,&ry,&child);

		xwc.x = rx + tmp->diff_x;
		xwc.y = ry + tmp->diff_y;
		xwc.border_width = tmp->old_bw;
		mask = (CWX | CWY | CWBorderWidth);

 		if( !restart ){
			if( xwc.y>=Scr.MyDisplayHeight )
				xwc.y %= Scr.MyDisplayHeight;
			if( xwc.y<0 )
				xwc.y = (tmp->frame_y+Scr.MyDisplayHeight*Scr.n_desktop)
					%Scr.MyDisplayHeight;
		}
		XReparentWindow (dpy, tmp->w, Scr.Root, xwc.x, xwc.y );
		XConfigureWindow (dpy, tmp->w, mask, &xwc);
		XSync(dpy,0);
	}
}

int GetContext( MlvwmWindow *t, XEvent *e, Window *w )
{
	int Context;

	if(!t)		return C_ROOT;
	Context = C_NO_CONTEXT;
	*w = e->xany.window;

	if( *w == Scr.NoFocusWin )		return C_ROOT;

	if((e->type==KeyPress)&&(e->xkey.subwindow!=None))
		*w = e->xkey.subwindow;

	if((e->type == ButtonPress)&&(e->xbutton.subwindow != None)&&
	   ((e->xbutton.subwindow == t->w)||(e->xbutton.subwindow == t->Parent)))
		*w = e->xbutton.subwindow;

	if(*w == Scr.Root)		Context = C_ROOT;
	if(t){
		if(*w==t->frame)							Context = C_FRAME;
		if((*w == t->w)||(*w==t->Parent))			Context = C_WINDOW;
		if(t->flags&TITLE && *w == t->title_w)		Context = C_TITLE;
		if(t->flags&CLOSER && *w == t->close_b)		Context = C_CLOSE;
		if(t->flags&MINMAXR && *w == t->minmax_b)	Context = C_MINMAX;
		if(t->flags&RESIZER && *w == t->resize_b)	Context = C_RESIZE;
		if(t->flags&SHADER && *w == t->shade_b )    Context = C_SHADE;
		if( t->flags&SBARV ){
			if(*w == t->scroll_v[0] )		Context = C_SBAR_V;
			if(*w == t->scroll_v[1] )		Context = C_SBAR_UP;
			if(*w == t->scroll_v[2] )		Context = C_SBAR_DOWN;
			if(*w == t->scroll_v[3] )		Context = C_SBAR_V_AN;
		}
		if( t->flags&SBARH ){
			if(*w == t->scroll_h[0] )		Context = C_SBAR_H;
			if(*w == t->scroll_h[1] )		Context = C_SBAR_LEFT;
			if(*w == t->scroll_h[2] )		Context = C_SBAR_RIGHT;
			if(*w == t->scroll_h[3] )		Context = C_SBAR_H_AN;
		}
	}
	return Context;
}

MlvwmWindow *NextActiveWin( MlvwmWindow *t )
{
	int lp;
	Window parent, *children;
	unsigned nchildren;
	MlvwmWindow *NextActive;

	XQueryTree( dpy, Scr.Root, &Scr.Root, &parent, &children, &nchildren );
	for( lp=nchildren-1; lp>-1; lp-- ){
		if( XFindContext( dpy, children[lp], MlvwmContext,
						 (caddr_t *)&NextActive )
		   !=XCNOENT &&
		   NextActive->w!=t->w &&
		   !(NextActive->flags&HIDED) &&
		   !(NextActive->flags&NOWINLIST) &&
		   !(NextActive->flags&SKIPSELECT) &&
		   !(NextActive->flags&NOFOCUS) &&
		   (!t->wmhints || t->wmhints->input!=False) &&
		   NextActive->Desk==Scr.currentdesk)
			break;
	}
	if( lp==-1 )
		NextActive = NULL;
	XFree( children );
	return NextActive;
}

void SetMapStateProp( MlvwmWindow *tmp_win, int state)
{
    unsigned long data[2];              /* "suggested" by ICCCM version 1 */

    data[0] = (unsigned long) state;
    data[1] = (unsigned long) None;

    XChangeProperty (dpy, tmp_win->w, _XA_WM_STATE, _XA_WM_STATE, 32, 
					 PropModeReplace, (unsigned char *) data, 2);
}

int GetMapStateProp( MlvwmWindow *tmp_win )
{
	Atom *protocols = NULL;
	Atom atype;
	int aformat;
	unsigned long bytes_remain,nitems;

	if( XGetWindowProperty( dpy, tmp_win->w, _XA_WM_STATE,
						   0L, 2L, False,
						   _XA_WM_STATE, &atype, &aformat,
						   &nitems, &bytes_remain, 
						   (unsigned char **)&protocols ) == Success &&
	   protocols )
		return
			(int) protocols[0];
	else
		return WithdrawnState;
}

void Destroy( MlvwmWindow *t )
{
	MlvwmWindow *NextActive;
	int lp;

	NextActive = NextActiveWin( t );
	XDestroyWindow( dpy, t->Parent );
	XDestroyWindow( dpy, t->frame );

	XDeleteContext( dpy, t->Parent, MlvwmContext );
	XDeleteContext( dpy, t->frame, MlvwmContext );
	XDeleteContext( dpy, t->w, MlvwmContext );
	if( t->flags&CLOSER )
		XDeleteContext( dpy, t->close_b, MlvwmContext );
	if( t->flags&MINMAXR )
		XDeleteContext( dpy, t->minmax_b, MlvwmContext );
	if( t->flags&SHADER )
		XDeleteContext( dpy, t->shade_b, MlvwmContext );
	if( t->flags&TITLE )
		XDeleteContext( dpy, t->title_w, MlvwmContext );
	if( t->flags&SBARV )
		for( lp=0; lp<4; lp++ )
			XDeleteContext( dpy, t->scroll_v[lp], MlvwmContext );
	if( t->flags&SBARH )
		for( lp=0; lp<4; lp++ )
			XDeleteContext( dpy, t->scroll_h[lp], MlvwmContext );
	if( t->flags&RESIZER )
		XDeleteContext( dpy, t->resize_b, MlvwmContext );
	if( Scr.n_desktop>1 )
		for( lp=0; lp<Scr.n_desktop; lp++ )
			if( Scr.LastActive[lp]==t )
				Scr.LastActive[lp] = NULL;

	t->prev->next = t->next;
	if( t->next != NULL )		t->next->prev = t->prev;

	if( t==Scr.ActiveWin ){
		if( Scr.PreviousActive == Scr.ActiveWin )
			Scr.PreviousActive = NULL;
		if( !(Scr.flags & FOLLOWTOMOUSE) || !(Scr.flags&SLOPPYFOCUS) ){
			if( Scr.PreviousActive )
				SetFocus( Scr.PreviousActive );
			else
				SetFocus( NextActive );
		}
		else
			SetFocus( NULL );
		if( Scr.ActiveWin && 
			(!(Scr.flags & FOLLOWTOMOUSE) || !(Scr.flags&SLOPPYFOCUS)) )
			RaiseMlvwmWindow( Scr.ActiveWin );
	}

	if( t->wmhints )		XFree( t->wmhints );
	if ( t->name != NoName)            XFree ( t->name );
	t->name = NULL;
	if( t->class.res_name && t->class.res_name != NoName )
		XFree((char *)t->class.res_name);
	if( t->class.res_class && t->class.res_class != NoName )
		XFree((char *)t->class.res_class);
	free( t );
	XSync( dpy, 0 );
}

void HandleDestroy( XEvent *ev )
{
	MlvwmWindow *tmp_win;
	Window w;
	char action[24];

/*	w = ev->xany.window;*/
	XFlush(dpy);
	w = ev->xdestroywindow.window;
	if( XFindContext( dpy, w, MlvwmContext, (caddr_t *)&tmp_win )==XCNOENT )
		return;
	if( !(tmp_win->flags & NOWINLIST) && !(tmp_win->flags&TRANSIENT)
	   && tmp_win->flags&MAPPED ){
		sprintf( action, "Select %lX", (unsigned long)tmp_win );
		DelMenuItem( &(Scr.IconMenu), action );
	}
	Destroy( tmp_win );
}

void handle_configure_request( XEvent *ev )
{
	XWindowChanges wc;
	unsigned long xwcm;
	MlvwmWindow *tmp_win;
	XConfigureRequestEvent *xcr = &ev->xconfigurerequest;
	int x, y, width, height;
	int title_height, sbar_v, sbar_h;
	Bool notify=False;

	ev->xany.window = xcr->window;      /* mash parent field */
	if (XFindContext (dpy, xcr->window, MlvwmContext, (caddr_t *) &tmp_win) ==
		XCNOENT)
		tmp_win = NULL;
	if( !tmp_win ){
		wc.x = xcr->x;
		wc.y = xcr->y;
		wc.width = xcr->width;
		wc.height = xcr->height;
		wc.sibling = wc.stack_mode = TopIf;
		xwcm = xcr->value_mask & (CWX | CWY | CWWidth | CWHeight);
		XConfigureWindow( dpy, ev->xany.window, xwcm, &wc );
		return;
	}

	{
		int xws, yws, xbs, ybs;
		unsigned wws, hws, wbs, hbs;
		int boundingShaped, clipShaped;
    
		XShapeQueryExtents (dpy, tmp_win->w,&boundingShaped, &xws, &yws, &wws,
							&hws,&clipShaped, &xbs, &ybs, &wbs, &hbs);
		tmp_win->wShaped = boundingShaped;
	}

	title_height = tmp_win->flags & TITLE ? TITLE_HEIGHT : -1;
	sbar_v = tmp_win->flags & SBARV ? SBAR_WH+2: 1;
	sbar_h = tmp_win->flags & SBARH ? SBAR_WH+2: 1;

	x = tmp_win->frame_x;
	y = tmp_win->frame_y;
	width = tmp_win->frame_w;
	height = tmp_win->frame_h;

	if( tmp_win->flags&(TITLE|SBARV|SBARH|RESIZER) ){
		if (xcr->value_mask & CWX)      x = xcr->x - 1;
		if (xcr->value_mask & CWY)      y = xcr->y - title_height - 2;
		if (xcr->value_mask & CWWidth)		width = xcr->width+1+sbar_v;
		if (xcr->value_mask & CWHeight)
			height = xcr->height+2+sbar_h+title_height;
		if( Scr.flags&SYSTEM8 ){
			if (xcr->value_mask & CWX)			x -= 6;
			if (xcr->value_mask & CWWidth)		width += 12;
			if (xcr->value_mask & CWHeight)		height += 6;
		}
	}
	else{
		if (xcr->value_mask & CWX)      x = xcr->x-6;
		if (xcr->value_mask & CWY)      y = xcr->y-6;
		if (xcr->value_mask & CWWidth)		width = xcr->width+12;
		if (xcr->value_mask & CWHeight)		height = xcr->height+12;
	}

	// prevent windows from being moved above menu bar
	if (y < MENUB_H)
		y = MENUB_H;

	if( tmp_win->frame_x!=x || tmp_win->frame_y!=y )	notify = True;

	if( !(tmp_win->flags&SCROLL) || 
	   width>tmp_win->win_w || !(tmp_win->flags&SBARH) )
		width = tmp_win->win_w;
	if( !(tmp_win->flags&SCROLL) || 
	   height>tmp_win->win_h || !(tmp_win->flags&SBARV) )
		height = tmp_win->win_h;

	SetUpFrame(tmp_win, x, y, width, height, notify );
	
	if( tmp_win==Scr.ActiveWin ){
		Scr.ActiveWin = NULL;
		SetFocus( tmp_win );
	}
	KeepOnTop();
}

void MoveWindow( MlvwmWindow *mw, XEvent *evp )
{
	Bool isEnd = False;
	int pre_x, pre_y, drag_x, drag_y, last_x;
	int x, y, JunkX, JunkY;
	unsigned int JunkMask, emask;
	Window JunkRoot, JunkChild;
	XEvent ev;

	pre_x = evp->xbutton.x_root;
	pre_y = evp->xbutton.y_root;
	last_x = -1;
	drag_x = 0;
	drag_y = 0;

	XSync( dpy, 0 );
	while(XCheckMaskEvent(dpy, PointerMotionMask | ButtonMotionMask |
						  ButtonReleaseMask, &ev))
		if( ev.type == ButtonRelease) return;

	if( !(Scr.flags&OPAQUEMOVE) ){
		XGrabServer( dpy );
		XDrawRectangle( dpy, Scr.Root, Scr.RobberGC, mw->frame_x, mw->frame_y,
					   mw->frame_w, mw->frame_h );
	}

	XSync( dpy, 0 );
	if( !GrabEvent( MOVE ) ){
		XBell( dpy, 30 );
		return;
	}
	while( !isEnd ){
		emask = ButtonReleaseMask|ButtonMotionMask|PointerMotionMask;
		if( Scr.flags&OPAQUEMOVE )
			emask |= ExposureMask;
		XMaskEvent( dpy, emask, &ev );
		if ( ev.type == MotionNotify )
			while(XCheckMaskEvent(dpy, PointerMotionMask | ButtonMotionMask |
								  ButtonReleaseMask, &ev))
				if( ev.type == ButtonRelease) break;
		switch( ev.type ){
		case Expose: handle_expose( &ev ); break;
		case ButtonRelease:
			if( !(Scr.flags&OPAQUEMOVE) )
				XDrawRectangle( dpy, Scr.Root, Scr.RobberGC, mw->frame_x-drag_x,
						   mw->frame_y-drag_y, mw->frame_w, mw->frame_h );
			XSync( dpy, 0 );
			mw->frame_x = mw->frame_x-drag_x;
			mw->frame_y = mw->frame_y-drag_y;
			SetUpFrame( mw, mw->frame_x, mw->frame_y,
					   mw->frame_w, mw->frame_h, True );
			isEnd = True;
			break;
		case MotionNotify:
			XQueryPointer( dpy, ev.xany.window,&JunkRoot, &JunkChild,
						  &x, &y,&JunkX, &JunkY,&JunkMask);
			if( drag_x==pre_x-x && drag_y==pre_y-y ) continue;
			if( !(Scr.flags&OPAQUEMOVE) )
				XDrawRectangle( dpy, Scr.Root, Scr.RobberGC, 
						   mw->frame_x-drag_x, mw->frame_y-drag_y,
						   mw->frame_w, mw->frame_h );

			/* Calc. edge resistance */
			/* edge inside the menu bar */
			if( mw->frame_y-pre_y+y<MENUB_H ){
				if( last_x==-1 )	last_x = x;
			}
			/* edge<0 */
			if( y<MENUB_H )		x=last_x;
			else{
				last_x = -1;
				if( pre_x-x>0 && Scr.resist_x>abs(mw->frame_x-pre_x+x) &&
					mw->frame_x-pre_x+x<0 )
					 x = pre_x-mw->frame_x;
				/* edge>display width */
				if( x-pre_x>0 && Scr.resist_x+Scr.MyDisplayWidth>
					abs(mw->frame_x+mw->frame_w-pre_x+x) &&
					 mw->frame_x+mw->frame_w-pre_x+x>Scr.MyDisplayWidth )
					 x = pre_x-mw->frame_x+Scr.MyDisplayWidth-mw->frame_w;
				/* edge>display height */
				if( y-pre_y>0 && Scr.resist_y+Scr.MyDisplayHeight>
					abs(mw->frame_y+mw->frame_h-pre_y+y) &&
					mw->frame_y+mw->frame_h-pre_y+y>Scr.MyDisplayHeight )
					 y = pre_y-mw->frame_y+Scr.MyDisplayHeight-mw->frame_h;
				XWarpPointer( dpy, None, ev.xany.window, 0, 0, 0, 0, x, y );
				if( mw->frame_y-pre_y+y<MENUB_H ) y=pre_y-mw->frame_y+MENUB_H;

				drag_x = pre_x-x;
				drag_y = pre_y-y;
				if( Scr.flags&OPAQUEMOVE )
					XMoveWindow( dpy, mw->frame,
							   mw->frame_x-drag_x, mw->frame_y-drag_y );
				else
					XDrawRectangle( dpy, Scr.Root, Scr.RobberGC, 
							   mw->frame_x-drag_x, mw->frame_y-drag_y,
							   mw->frame_w, mw->frame_h );
				XSync( dpy, 0 );
			}
			break;
		}
	}
	if( !(Scr.flags&OPAQUEMOVE) ) XUngrabServer( dpy );
	UnGrabEvent();
	KeepOnTop();
}

void DisplayPush( Window win )
{
	XSegment lines_a[4], lines_b[4];

	XClearWindow( dpy, win );
	if( Scr.flags&SYSTEM8 ){
		XFillRectangle( dpy, win, Scr.Gray3GC,
					   0, 0, BOXSIZE-1, BOXSIZE-1 );
		XDrawRectangle( dpy, win, Scr.BlackGC,
					   1, 1, BOXSIZE-3, BOXSIZE-3 );
		DrawShadowBox( 0, 0, BOXSIZE, BOXSIZE, win, 1,
					  Scr.Gray3GC, Scr.WhiteGC, SHADOW_ALL );
		DrawShadowBox( 4, 4, BOXSIZE-6, BOXSIZE-6, win, BOXSIZE/2-2,
					  Scr.Gray4GC, Scr.Gray4GC,
					  SHADOW_BOTTOM|SHADOW_RIGHT );
		DrawShadowBox( 2, 2, BOXSIZE-6, BOXSIZE-6, win, BOXSIZE/2-2,
					  Scr.Gray2GC, Scr.WhiteGC,
					  SHADOW_TOP|SHADOW_LEFT );
		DrawShadowBox( 2, 2, 3, 3, win, 2,
					  Scr.Gray1GC, Scr.WhiteGC,
				  SHADOW_TOP|SHADOW_LEFT );
	}
	else{
		SetSegment( 2, 4, 2, 4, lines_a );
		SetSegment( 2, 4, BOXSIZE-3, BOXSIZE-5, lines_a+1 );
		SetSegment( BOXSIZE-5, BOXSIZE-3, BOXSIZE-5, BOXSIZE-3, lines_a+2 );
		SetSegment( BOXSIZE-3, BOXSIZE-5, 2, 4, lines_a+3 );

		SetSegment( 1, 4, 6, 6, lines_b );
		SetSegment( 6, 6, BOXSIZE-6, BOXSIZE-2, lines_b+1 );
		SetSegment( BOXSIZE-5, BOXSIZE-2, 6, 6, lines_b+2 );
		SetSegment( 6, 6, 1, 5, lines_b+3 );

		XDrawSegments( dpy, win, Scr.Gray1GC, lines_a, 4 );
		XDrawSegments( dpy, win, Scr.BlackGC, lines_b, 4 );
		XDrawRectangle( dpy, win, Scr.BlackGC, 0, 0, BOXSIZE-1, BOXSIZE-1 );
	}
	XSync( dpy, 0 );
}

void CloseWindow( MlvwmWindow *mw, XEvent *evp )
{
	Bool isEnd = False, isIn = True;
	XEvent ev;
	int JunkX, JunkY;
	Window JunkRoot;
	unsigned int JunkWidth, JunkHeight, JunkBW, JunkDepth;

	DisplayPush( mw->close_b );
	while( !isEnd ){
		XMaskEvent( dpy,ButtonReleaseMask|EnterWindowMask|LeaveWindowMask, &ev );
		switch( ev.type ){
		  case ButtonRelease:
			isEnd = True;
			break;
		  case EnterNotify:
			if( ev.xcrossing.window==mw->close_b ){
				DisplayPush( mw->close_b );
				isIn = True;
			}
			break;
		  case LeaveNotify:
			if( ev.xcrossing.window==mw->close_b ){
				DrawCloseBox( mw, True );
				isIn = False;
			}
			break;
		}
	}


	if( isIn ){
		if ( mw->flags & DoesWmDeleteWindow)
			send_clientmessage( mw->w, _XA_WM_DELETE_WINDOW, CurrentTime);
		else if (XGetGeometry(dpy, mw->w, &JunkRoot, &JunkX, &JunkY,
							  &JunkWidth, &JunkHeight, &JunkBW, &JunkDepth)
				 == 0)
			Destroy( mw );
		else
			XKillClient(dpy, mw->w);
	}
	XSync( dpy, 0 );
}

void DrawResizeFrame( int x, int y, int w, int h, MlvwmWindow *t )
{
	XSegment lines[10];
	int nl=0;

	SetSegment( x, x+w, y, y, lines+nl );		nl++;
	SetSegment( x+w, x+w, y, y+h, lines+nl );	nl++;
	SetSegment( x, x+w, y+h, y+h, lines+nl );	nl++;
	SetSegment( x, x, y, y+h, lines+nl );	nl++;

	if( Scr.flags&SYSTEM8 ){
		SetSegment( x+6, x+6, y+TITLE_HEIGHT+1, y+h-6, lines+nl ); nl++;
		SetSegment( x+w-6, x+w-6,
				   y+TITLE_HEIGHT+1, y+h-6-(t->flags&RESIZER?SBAR_WH:0),
				   lines+nl );
		nl++;
		SetSegment( x+6, x+w-6-(t->flags&RESIZER?SBAR_WH:0),
				   y+h-6, y+h-6, lines+nl );
		nl++;
		if( t->flags&RESIZER ){
			SetSegment( x+w-SBAR_WH-6, x+w-SBAR_WH-6,
					   y+h-SBAR_WH-6, y+h-6, lines+nl );
			nl++;
			SetSegment( x+w-SBAR_WH-6, x+w-6,
					   y+h-SBAR_WH-6, y+h-SBAR_WH-6, lines+nl );
			nl++;
		}
		if( t->flags&TITLE ){
			SetSegment( x+6, x+w-6,
					   y+TITLE_HEIGHT+1, y+TITLE_HEIGHT+1, lines+nl );
			nl++;
		}
	}
	else{
		if( t->flags&SBARH ){
			SetSegment( x, x+w, y+h-2-SBAR_WH, y+h-2-SBAR_WH, lines+nl );
			nl++;
		}
		if( t->flags&SBARV ){
			SetSegment( x+w-2-SBAR_WH, x+w-2-SBAR_WH,
					   y+(t->flags&TITLE ? TITLE_HEIGHT : -1 )+1, y+h,
					   lines+nl );
			nl++;
		}
		if( t->flags&TITLE ){
			SetSegment( x, x+w, y+TITLE_HEIGHT+1, y+TITLE_HEIGHT+1, lines+nl );
			nl++;
		}
	}

	XDrawSegments( dpy, Scr.Root, Scr.RobberGC, lines, nl );
	XSync( dpy, 0 );
}

void ConstrainSize( MlvwmWindow *tmp_win, int *widthp, int *heightp )
{
#define makemult(a,b) ((b==1) ? (a) : (((int)((a)/(b))) * (b)) )
#define _min(a,b) (((a) < (b)) ? (a) : (b))

    int minWidth, minHeight, maxWidth, maxHeight, xinc, yinc, delta;
    int baseWidth, baseHeight;
    int dwidth, dheight;
	int title_height, sbar_v, sbar_h;

	title_height = tmp_win->flags & TITLE ? TITLE_HEIGHT : -1;
	sbar_v = tmp_win->flags & SBARV ? SBAR_WH+2 : 1;
	sbar_h = tmp_win->flags & SBARH ? SBAR_WH+2: 1;

	if( tmp_win->flags & ( TITLE | SBARV | SBARH | RESIZER ) ){
		dwidth = *widthp-1-sbar_v;
		dheight = *heightp-2-sbar_h-title_height;
		if( Scr.flags&SYSTEM8 ){
			dwidth -= 12;
			dheight -= 6;
		}
	}
	else{
		dwidth = *widthp-12;
		dheight = *heightp-12;
	}
    if (tmp_win->hints.flags & PMinSize) {
        minWidth = tmp_win->hints.min_width;
        minHeight = tmp_win->hints.min_height;
    } else if (tmp_win->hints.flags & PBaseSize) {
        minWidth = tmp_win->hints.base_width;
        minHeight = tmp_win->hints.base_height;
    } else
        minWidth = minHeight = 1;

    if (tmp_win->hints.flags & PBaseSize) {
        baseWidth = tmp_win->hints.base_width;
        baseHeight = tmp_win->hints.base_height;
    } 
	else if (tmp_win->hints.flags & PMinSize) {
        baseWidth = tmp_win->hints.min_width;
        baseHeight = tmp_win->hints.min_height;
    }
	else
        baseWidth = baseHeight = 0;

	if (tmp_win->hints.flags & PMinSize) {
		baseWidth = tmp_win->hints.min_width;
		baseHeight = tmp_win->hints.min_height;
	}
	else
		baseWidth = baseHeight = 0;


    if (tmp_win->hints.flags & PMaxSize) {
        maxWidth = tmp_win->hints.max_width;
        maxHeight = tmp_win->hints.max_height;
    }
	else {
        maxWidth = MAX_WINDOW_WIDTH;
        maxHeight = MAX_WINDOW_HEIGHT;
    }

	if (tmp_win->hints.flags & PResizeInc) {
		xinc = tmp_win->hints.width_inc;
        yinc = tmp_win->hints.height_inc;
    } else
        xinc = yinc = 1;
    /*
     * First, clamp to min and max values
     */
    if (dwidth < minWidth) dwidth = minWidth;
    if (dheight < minHeight) dheight = minHeight;
	
    if (dwidth > maxWidth) dwidth = maxWidth;
    if (dheight > maxHeight) dheight = maxHeight;
	
    /*
     * Second, fit to base + N * inc
     */
	dwidth = ((dwidth - baseWidth) / xinc * xinc) + baseWidth;
	dheight = ((dheight - baseHeight) / yinc * yinc) + baseHeight;
	
    /*
     * Third, adjust for aspect ratio
     */
#define maxAspectX tmp_win->hints.max_aspect.x
#define maxAspectY tmp_win->hints.max_aspect.y
#define minAspectX tmp_win->hints.min_aspect.x
#define minAspectY tmp_win->hints.min_aspect.y
    /*
     * The math looks like this:
     *
     * minAspectX    dwidth     maxAspectX
     * ---------- <= ------- <= ----------
     * minAspectY    dheight    maxAspectY
     *
     * If that is multiplied out, then the width and height are
     * invalid in the following situations:
     *
     * minAspectX * dheight > minAspectY * dwidth
     * maxAspectX * dheight < maxAspectY * dwidth
     * 
     */
    
	if (tmp_win->hints.flags & PAspect){
        if (minAspectX * dheight > minAspectY * dwidth){
            delta = makemult(minAspectX * dheight / minAspectY - dwidth,
                             xinc);
            if (dwidth + delta <= maxWidth) dwidth += delta;
            else{
                delta = makemult(dheight - dwidth*minAspectY/minAspectX,
                                 yinc);
                if (dheight - delta >= minHeight) dheight -= delta;
            }
        }
		
        if (maxAspectX * dheight < maxAspectY * dwidth){
            delta = makemult(dwidth * maxAspectY / maxAspectX - dheight,
                             yinc);
            if (dheight + delta <= maxHeight) dheight += delta;
            else{
				delta = makemult(dwidth - maxAspectX*dheight/maxAspectY,
                                 xinc);
                if (dwidth - delta >= minWidth) dwidth -= delta;
            }
        }
    }

    /*
     * Fourth, account for border width and title height
     */
	if( tmp_win->flags & ( TITLE | SBARV | SBARH | RESIZER ) ){
		if( Scr.flags&SYSTEM8 ){
			dwidth += 12;
			dheight += 6;
		}
		*widthp = dwidth + sbar_v + 1;
		*heightp = dheight + sbar_h + title_height + 2;
	}
	else{
		*widthp = dwidth + 12;
		*heightp = dheight + 12;
	}
}

void ResizeWindow( MlvwmWindow *mw, XEvent *evp, Bool s_move )
{
	Bool isEnd = False;
	int pre_x, pre_y, new_w, new_h, org_w, org_h;
	int x, y, JunkX, JunkY;
	unsigned int JunkMask, evmask;
	int xmotion, ymotion;
	Window JunkRoot, JunkChild;
	XEvent ev;

	pre_x = evp->xbutton.x_root;
	pre_y = evp->xbutton.y_root;
	org_w = new_w = mw->frame_w;
	org_h = new_h = mw->frame_h;
	if( s_move ){
		xmotion=1;
		ymotion=1;
	}
	else{
		xmotion=0;
		ymotion=0;
	}
	if( !GrabEvent( MOVE ) ){
		XBell( dpy, 30 );
		return;
	}
	if( !(Scr.flags&OPAQUERESIZE) ){
		XGrabServer( dpy );
		DrawResizeFrame(mw->frame_x,mw->frame_y,mw->frame_w,mw->frame_h,mw);
	}
	while( !isEnd ){
		evmask = ButtonReleaseMask|ButtonMotionMask|PointerMotionMask;
		if( Scr.flags&OPAQUERESIZE )	evmask |= ExposureMask;
		XMaskEvent( dpy, evmask, &ev);
		if ( ev.type == MotionNotify )
			while(XCheckMaskEvent(dpy, PointerMotionMask | ButtonMotionMask |
								  ButtonReleaseMask, &ev))
				if( ev.type == ButtonRelease) break;
		switch( ev.type ){
		case Expose: handle_expose( &ev ); break;
		case ButtonRelease:
			if( !(Scr.flags&OPAQUERESIZE) )
				DrawResizeFrame( mw->frame_x, mw->frame_y, new_w, new_h, mw );
			XSync( dpy, 0 );
			mw->frame_w = new_w;
			mw->frame_h = new_h;
			SetUpFrame( mw, mw->frame_x, mw->frame_y,
					   mw->frame_w, mw->frame_h, False );
			isEnd = True;
			break;
		case MotionNotify:
			XQueryPointer( dpy, ev.xany.window,&JunkRoot, &JunkChild,
						  &x, &y,&JunkX, &JunkY,&JunkMask);
			if( xmotion==0 && x<=mw->frame_x ){
				xmotion=-1;
				pre_x = mw->frame_x;
			}
			if( xmotion==0 && x>mw->frame_x + mw->frame_w ){
				xmotion=1;
				pre_x = x;
			}
			if( ymotion==0 && y<=mw->frame_y ){
				ymotion=-1;
				pre_y = mw->frame_y;
			}
			if( ymotion==0 && y>=mw->frame_y + mw->frame_h ){
				ymotion=1;
				pre_y = y;
			}
			if( xmotion!=0 || ymotion!=0 ){
				if( !(Scr.flags&OPAQUERESIZE) )
					DrawResizeFrame(mw->frame_x,mw->frame_y,new_w,new_h,mw);
				if( xmotion!=0 )
					new_w = org_w + (x - pre_x)*xmotion;
				if( ymotion!=0 )
					new_h = org_h + (y - pre_y)*ymotion;

				if( new_w<(mw->flags&SBARH?4*SBAR_WH:0) )
					new_w = (mw->flags*SBARH?4*SBAR_WH:0)+1;
				if( new_h<(mw->flags&SBARV?4*SBAR_WH:0)+
								(mw->flags&TITLE?TITLE_HEIGHT:0) )
					new_h = (mw->flags&SBARV?4*SBAR_WH:0)+
								(mw->flags&TITLE?TITLE_HEIGHT:0)+1;

				ConstrainSize( mw, &new_w, &new_h );

				if( xmotion==-1 )		mw->frame_x = pre_x+mw->frame_w-new_w;
				if( ymotion==-1 )		mw->frame_y = pre_y+mw->frame_h-new_h;

				if( Scr.flags&OPAQUERESIZE ){
					if( mw->frame_w!=new_w ) mw->frame_w = new_w;
					if( mw->frame_h!=new_h ) mw->frame_h = new_h;
					SetUpFrame( mw,mw->frame_x,mw->frame_y,new_w,new_h,False );
				}
				else
					DrawResizeFrame( mw->frame_x, mw->frame_y, new_w, new_h, mw );
			}
			break;
		}
	}
	if( !(Scr.flags&OPAQUERESIZE) )
		XUngrabServer( dpy );
	UnGrabEvent();
}

void MinMaxWindow( MlvwmWindow *mw, XEvent *evp )
{
	Bool isEnd = False, isIn = True;
	XEvent ev;

	DisplayPush( mw->minmax_b );
	XDrawRectangle( dpy, mw->minmax_b, Scr.BlackGC,
                   1, 1, BOXSIZE-7, BOXSIZE-7 );
	while( !isEnd ){
		XMaskEvent( dpy,
				   ButtonReleaseMask|EnterWindowMask|LeaveWindowMask, &ev );
		switch( ev.type ){
		  case ButtonRelease:
			DrawMinMax( mw, True );
			XSync( dpy, 0 );
			isEnd = True;
			break;
		  case EnterNotify:
			if( ev.xcrossing.window==mw->minmax_b ){
				DisplayPush( mw->minmax_b );
				XDrawRectangle( dpy, mw->minmax_b, Scr.BlackGC,
                               1, 1, BOXSIZE-6, BOXSIZE-6 );
				isIn = True;
			}
			break;
		  case LeaveNotify:
			if( ev.xcrossing.window==mw->minmax_b ){
				DrawMinMax( mw, True );
				XSync( dpy, 0 );
				isIn = False;
			}
			break;
		}
	}
	if( isIn ){
		if( mw->flags &SHADE )
			UnShadeWindow( mw );
		if( mw->flags & MAXIMAIZED ){
			if( mw->flags&SCROLL ){
				mw->win_x = mw->orig_win_x;
				mw->win_y = mw->orig_win_y;
			}
			mw->frame_x = mw->orig_x;
			mw->frame_y = mw->orig_y;
			mw->frame_w = mw->orig_w;
			mw->frame_h = mw->orig_h;
			mw->flags &= ~MAXIMAIZED;
		}
		else{
			mw->orig_x = mw->frame_x;
			mw->orig_y = mw->frame_y;
			mw->orig_w = mw->frame_w;
			mw->orig_h = mw->frame_h;
			mw->frame_x = 0;
			mw->frame_y = MENUB_H;
			if( mw->flags&SCROLL ){
				mw->orig_win_x = mw->win_x;
				mw->orig_win_y = mw->win_y;
				mw->win_x = 0;
				mw->win_y = 0;
			}
			if( mw->flags&SCROLL &&
			   (mw->frame_w<mw->win_w ||
			   mw->frame_h<mw->win_h) ){
				mw->frame_w = mw->win_w;
				mw->frame_h = mw->win_h;
			}
			else{
				mw->frame_w = mw->size_w;
				mw->frame_h = mw->size_h;
			}
			ConstrainSize( mw, &(mw->frame_w), &(mw->frame_h) );
			mw->flags |= MAXIMAIZED;
		}
		SetUpFrame( mw, mw->frame_x, mw->frame_y,
				   mw->frame_w, mw->frame_h, True );
	}
}

void ShadeBox( MlvwmWindow *mw, XEvent *evp )
{
	Bool isEnd = False, isIn = True;
	XEvent ev;

	DisplayPush( mw->shade_b );
	XDrawLine( dpy, mw->shade_b, Scr.BlackGC,
			  1, BOXSIZE/2-1, BOXSIZE-3, BOXSIZE/2-1 );
	XDrawLine( dpy, mw->shade_b, Scr.BlackGC,
			  1, BOXSIZE/2+1, BOXSIZE-3, BOXSIZE/2+1 );
	while( !isEnd ){
		XMaskEvent( dpy,
				   ButtonReleaseMask|EnterWindowMask|LeaveWindowMask, &ev );
		switch( ev.type ){
		  case ButtonRelease:
			DrawShadeR( mw, True );
			XSync( dpy, 0 );
			isEnd = True;
			break;
		  case EnterNotify:
			if( ev.xcrossing.window==mw->shade_b ){
				DisplayPush( mw->shade_b );
				XDrawLine( dpy, mw->shade_b, Scr.BlackGC,
						  1, BOXSIZE/2-1, BOXSIZE-3, BOXSIZE/2-1 );
				XDrawLine( dpy, mw->shade_b, Scr.BlackGC,
						  1, BOXSIZE/2+1, BOXSIZE-3, BOXSIZE/2+1 );
				isIn = True;
			}
			break;
		  case LeaveNotify:
			if( ev.xcrossing.window==mw->shade_b ){
				DrawShadeR( mw, True );
				XSync( dpy, 0 );
				isIn = False;
			}
			break;
		}
	}
	if( isIn ){
		if( mw->flags&SHADE )
			UnShadeWindow( mw );
		else
			ShadeWindow( mw );
	}
}

void DoubleClickEvent( XEvent *ev )
{
	MlvwmWindow *Tmp_win;
	int context;
	Window win;

	if( XFindContext( dpy, ev->xany.window, MlvwmContext, (caddr_t *)&Tmp_win )
	   != XCNOENT ){
		context = GetContext( Tmp_win, ev, &win );
		if( context == C_TITLE ){
			if( Tmp_win->flags&SHADE )
				UnShadeWindow( Tmp_win );
			else
				ShadeWindow( Tmp_win );
		}
	}
}

/***
  isHbar  if scroll bar is Horizontal, then true
***/
void PressSbarAnker( Bool isHbar, MlvwmWindow *tmp_win )
{
	Window JunkRoot, JunkChild, bar, anker;
	XEvent ev;
	XWindowAttributes winattrs;
	Bool isEnd = False;
	int *mouse_p, *mouse_w, zero=0, *anker_x, *anker_y, *vector, anker_end;
	int x, y, JunkX, JunkY;
	unsigned int JunkMask;
	int sub_edge=0, anker_calc, init_posi, hide_size;

	if( !(tmp_win->flags&SCROLL) || 
	   (tmp_win->frame_w>=tmp_win->win_w && isHbar) ||
	   (tmp_win->frame_h>=tmp_win->win_h && !isHbar) )
		return;
	if( isHbar ){
		XGetWindowAttributes( dpy, tmp_win->scroll_h[3], &winattrs);
		init_posi = winattrs.x;
		mouse_p = &x;
		mouse_w = &y;
		anker = tmp_win->scroll_h[3];
		bar = tmp_win->scroll_h[0];
		anker_x = &anker_calc;
		anker_y = &zero;
		XGetWindowAttributes( dpy, tmp_win->scroll_h[0], &winattrs);
		anker_end = winattrs.width-SBAR_WH-1-SBAR_WH;
		vector = &(tmp_win->win_x);
		hide_size = tmp_win->frame_w-tmp_win->win_w;
	}
	else{
		XGetWindowAttributes( dpy, tmp_win->scroll_v[3], &winattrs);
		init_posi = winattrs.y;
		mouse_w = &x;
		mouse_p = &y;
		anker = tmp_win->scroll_v[3];
		bar = tmp_win->scroll_v[0];
		anker_x = &zero;
		anker_y = &anker_calc;
		XGetWindowAttributes( dpy, tmp_win->scroll_v[0], &winattrs);
		anker_end = winattrs.height-SBAR_WH-1-SBAR_WH;
		vector = &(tmp_win->win_y);
		hide_size = tmp_win->frame_h-tmp_win->win_h;
	}

	if( XQueryPointer( dpy, bar, &JunkRoot, &JunkChild,
					  &JunkX, &JunkY, &x, &y, &JunkMask ) )
		sub_edge = *mouse_p-init_posi;
	if( !GrabEvent( DEFAULT ) ){
		XBell( dpy, 30 );
		return;
	}
	XDrawRectangle( dpy, anker, Scr.BlackGC, 0, 0, SBAR_WH-1, SBAR_WH-1 );
	XSync( dpy, 0 );
	do{
		if(	XCheckMaskEvent( dpy, ButtonReleaseMask | ExposureMask |
							ButtonMotionMask | PointerMotionMask |
							PointerMotionHintMask, &ev ) ){
			if( ev.type==MotionNotify )
				while( XCheckMaskEvent( dpy, PointerMotionMask | 
									   ButtonMotionMask | ButtonReleaseMask,
									   &ev ) )
					if( ev.type == ButtonRelease )		break;
			switch( ev.type ){
			  case ButtonRelease:		isEnd = True;
				break;
			  case MotionNotify:
				if( XQueryPointer( dpy, bar, &JunkRoot, &JunkChild,
								 &JunkX, &JunkY, &x, &y, &JunkMask ) &&
				   *mouse_w>-2*SBAR_WH && *mouse_w<3*SBAR_WH){
					anker_calc = *mouse_p-sub_edge;
					if( anker_calc<=SBAR_WH+1 )	anker_calc = SBAR_WH+1;
					if( anker_calc>anker_end )	anker_calc = anker_end;
					XMoveWindow( dpy, anker, *anker_x, *anker_y );
				}
			}
		}
	}
	while( !isEnd );
	*vector = (double)((anker_calc-SBAR_WH-1)*hide_size)/
		(double)(anker_end-SBAR_WH-1);
	SetUpFrame( tmp_win, tmp_win->frame_x, tmp_win->frame_y,
			   tmp_win->frame_w, tmp_win->frame_h, True );
	UnGrabEvent();
}

/***
  isHbar  if scroll bar is Horizontal, then true
***/
void PressSbar( Bool isHbar, Window pressedwin, MlvwmWindow *tmp_win )
{
	int *vector, pushd, pre_vector, direction=0;
	Bool isEnd = False, isSelect = True;
	int view, world, inc_view, inc=1;
	int x, y, JunkX, JunkY;
	unsigned int JunkMask;
	int *check_axis, *anker_check, timeout=1;
	struct timeval tp, current;
	Window JunkRoot, JunkChild, bar, anker;
	XWindowAttributes winattrs, winattrs_a;
	int ignore=1;
	XEvent ev;

	if( !(tmp_win->flags&SCROLL) || 
	   (tmp_win->frame_w>=tmp_win->win_w && isHbar) ||
	   (tmp_win->frame_h>=tmp_win->win_h && !isHbar) )
		return;
	XGetWindowAttributes(dpy, tmp_win->Parent, &winattrs);
	if( isHbar ){
		vector = &(tmp_win->win_x);
		view = tmp_win->frame_w;
		world = tmp_win->win_w;
		for( pushd=0; pushd<4 &&
			tmp_win->scroll_h[pushd]!=pressedwin; pushd++ );
		if( pushd==1 )	direction = C_SBAR_LEFT;
		if( pushd==2 )	direction = C_SBAR_RIGHT;
		bar = tmp_win->scroll_h[0];
		if (tmp_win->hints.flags & PResizeInc)
			inc = tmp_win->hints.width_inc;
		inc_view = winattrs.width;
		check_axis = &x;
		anker_check = &(winattrs_a.x);
		anker = tmp_win->scroll_h[3];
	}
	else{
		vector = &(tmp_win->win_y);
		view = tmp_win->frame_h;
		world = tmp_win->win_h;
		for( pushd=0; pushd<4 &&
			tmp_win->scroll_v[pushd]!=pressedwin; pushd++ );
		if( pushd==1 )	direction = C_SBAR_UP;
		if( pushd==2 )	direction = C_SBAR_DOWN;
		bar = tmp_win->scroll_v[0];
		if( tmp_win->hints.flags & PResizeInc )
			inc = tmp_win->hints.height_inc;
		inc_view = winattrs.height;
		check_axis = &y;
		anker_check = &(winattrs_a.y);
		anker = tmp_win->scroll_v[3];
	}
	if( pushd!=0 ){
		if( Scr.flags&SYSTEM8 )
			DrawShadowBox( 0, 0, SBAR_WH, SBAR_WH, pressedwin, 1, Scr.Gray2GC, Scr.WhiteGC, SHADOW_ALL );
		else
			DrawArrow( pressedwin, direction, Scr.BlackGC, Scr.BlackGC );
		XSync( dpy, 0 );
	}

	pre_vector = *vector;

	tp.tv_usec = 0;
	tp.tv_sec = 0;
	do{
		if( timeout && isSelect && !isEnd ){
			switch( pushd ){
			  case 1:	(*vector)+=inc;		break;
			  case 2:	(*vector)-=inc;		break;
			  case 0:
				XQueryPointer( dpy, pressedwin,&JunkRoot, &JunkChild,
							  &JunkX, &JunkY, &x, &y, &JunkMask);
				XGetWindowAttributes(dpy, anker, &winattrs_a);
				if( *check_axis>*anker_check+SBAR_WH )	(*vector)-=inc_view;
				if( *check_axis<*anker_check )			(*vector)+=inc_view;
				break;
			}
			*vector = view-world > *vector ? view-world : *vector;
			*vector = *vector>0 ? 0 : *vector;

			if( pre_vector != *vector ){
				SetUpFrame( tmp_win, tmp_win->frame_x, tmp_win->frame_y,
						   tmp_win->frame_w, tmp_win->frame_h, True );
				XSync( dpy, 0 );
				pre_vector = *vector;
			}
		}
		if( XPending(dpy)){
			if(XCheckMaskEvent( dpy, ButtonReleaseMask|EnterWindowMask|
				LeaveWindowMask|ExposureMask, &ev ) ){
				switch( ev.type ){
				case ButtonRelease:
					isEnd = True;
					timeout = 1;
					break;
				case EnterNotify:
					if( ev.xcrossing.window==pressedwin ){
						if( !isSelect ) {
							if( Scr.flags&SYSTEM8 )
								DrawShadowBox( 0, 0, SBAR_WH, SBAR_WH, pressedwin, 1, Scr.Gray2GC, Scr.WhiteGC, SHADOW_ALL );
							else
								DrawArrow( pressedwin, direction, Scr.BlackGC, Scr.BlackGC );
						}
						isSelect = True;
					}
					break;
				case LeaveNotify:
					if( ev.xcrossing.window==pressedwin ){
						if( isSelect ) {
							if( Scr.flags&SYSTEM8 )
								DrawShadowBox( 0, 0, SBAR_WH, SBAR_WH, pressedwin, 1, Scr.WhiteGC, Scr.Gray2GC, SHADOW_ALL );
							else
								DrawArrow( pressedwin, direction, Scr.Gray3GC, Scr.BlackGC );
						}
						isSelect = False;
					}
					break;
				}
			}
		}
		if( timeout ){
			gettimeofday( &tp, NULL );
			if( ignore ){
				tp.tv_usec += 1000000;
				ignore = 0;
			}
			else
				tp.tv_usec += 100000;
			
			tp.tv_sec += tp.tv_usec/1000000;
			tp.tv_usec = tp.tv_usec%1000000;
			timeout = 0;
		}

		gettimeofday( &current, NULL );

		if( (tp.tv_sec-current.tv_sec)*1000000-(tp.tv_usec-current.tv_usec)<0 ){
			timeout = 1;
		}
		else{
			if( !isEnd )	sleep_a_little( 20000 );
		}
	}
	while( !isEnd );

	if( pushd!=0 ) {
		if( Scr.flags&SYSTEM8 )
			DrawShadowBox( 0, 0, SBAR_WH, SBAR_WH, pressedwin, 1, Scr.WhiteGC, Scr.Gray2GC, SHADOW_ALL );
		else
			DrawArrow( pressedwin, direction, Scr.Gray3GC, Scr.BlackGC );
	}
	XSync( dpy, 0 );
}

void handle_button_press( XEvent *ev )
{
	MlvwmWindow *Tmp_win;
	MenuLabel *tmp_menu;
	Window win;
	static Time PressTime=0;
	static int Click_x, Click_y;
	int context;
	Bool newactive=False;

	if( ev->xbutton.time-PressTime<Scr.double_click_time &&
	   ev->xbutton.x>Click_x-4 && ev->xbutton.x<Click_x+4 &&
	   ev->xbutton.y>Click_y-4 && ev->xbutton.y<Click_y+4 &&
	   ev->xbutton.button == Button1  ){
		DoubleClickEvent( ev );
		PressTime=0;

		return;
	}
	else{
		PressTime = ev->xbutton.time;
		Click_x = ev->xbutton.x;
		Click_y = ev->xbutton.y;
	}
	if( XFindContext( dpy, ev->xany.window, MlvwmContext, (caddr_t *)&Tmp_win )
	   != XCNOENT ){
		context = GetContext( Tmp_win, ev, &win );
		XGrabServer( dpy );
		if( (Tmp_win!=Scr.ActiveWin || Scr.flags&FOLLOWTOMOUSE) )
			RaiseMlvwmWindow( Tmp_win );
		XUngrabServer( dpy );

		if( Tmp_win != Scr.ActiveWin  ){
			SetFocus( Tmp_win );
			PressTime = 0;
			newactive = True;
			if( ev->xbutton.subwindow!=None ){
				ev->xany.window = ev->xbutton.subwindow;
				context = GetContext( Tmp_win, ev, &win );
			}
			XAllowEvents(dpy, AsyncPointer, CurrentTime ); 
		}
		else
			XAllowEvents(dpy, ReplayPointer, CurrentTime ); 
		XSync(dpy,0);

		if( newactive && context!=C_TITLE )		return;
		if( ev->xbutton.button != Button1 )						return;

		switch( context ){
		  case C_TITLE:		MoveWindow( Tmp_win, ev );		break;
		  case C_CLOSE:		CloseWindow( Tmp_win, ev );		break;
		  case C_MINMAX:	MinMaxWindow( Tmp_win, ev );	break;
		  case C_SHADE:     ShadeBox( Tmp_win, ev );		break;
		  case C_RESIZE:	ResizeWindow( Tmp_win, ev, True );		break;
		  case C_SBAR_V:
		  case C_SBAR_UP:
		  case C_SBAR_DOWN:
			PressSbar( False, ev->xany.window, Tmp_win );
			break;
		  case C_SBAR_H:
		  case C_SBAR_LEFT:
		  case C_SBAR_RIGHT:
			PressSbar( True, ev->xany.window, Tmp_win );
			break;
		  case C_SBAR_V_AN:		PressSbarAnker( False, Tmp_win );	break;
		  case C_SBAR_H_AN:		PressSbarAnker( True, Tmp_win );	break;
		  case C_FRAME:
			if( Scr.flags&SYSTEM8)MoveWindow( Tmp_win, ev );		break;
		}
	}
	else{
		if( ev->xbutton.button != Button1 )						return;
		if( XFindContext( dpy, ev->xany.window,
						 MenuContext, (caddr_t *)&tmp_menu )==XCNOENT )
			tmp_menu = NULL;
		if( ev->xbutton.y<=MENUB_H )
			press_menu( tmp_menu );
	}

	if( !(Scr.flags&FOLLOWTOMOUSE) && Scr.ActiveWin != NULL &&
	   GetContext( Scr.ActiveWin, ev, &win )==C_ROOT )
		SetFocus( NULL );
	if( Scr.flags&SLOPPYFOCUS && Scr.ActiveWin != NULL && 
	   GetContext( Scr.ActiveWin, ev, &win )==C_ROOT ) 
		SetFocus( NULL );

	KeepOnTop();
}

void handle_expose( XEvent *ev )
{
	MlvwmWindow *Tmp_win, *ActiveWin;
	MenuLabel *Tmp_menu;
	int context;
	Window win;

	if( Scr.ActiveWin==NULL && Scr.PreviousActive!=NULL )
		ActiveWin = Scr.PreviousActive;
	else
		ActiveWin = Scr.ActiveWin;
	
	if( ev->xexpose.count != 0 )	return;
	if( XFindContext( dpy, ev->xany.window, MlvwmContext, (caddr_t *)&Tmp_win )
	   != XCNOENT ){
		context = GetContext( Tmp_win, ev, &win );
		switch( context ){
		case C_TITLE:
			SetTitleBar( Tmp_win, (ActiveWin==Tmp_win?True:False) );
			break;
		case C_SBAR_V:
			DrawSbarBar( Tmp_win, C_SBAR_V, (ActiveWin==Tmp_win?True:False) );
			break;
		case C_SBAR_UP: case C_SBAR_DOWN:
		case C_SBAR_LEFT: case C_SBAR_RIGHT:
	        DrawSbarArrow( Tmp_win, context, (ActiveWin==Tmp_win?True:False) );
			break;
		case C_SBAR_V_AN:
			DrawSbarAnk(Tmp_win, C_SBAR_V_AN ,(ActiveWin==Tmp_win?True:False));
			break;
		case C_SBAR_H:
			DrawSbarBar( Tmp_win, C_SBAR_H, (ActiveWin==Tmp_win?True:False) );
			break;
		case C_SBAR_H_AN:
			DrawSbarAnk(Tmp_win, C_SBAR_H_AN ,(ActiveWin==Tmp_win?True:False));
			break;
		case C_RESIZE:
			DrawResizeBox( Tmp_win, (ActiveWin==Tmp_win?True:False));
			break;
		case C_FRAME:
			DrawFrameShadow( Tmp_win, (ActiveWin==Tmp_win?True:False) );
			break;
		}
	}
	if( ev->xany.window == Scr.MenuBar )
		RedrawMenuBar();
	if( XFindContext( dpy, ev->xany.window, MenuContext, (caddr_t *)&Tmp_menu )
	   != XCNOENT )
		RedrawMenu( Tmp_menu, False );
}

void HandleEnterNotify( XEvent *ev )
{
	MlvwmWindow *tmp_win;
	XEnterWindowEvent *ewp = &(ev->xcrossing);
	XEvent d;

	if(XCheckTypedWindowEvent (dpy, ewp->window, LeaveNotify, &d))
		if((d.xcrossing.mode==NotifyNormal)&&(d.xcrossing.detail!=NotifyInferior))
			return;
	if( ev->xany.window == Scr.Root ){
		if ( Scr.flags&FOLLOWTOMOUSE && !(Scr.flags&SLOPPYFOCUS)) 
			SetFocus( NULL );
		InstallWindowColormaps( &Scr.MlvwmRoot );
		return;
	}
	if( XFindContext ( dpy, ev->xany.window, MlvwmContext, (caddr_t *)&tmp_win )
	   != XCNOENT ){
		if( Scr.flags&FOLLOWTOMOUSE )			SetFocus( tmp_win );
		if( ev->xcrossing.window==tmp_win->w )
			InstallWindowColormaps( tmp_win );
	}
}

void HandleLeaveNotify( XEvent *ev )
{
	MlvwmWindow *tmp_win;
	XEvent dummy;

	if( ev->xcrossing.window == Scr.Root)	return;
	if(XCheckTypedWindowEvent (dpy, ev->xcrossing.window, EnterNotify, &dummy))
		return;
	if( XFindContext ( dpy, ev->xany.window, MlvwmContext, (caddr_t *)&tmp_win )
	   == XCNOENT )
		return;

	if ( ev->xcrossing.detail != NotifyInferior ){
		if( ev->xcrossing.window==tmp_win->frame ){
			if( (Scr.flags & FOLLOWTOMOUSE) && !(Scr.flags&SLOPPYFOCUS) ){
				if( Scr.ActiveWin == tmp_win )
					SetFocus( NULL );
            }
			InstallWindowColormaps (&Scr.MlvwmRoot);
        }
		else{
			if( ev->xcrossing.window==tmp_win->w )
				InstallWindowColormaps( &Scr.MlvwmRoot );
		}
	}
}

void HandleShapeNotify ( XEvent *ev )
{
	MlvwmWindow *Tmp_win;
	XShapeEvent *sev = (XShapeEvent *) ev;

	if( XFindContext ( dpy, ev->xany.window, MlvwmContext, (caddr_t *)&Tmp_win )
	   == XCNOENT )
		return;
	if (sev->kind != ShapeBounding)
		return;
	Tmp_win->wShaped = sev->shaped;
	SetShape(Tmp_win,Tmp_win->frame_w);
}

void handle_unmap_request( XEvent *ev )
{
	int dstx, dsty;
	Window dumwin;
	XEvent dummy;
	MlvwmWindow *tmp_win;
	Bool reparented;

	if( ev->xunmap.event != ev->xunmap.window )		return;
	if( XFindContext( dpy, ev->xany.window, MlvwmContext, (caddr_t *)&tmp_win )
	   ==XCNOENT ){
		ev->xany.window = ev->xunmap.window;
		if( XFindContext( dpy, ev->xany.window, MlvwmContext, (caddr_t *)&tmp_win )
		   ==XCNOENT )
			return;
	}

	if ( !(tmp_win->flags & MAPPED) )      return;
	XGrabServer( dpy );

	if(XCheckTypedWindowEvent (dpy, ev->xunmap.window, DestroyNotify, &dummy)){
/*		ev->xany.window = ev->xunmap.window;*/
		ev->xdestroywindow.window = ev->xunmap.window;
		HandleDestroy( ev );
		XUngrabServer( dpy );
		return;
	} 

	if( XTranslateCoordinates ( dpy, ev->xunmap.window, Scr.Root,
							   0, 0, &dstx, &dsty, &dumwin ) ){
		reparented = XCheckTypedWindowEvent (dpy, ev->xunmap.window, 
									ReparentNotify, &dummy);
		SetMapStateProp (tmp_win, WithdrawnState);
		if( !reparented )
			RestoreWithdrawnLocation (tmp_win,False);
		XRemoveFromSaveSet (dpy, ev->xunmap.window);
		XSelectInput (dpy, ev->xunmap.window, NoEventMask);
/*		ev->xany.window = ev->xunmap.window;*/
		ev->xdestroywindow.window = ev->xunmap.window;
		HandleDestroy( ev );
	}
	XUngrabServer( dpy );
	XFlush (dpy);
}

void handle_property_request( XEvent *ev )
{
	MlvwmWindow *tmp_win;
	unsigned char *prop = NULL;
	Atom actual = None;
	int actual_format;
	unsigned long nitems, bytesafter;
	int JunkX = 0, JunkY = 0;
	Window JunkRoot;             /* junk window */
	XTextProperty text_prop;
	unsigned int JunkWidth, JunkHeight, JunkBW, JunkDepth;
	char desk[8], *str, action[24];
	size_t str_size;
#ifdef USE_LOCALE
	int num;
	char **list;
#endif
	char *oldWinName, *winname;

	if( ev->xproperty.atom==_XA_WM_DESKTOP && ev->xany.window==Scr.Root ){
		if ((XGetWindowProperty(dpy, Scr.Root, _XA_WM_DESKTOP, 0L, 1L, True,
								_XA_WM_DESKTOP, &actual, &actual_format, &nitems,
								&bytesafter, &prop))==Success){
			if(prop != NULL){
				sprintf( desk, "Desk %ld", *(unsigned long *)prop );
				ChangeDesk( desk );
			}
		}
		return;
	}
	if( XFindContext( dpy, ev->xany.window, MlvwmContext, (caddr_t *)&tmp_win )
	   ==XCNOENT )		return;
	if ( XGetGeometry( dpy, tmp_win->w, &JunkRoot, &JunkX, &JunkY,
					  & JunkWidth, &JunkHeight, &JunkBW, &JunkDepth) == 0 )
		return;
	switch( ev->xproperty.atom ){
	  case XA_WM_NAME:
		oldWinName = WinListName( tmp_win );
		if( XGetWMName( dpy, tmp_win->w, &text_prop) != 0 ){
#ifdef USE_LOCALE
			if(text_prop.value)
				text_prop.nitems = strlen(text_prop.value);
			if(XmbTextPropertyToTextList(dpy, &text_prop, &list, &num) >= Success
			   && num > 0 && *list)
				tmp_win->name = *list;
#else
			tmp_win->name = (char *)text_prop.value;
#endif
		}			
		else
			tmp_win->name = NoName;

		winname = WinListName( tmp_win );
		sprintf( action, "Select %lX", (unsigned long)tmp_win );
		ChangeMenuItemLabel( "ICON", oldWinName, winname,
							action, M_ALLSET, M_AND );
		if( tmp_win==Scr.ActiveWin ){
			str_size = strlen(tmp_win->name)+6;
			str = calloc( str_size, 1 );
			snprintf( str, str_size, "Hide %s", tmp_win->name );
			ChangeMenuItemLabel( "ICON", Scr.IconMenu.m_item->label,
								str, NULL, SELECTON, M_COPY );
			free( str );
		}
		free( oldWinName );
		free( winname );
		SetTitleBar( tmp_win, (Scr.ActiveWin==tmp_win?True:False) );
		break;
	  case XA_WM_ICON_NAME:
		break;
	  case XA_WM_HINTS:
		break;
	  case XA_WM_NORMAL_HINTS:
		GetWindowSizeHints( tmp_win );
		break;
	  default:
		if( ev->xproperty.atom == _XA_WM_PROTOCOLS)
			FetchWmProtocols( tmp_win );
		break;
    }
}

void HandleMapNotify( XEvent *ev )
{
	MlvwmWindow *tmp_win;
	char action[24];

	if( XFindContext( dpy, ev->xany.window, MlvwmContext, (caddr_t *)&tmp_win )
	   ==XCNOENT )
		tmp_win = NULL;
	if (!tmp_win){
		if( (ev->xmap.override_redirect == True) &&
		   (ev->xmap.window != Scr.NoFocusWin))
          XSelectInput( dpy, ev->xmap.window,FocusChangeMask);
		return;
    }

	if( ev->xmap.event != ev->xmap.window)
		return;
   
	XGrabServer (dpy);

	XMapSubwindows(dpy, tmp_win->frame);

	if( !(tmp_win->flags & MAPPED)){
		if( !(tmp_win->flags & NOWINLIST) &&
		   !(tmp_win->flags & TRANSIENT) ){
			sprintf( action, "Select %lX", (unsigned long)tmp_win );
			AddMenuItem( &(Scr.IconMenu), WinListName( tmp_win ), action,
						NULL, tmp_win->miniicon, NULL,
						SELECTON );
		}
		if( tmp_win->flags&STARTICONIC ){
			if( Scr.flags&ICONIFYSHADE && !(tmp_win->flags&SHADE ))
				ShadeWindow( tmp_win );
			if( Scr.flags&ICONIFYHIDE && !(tmp_win->flags&HIDED ))
				HideWindow( tmp_win );
			tmp_win->flags &= ~STARTICONIC;
		}
	}
	if(tmp_win->Desk == Scr.currentdesk && !(tmp_win->flags&HIDED)){

		if( Scr.flags&STARTING )
		   XLowerWindow( dpy, tmp_win->frame );
		else
		   RaiseMlvwmWindow( tmp_win );
		XMapWindow(dpy, tmp_win->frame);
		if( !(Scr.flags & FOLLOWTOMOUSE) && 
			!(Scr.flags&SLOPPYFOCUS) &&
			(!tmp_win->wmhints || tmp_win->wmhints->input!=False) &&
		   !(tmp_win->flags&NOFOCUS) )
			SetFocus( tmp_win );
	}

	tmp_win->flags |= MAPPED;

	XSync( dpy, 0 );
	XUngrabServer (dpy);
	XFlush (dpy);
/*
	KeepOnTop();
*/
}


void HandleMapRequest( Window win )
{
	MlvwmWindow *tmp_win;

	if(XFindContext(dpy, win, MlvwmContext,
					(caddr_t *)&tmp_win)==XCNOENT)
		tmp_win = NULL;

	XFlush(dpy);

	/* If the window has never been mapped before ... */
	if(!tmp_win){
		/* Add decorations. */
		tmp_win = AddWindow( win );
		if( !tmp_win )	return;
    }

	if (!(tmp_win->flags & ICON)){
		int state;
		if( (!(Scr.flags&RSTPREVSTATE) ||
		   (state = GetMapStateProp( tmp_win ))==WithdrawnState) &&
		   tmp_win->wmhints && (tmp_win->wmhints->flags & StateHint) )
			state = tmp_win->wmhints->initial_state;
		else
			state = NormalState;

		XGrabServer( dpy );
		XMapWindow(dpy, tmp_win->w);
		switch (state){
		  case DontCareState:
		  case NormalState:
		  case InactiveState:
			SetMapStateProp(tmp_win, NormalState);
			break;
		  case IconicState:
			tmp_win->flags |= STARTICONIC;
			break;
        }
		XSync( dpy, 0 );
		XUngrabServer( dpy );
    }
	KeepOnTop();
}

void HandleKeyPress( XEvent *ev )
{
	MlvwmWindow *tmp_win;
	ShortCut *key;
	unsigned int mods_used = (ShiftMask | ControlMask | Mod1Mask |
							  Mod2Mask| Mod3Mask| Mod4Mask| Mod5Mask);
	unsigned int modifier;

	if(XFindContext( dpy, ev->xany.window, MlvwmContext, (caddr_t *)&tmp_win )
	   ==XCNOENT)
		tmp_win=NULL;

	modifier = (ev->xkey.state & mods_used);
	for (key = Scr.ShortCutRoot; key != NULL; key = key->next){
		ev->xkey.keycode =
			XKeysymToKeycode(dpy, XKeycodeToKeysym(dpy,ev->xkey.keycode,0));
		if ((key->keycode == ev->xkey.keycode) &&
			((key->mods == modifier)||(key->mods == AnyModifier))){
			ExecuteFunction( key->action );
			return;
        }
    }
	if( tmp_win ){
		if( ev->xkey.window != tmp_win->w){
			ev->xkey.window = tmp_win->w;
			XSendEvent(dpy, tmp_win->w, False, KeyPressMask, ev );
        }
    }
}

void HandleClientMessage( XEvent *ev )
{
	MlvwmWindow *tmp_win;
	XWindowAttributes winattrs;
	unsigned long eventMask;
	MlvwmWindow *NextActive;
	char action[24];

	if( XFindContext( dpy, ev->xany.window, MlvwmContext, (caddr_t *)&tmp_win )
	   == XCNOENT ) return;

	if( ev->xclient.message_type==_XA_WM_CHANGE_STATE ){
		XGetWindowAttributes(dpy, tmp_win->w, &winattrs);
		eventMask = winattrs.your_event_mask;

		switch( ev->xclient.data.l[0] ){
		  case IconicState:
			XSelectInput(dpy, tmp_win->w, eventMask & ~StructureNotifyMask);

			if( Scr.flags&ICONIFYSHADE && !(tmp_win->flags&SHADE )){
				ShadeWindow( tmp_win );
				XUnmapWindow( dpy, tmp_win->w );
			}
			if( Scr.flags&ICONIFYHIDE && !(tmp_win->flags&HIDED )){
				NextActive = NextActiveWin( tmp_win );
				XUnmapWindow( dpy, tmp_win->w );
				HideWindow( tmp_win );
				if( !(Scr.flags & FOLLOWTOMOUSE) ){
					SetFocus( NextActive );
					if( NextActive )
						RaiseMlvwmWindow( NextActive );
				}
				ChangeMenuItemLabel( "ICON", "Show All", "Show All",
									NULL, SELECTON, M_COPY );
			}
			XSelectInput(dpy, tmp_win->w, eventMask);

			break;
		  case NormalState:
			if( Scr.flags&ICONIFYSHADE && !(tmp_win->flags&SHADE ))
				UnShadeWindow( tmp_win );
			if( Scr.flags&ICONIFYHIDE && !(tmp_win->flags&HIDED )){
				sprintf( action, "Select %lX", (unsigned long)tmp_win );
				SelectWindow( action );
			}
			break;
		}
		return;
	}
	if( ev->xclient.window != tmp_win->w){
		ev->xclient.window = tmp_win->w;
		XSendEvent(dpy, tmp_win->w, False, NoEventMask, ev );
	}
}

void HandleEvents( XEvent ev )
{
	strcpy( Scr.ErrorFunc, "HandleEvents" );
	switch( ev.type ){
	  case Expose:
		strcpy( Scr.ErrorFunc, "Expose" );
		handle_expose( &ev );
		strcpy( Scr.ErrorFunc, "Expose End" );
		break;
	  case DestroyNotify:
		strcpy( Scr.ErrorFunc, "DestroyNotify" );
		HandleDestroy( &ev );
		strcpy( Scr.ErrorFunc, "DestroyNotify End" );
		break;
	  case MapRequest:
		strcpy( Scr.ErrorFunc, "MapRequest" );
		HandleMapRequest( ev.xmaprequest.window );
		strcpy( Scr.ErrorFunc, "MapRequest End" );
		break;
	  case MapNotify:
		strcpy( Scr.ErrorFunc, "MapNotify" );
		HandleMapNotify( &ev );
		strcpy( Scr.ErrorFunc, "MapNotify End" );
		break;
	  case UnmapNotify:
		strcpy( Scr.ErrorFunc, "UnmapNotify" );
		handle_unmap_request( &ev );
		strcpy( Scr.ErrorFunc, "UnmapNotify End" );
		break;
	  case ButtonPress:
		strcpy( Scr.ErrorFunc, "ButtonPress" );
		handle_button_press( &ev );
		strcpy( Scr.ErrorFunc, "ButtonPress End" );
		break;
	  case EnterNotify:
		strcpy( Scr.ErrorFunc, "EnterNotify" );
		HandleEnterNotify( &ev );
		strcpy( Scr.ErrorFunc, "EnterNotify End" );
		break;
	  case LeaveNotify:
		strcpy( Scr.ErrorFunc, "LeaveNotify" );
		HandleLeaveNotify( &ev );
		strcpy( Scr.ErrorFunc, "LeaveNotify End" );
		break;
	  case FocusIn:
		strcpy( Scr.ErrorFunc, "FocusIn" );
		break;
	  case ConfigureRequest:
		strcpy( Scr.ErrorFunc, "ConfigureRequest" );
		handle_configure_request( &ev );
		strcpy( Scr.ErrorFunc, "ConfigureRequest End" );
		break;
	  case ClientMessage:
		strcpy( Scr.ErrorFunc, "ClientMessage" );
		HandleClientMessage( &ev );
		break;
      case PropertyNotify:
		strcpy( Scr.ErrorFunc, "PropertyNotify" );
		handle_property_request( &ev );
		strcpy( Scr.ErrorFunc, "PropertyNotify End" );
		break;
      case KeyPress:
		strcpy( Scr.ErrorFunc, "KeyPress" );
        HandleKeyPress( &ev );
		strcpy( Scr.ErrorFunc, "KeyPress End" );
        break;
 	  case ColormapNotify:
		strcpy( Scr.ErrorFunc, "ColormapNotify" );
		break;
	  default:
		strcpy( Scr.ErrorFunc, "default" );
        if( ev.type == (ShapeEventBase + ShapeNotify)){
			strcpy( Scr.ErrorFunc, "Shape" );
			HandleShapeNotify( &ev );
			strcpy( Scr.ErrorFunc, "Shape End" );
		}
		strcpy( Scr.ErrorFunc, "default End" );
		break;
	}
	strcpy( Scr.ErrorFunc, "Leave Handle Events" );
}

void WaitEvents( void )
{
	extern int xfd;

	XEvent ev;
	struct timeval value;
	fd_set in_fdset;

	value.tv_usec = 0;
	value.tv_sec = 1;

	FD_ZERO( &in_fdset );
	FD_SET( xfd, &in_fdset );

	strcpy( Scr.ErrorFunc, "WaitEvents" );
	XFlush( dpy );
	if( XPending( dpy ) ){
		XNextEvent( dpy, &ev );
		HandleEvents( ev );

		return;
	}
	XFlush( dpy );
	ReapChildren();
	strcpy( Scr.ErrorFunc, "WaitEvents End" );
	if( Scr.flags & DEBUGOUT )	fprintf( stderr, "Return WaitEvents\n" );

#ifdef __hpux
    if( select( xfd+1, (int *)&in_fdset, 0, 0, &value ) == 0){
#else
	if( select( xfd+1, &in_fdset, 0, 0, &value ) == 0){
#endif
		if( Scr.flags&BALLOONON )		BalloonHelp();
	}
}
