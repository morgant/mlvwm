/****************************************************************************/
/* This module is based on fvwm, but has been siginificantly modified       */
/* by TakaC Hasegawa (tak@bioele.nuee.nagoya-u.ac.jp)                       */
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

#include "mlvwm.h"
#include "screen.h"
#include "menus.h"
#include "borders.h"
#include "misc.h"

#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <X11/extensions/shape.h>

char NoName[] = "Untitled";

extern int matchWildcards( char *, char * );

/* Used to parse command line of clients for specific desk requests. */
/* Todo: check for multiple desks. */
static XrmDatabase db;
static XrmOptionDescRec table [] = {
  /* Want to accept "-workspace N" or -xrm "afterstep*desk:N" as options
   * to specify the desktop. I have to include dummy options that
   * are meaningless since Xrm seems to allow -w to match -workspace
   * if there would be no ambiguity. */
    {"-workspace",      "*desk",        XrmoptionSepArg, (caddr_t) NULL},
    {"-xrm",            NULL,           XrmoptionResArg, (caddr_t) NULL},
};

styles *lookupstyles( char *name, XClassHint *class )
{
	styles *nptr, *style=NULL;

	/* look for the name first */
	for( nptr = Scr.style_list; nptr != NULL; nptr = nptr->next){
		if( matchWildcards( nptr->name, name ) )
			style = nptr;
		if( class ){
			if( matchWildcards( nptr->name, class->res_name ) )
				style = nptr;
			if( matchWildcards( nptr->name, class->res_class ) )
				style = nptr;
		}
	}
	if( style && style->iconname && !(style->miniicon) )
	   style->miniicon = ReadIcon( style->iconname, NULL, False );
	return style;
}

void create_resizebox( MlvwmWindow *tmp_win )
{
	unsigned long valuemask;
	XSetWindowAttributes attributes;

	valuemask = CWCursor | CWEventMask | CWBackPixel;
	attributes.event_mask = ButtonPressMask | ButtonReleaseMask | ExposureMask;
	attributes.background_pixel = WhitePixel( dpy, Scr.screen );
	attributes.cursor = Scr.MlvwmCursors[RESIZE];

	tmp_win->resize_b = XCreateWindow( dpy, tmp_win->frame,
									  tmp_win->frame_w-2-SBAR_WH,
									  tmp_win->frame_h-2-SBAR_WH,
									  SBAR_WH+(Scr.flags&SYSTEM8?1:0),
									  SBAR_WH+(Scr.flags&SYSTEM8?1:0),
									  Scr.flags&SYSTEM8?0:1,
									  CopyFromParent, InputOutput,
									  CopyFromParent,
									  valuemask, &attributes );
	XSaveContext( dpy, tmp_win->resize_b, MlvwmContext, (caddr_t)tmp_win );
}

void create_scrollbar( MlvwmWindow *tmp_win )
{
	unsigned long valuemask;
	XSetWindowAttributes attributes;
	int title_height;
	int lp;

	title_height = tmp_win->flags & TITLE ? TITLE_HEIGHT : -1;
	valuemask = CWCursor | CWEventMask | CWBackPixel;
	attributes.event_mask = ButtonPressMask | ButtonReleaseMask | ExposureMask 
		| EnterWindowMask | LeaveWindowMask;
	attributes.background_pixel = WhitePixel( dpy, Scr.screen );
	if( tmp_win->flags&SBARV ){
		attributes.cursor = Scr.MlvwmCursors[SBARV_CURSOR];
		tmp_win->scroll_v[0] = XCreateWindow( dpy, tmp_win->frame,
											 tmp_win->frame_w-2-SBAR_WH,
											 title_height,
											 SBAR_WH, tmp_win->attr.height, 1,
											 CopyFromParent, InputOutput,
											 CopyFromParent,
											 valuemask, &attributes );
		tmp_win->scroll_v[1] = XCreateWindow( dpy, tmp_win->scroll_v[0],
											 0, 0,
											 SBAR_WH, SBAR_WH, 0,
											 CopyFromParent, InputOutput,
											 CopyFromParent,
											 valuemask, &attributes );
		tmp_win->scroll_v[2] = XCreateWindow( dpy, tmp_win->scroll_v[0],
											 0, 0,
											 SBAR_WH, SBAR_WH, 0,
											 CopyFromParent, InputOutput,
											 CopyFromParent,
											 valuemask, &attributes );
		tmp_win->scroll_v[3] = XCreateWindow( dpy, tmp_win->scroll_v[0],
											 0, 0,
											 SBAR_WH, SBAR_WH, 0,
											 CopyFromParent, InputOutput,
											 CopyFromParent,
											 valuemask, &attributes );
		XMapSubwindows( dpy, tmp_win->scroll_v[0] );
		if( !(tmp_win->flags&SCROLL) )	XUnmapWindow( dpy, tmp_win->scroll_v[3] );
		for( lp=0; lp<4; lp++ )
			XSaveContext( dpy, tmp_win->scroll_v[lp], MlvwmContext, (caddr_t)tmp_win );
	}
	if( tmp_win->flags&SBARH ){
		attributes.cursor = Scr.MlvwmCursors[SBARH_CURSOR];
		tmp_win->scroll_h[0] = XCreateWindow( dpy, tmp_win->frame,
											 -1, tmp_win->frame_h-2-SBAR_WH,
											 tmp_win->attr.width, SBAR_WH, 1,
											 CopyFromParent, InputOutput,
											 CopyFromParent,
											 valuemask, &attributes );
		tmp_win->scroll_h[1] = XCreateWindow( dpy, tmp_win->scroll_h[0],
											 0, 0,
											 SBAR_WH, SBAR_WH, 0,
											 CopyFromParent, InputOutput,
											 CopyFromParent,
											 valuemask, &attributes );
		tmp_win->scroll_h[2] = XCreateWindow( dpy, tmp_win->scroll_h[0],
											 0, 0,
											 SBAR_WH, SBAR_WH, 0,
											 CopyFromParent, InputOutput,
											 CopyFromParent,
											 valuemask, &attributes );
		tmp_win->scroll_h[3] = XCreateWindow( dpy, tmp_win->scroll_h[0],
											 0, 0,
											 SBAR_WH, SBAR_WH, 0,
											 CopyFromParent, InputOutput,
											 CopyFromParent,
											 valuemask, &attributes );
		XMapSubwindows( dpy, tmp_win->scroll_h[0] );
		if( !(tmp_win->flags&SCROLL) )	XUnmapWindow( dpy, tmp_win->scroll_h[3] );
		for( lp=0; lp<4; lp++ )
			XSaveContext(dpy,tmp_win->scroll_h[lp],MlvwmContext,(caddr_t)tmp_win);
	}
}

void create_titlebar( MlvwmWindow *tmp_win )
{
	unsigned long valuemask;
	XSetWindowAttributes attributes;

	valuemask = CWCursor | CWEventMask | CWBackPixel;
	attributes.event_mask = ButtonPressMask | ButtonReleaseMask | ExposureMask;
	attributes.background_pixel = WhitePixel( dpy, Scr.screen );
	attributes.cursor = Scr.MlvwmCursors[TITLE_CURSOR];
	tmp_win->title_w = XCreateWindow( dpy, tmp_win->frame,
									 Scr.flags&SYSTEM8?2:0, 0,
									 tmp_win->attr.width+SBAR_WH+1,
									 TITLE_HEIGHT, 0,
									 CopyFromParent, InputOutput,
									 CopyFromParent,
									 valuemask, &attributes );
	attributes.event_mask = ( ButtonPressMask|ButtonReleaseMask|ExposureMask
							 | EnterWindowMask | LeaveWindowMask );

	if( tmp_win->flags&CLOSER ){
		attributes.cursor = Scr.MlvwmCursors[DESTROY];
		tmp_win->close_b = XCreateWindow( dpy, tmp_win->title_w,
										 Scr.flags&SYSTEM8?4:BOXSIZE+1,
										 (TITLE_HEIGHT-BOXSIZE)/2,
										 BOXSIZE, BOXSIZE, 0,
										 CopyFromParent, InputOutput,
										 CopyFromParent,
										 valuemask, &attributes );
		XSaveContext( dpy, tmp_win->close_b, MlvwmContext, (caddr_t)tmp_win );
	}
	if( tmp_win->flags&MINMAXR ){
		attributes.cursor = Scr.MlvwmCursors[SELECT];
		tmp_win->minmax_b = XCreateWindow( dpy, tmp_win->title_w,
										  tmp_win->frame_w-BOXSIZE*2-1,
										  (TITLE_HEIGHT-BOXSIZE)/2,
										  BOXSIZE, BOXSIZE, 0,
										  CopyFromParent, InputOutput,
										  CopyFromParent,
										  valuemask, &attributes );
		XSaveContext( dpy, tmp_win->minmax_b, MlvwmContext, (caddr_t)tmp_win );
	}
	if( tmp_win->flags&SHADER ){
		attributes.cursor = Scr.MlvwmCursors[SELECT];
		tmp_win->shade_b = XCreateWindow( dpy, tmp_win->title_w,
										 tmp_win->frame_w-BOXSIZE*2-1,
										 (TITLE_HEIGHT-BOXSIZE)/2,
										 BOXSIZE, BOXSIZE, 0,
										 CopyFromParent, InputOutput,
										 CopyFromParent,
										 valuemask, &attributes );
		XSaveContext( dpy, tmp_win->shade_b, MlvwmContext, (caddr_t)tmp_win );
	}
 	XMapSubwindows( dpy, tmp_win->title_w );
	XSaveContext( dpy, tmp_win->title_w, MlvwmContext, (caddr_t)tmp_win );
}

void FetchWmProtocols (MlvwmWindow *tmp)
{
	unsigned long flags = 0L;
	Atom *protocols = NULL, *ap;
	int i, n;
	Atom atype;
	int aformat;
	unsigned long bytes_remain,nitems;

	if(tmp == NULL) return;
	/* First, try the Xlib function to read the protocols.
	 * This is what Twm uses. */
	if (XGetWMProtocols (dpy, tmp->w, &protocols, &n)){
		for (i = 0, ap = protocols; i < n; i++, ap++){
			if (*ap == (Atom)_XA_WM_TAKE_FOCUS) flags |= DoesWmTakeFocus;
			if (*ap == (Atom)_XA_WM_DELETE_WINDOW) flags |= DoesWmDeleteWindow;
		}
		if (protocols) XFree ((char *) protocols);
	}
	else{
		/* Next, read it the hard way. mosaic from Coreldraw needs to 
		 * be read in this way. */
		if ((XGetWindowProperty(dpy, tmp->w, _XA_WM_PROTOCOLS, 0L, 10L, False,
								_XA_WM_PROTOCOLS, &atype, &aformat, &nitems,
								&bytes_remain,
								(unsigned char **)&protocols))==Success){
			for (i = 0, ap = protocols; i < nitems; i++, ap++){
				if (*ap == (Atom)_XA_WM_TAKE_FOCUS) flags |= DoesWmTakeFocus;
				if (*ap == (Atom)_XA_WM_DELETE_WINDOW)
					flags |= DoesWmDeleteWindow;
			}
			if (protocols) XFree ((char *) protocols);
		}
	}
	if( XGetWindowProperty(dpy, tmp->w, _XA_WM_STATE, 0L, 3L, False,
						   _XA_WM_STATE, &atype, &aformat, &nitems,
						   &bytes_remain,
						   (unsigned char **)&protocols)==Success &&
	   protocols != NULL && *protocols==IconicState)
		tmp->flags |= STARTICONIC;

	tmp->flags |= flags;
	return;
}

void GrabKeys( MlvwmWindow *tmp_win )
{
	ShortCut *tmp;

	for (tmp = Scr.ShortCutRoot; tmp != NULL; tmp = tmp->next){
        XGrabKey(dpy, tmp->keycode, tmp->mods, tmp_win->frame, True,
                 GrabModeAsync, GrabModeAsync);
    }
	return;
}

void GetWindowSizeHints( MlvwmWindow *mw )
{
    long supplied=0;

    if( !XGetWMNormalHints( dpy, mw->w,&(mw->hints), &supplied ) )
	mw->hints.flags = 0;
    if (mw->hints.flags & PResizeInc){
	if (mw->hints.width_inc == 0)
	    mw->hints.width_inc = 1;
	if (mw->hints.height_inc == 0)
	    mw->hints.height_inc = 1;
    }
    else{
		mw->hints.width_inc = 1;
		mw->hints.height_inc = 1;
    }

    if(!(mw->hints.flags & PBaseSize)){
	    mw->hints.base_width = 0;
	    mw->hints.base_height = 0;
    }
    if(!(mw->hints.flags & PMinSize)){
		mw->hints.min_width = mw->hints.base_width;
		mw->hints.min_height = mw->hints.base_height;            
    }
    if(!(mw->hints.flags & PMaxSize)){
		mw->hints.max_width = MAX_WINDOW_WIDTH;
		mw->hints.max_height = MAX_WINDOW_HEIGHT;
    }
    if(mw->hints.max_width < mw->hints.min_width)
		mw->hints.max_width = MAX_WINDOW_WIDTH;    
    if(mw->hints.max_height < mw->hints.min_height)
		mw->hints.max_height = MAX_WINDOW_HEIGHT;    

    if(mw->hints.min_height <= 0)
		mw->hints.min_height = 1;
    if(mw->hints.min_width <= 0)
		mw->hints.min_width = 1;
}

void SmartPlacement( MlvwmWindow *t, int width, int height, int *x, int *y )
{
	int temp_h,temp_w;
	int test_x = 0,test_y = 0;
	int loc_ok = False, tw,tx,ty,th;
	MlvwmWindow *test_window;

	temp_h = height;
	temp_w = width;
      
	while( test_y+temp_h<Scr.MyDisplayHeight && !loc_ok ){
		test_x = 0;
		while( test_x+temp_w<Scr.MyDisplayWidth && !loc_ok ){
			loc_ok = True;
			test_window = Scr.MlvwmRoot.next;
			while( test_window && loc_ok==True ){   
				if( ((test_window->Desk==Scr.currentdesk &&
					  !(Scr.flags&STARTING))||
					 test_window->Desk==t->Desk) &&
					  !(test_window->flags&HIDED) &&
				   !(test_window->flags&TRANSIENT) ){
					if( test_window!=t ){
						tw=test_window->frame_w+2;
						th=test_window->frame_h+2;
						tx = test_window->frame_x;
						ty = test_window->frame_y;
						if( tx<=test_x+width && tx+tw>=test_x &&
						   ty<=test_y+height && ty+th>=test_y ){
							loc_ok = False;
							test_x = tx + tw;
						}
                    }
                }
				test_window = test_window->next;
            }
			test_x +=1;
        }
		test_y +=1;
    }
	if( loc_ok==False ){
		if( Scr.ActiveWin ){
			*x = Scr.ActiveWin->frame_x+TITLE_HEIGHT;
			*y = Scr.ActiveWin->frame_y+TITLE_HEIGHT;
		}
    }
	else{
		*x = test_x;
		*y = test_y;
	}
	return;
}


MlvwmWindow *AddWindow( Window win )
{
	MlvwmWindow *tmp_win, *last;
	unsigned long valuemask;
	XSetWindowAttributes attributes;
	int title_height;
	XTextProperty text_prop;
	Atom atype;
	int aformat;
	int diff_x=0, diff_y=0;
	styles *tmp_style;
#ifdef USE_LOCALE
	int num;
	char **list;
#endif
	unsigned long nitems, bytes_remain;
	unsigned char *prop;
  
	XrmValue rm_value;
	int client_argc;
	char **client_argv = NULL, *str_type;
	Bool status;

	tmp_win = calloc( 1, sizeof( MlvwmWindow ) );
	
	tmp_win->flags = NORMALWIN;
	if( Scr.flags&SYSTEM8 )		tmp_win->flags |= SHADER;
	tmp_win->size_w = Scr.MyDisplayWidth*90./100.;
	tmp_win->size_h = Scr.MyDisplayHeight*90./100.;

	tmp_win->miniicon = NULL;
	
	if( Scr.ActiveWin )
		last = Scr.ActiveWin;
	else
		for ( last = (MlvwmWindow *)&Scr.MlvwmRoot;
			 last->next != NULL;
			 last = last->next);
	if( last->next ){
		tmp_win->next = last->next;
		last->next->prev = tmp_win;
	}
	last->next = (struct MlvwmWindow *)tmp_win;
	tmp_win->prev = last;
	if( tmp_win==(MlvwmWindow *)0 )	return NULL;
	tmp_win->flags = 0;
	tmp_win->w = win;

	XGetWindowAttributes( dpy, tmp_win->w, &tmp_win->attr );
	if( XGetWMName( dpy, tmp_win->w, &text_prop) != 0 ){
#ifdef USE_LOCALE
		if(text_prop.value)
			text_prop.nitems = strlen(text_prop.value);
		if(XmbTextPropertyToTextList(dpy, &text_prop, &list, &num) >= Success
		   && num > 0 && *list)
			tmp_win->name = *list;
		else
			tmp_win->name = NoName;
#else
		tmp_win->name = (char *)text_prop.value;
#endif
	}
	else
		tmp_win->name = NoName;
	tmp_win->class = NoClass;
	XGetClassHint(dpy, tmp_win->w, &tmp_win->class);
	if (tmp_win->class.res_name == NULL)
		tmp_win->class.res_name = NoName;
	if (tmp_win->class.res_class == NULL)
		tmp_win->class.res_class = NoName;
	
	tmp_win->wmhints = XGetWMHints( dpy, tmp_win->w );
	tmp_win->old_bw = tmp_win->attr.border_width;
	
	{
		int xws, yws, xbs, ybs;
		unsigned wws, hws, wbs, hbs;
		int boundingShaped, clipShaped;
		
		XShapeSelectInput (dpy, tmp_win->w, ShapeNotifyMask);
		XShapeQueryExtents (dpy, tmp_win->w,
							&boundingShaped, &xws, &yws, &wws, &hws,
							&clipShaped, &xbs, &ybs, &wbs, &hbs);
		tmp_win->wShaped = boundingShaped;
	}
	
	GetWindowSizeHints( tmp_win );
	if( (tmp_style = lookupstyles( tmp_win->name, &tmp_win->class ))!=NULL ){
		tmp_win->flags = tmp_style->flags;
		if( tmp_style->maxmizescale ){
			tmp_win->size_w =
				Scr.MyDisplayWidth*(double)tmp_style->maxmizescale/100.;
			tmp_win->size_h = (Scr.MyDisplayHeight-MENUB_H)*
				(double)tmp_style->maxmizescale/100.;
		}
		else{
			tmp_win->size_w =
				tmp_style->maxmizesize_w*tmp_win->hints.width_inc+
					tmp_win->hints.base_width;
			tmp_win->size_h =
				tmp_style->maxmizesize_h*tmp_win->hints.height_inc+
					tmp_win->hints.base_height;
			tmp_win->size_w += SBAR_WH+1+2;
			if( !(tmp_win->flags&SBARV) )		tmp_win->size_w-=(SBAR_WH+1);
			tmp_win->size_h += SBAR_WH+TITLE_HEIGHT+2+2;
			if( !(tmp_win->flags&SBARH) )		tmp_win->size_h-=(SBAR_WH+1);
			if( Scr.flags&SYSTEM8 ){
				tmp_win->size_w += 12;
				tmp_win->size_h += 6;
			}
		}
		tmp_win->miniicon = tmp_style->miniicon;
		if( !tmp_win->miniicon )
			if( (tmp_style = lookupstyles( "*", NULL ))!=NULL &&
			   tmp_style->miniicon )
				tmp_win->miniicon = tmp_style->miniicon;
	}
	if(XGetTransientForHint(dpy, tmp_win->w,  &tmp_win->transientfor)){
		tmp_win->flags &= ~NORMALWIN;
		if( Scr.flags&SYSTEM8 )		tmp_win->flags &= ~SHADER;
		tmp_win->flags |= TRANSIENT;
		if( !(tmp_win->flags&NONTRANSIENTDECORATE) )
			tmp_win->flags |= TITLE;
	}
	
	title_height = tmp_win->flags & TITLE ? TITLE_HEIGHT : -1;
  
	tmp_win->Desk = Scr.currentdesk;
	/* Find out if the client requested a specific desk on the command line. */
	if ( XGetCommand( dpy, tmp_win->w, &client_argv, &client_argc ) ) {
		XrmParseCommand( &db, table, 2, "mlvwm", &client_argc, client_argv );
		status = XrmGetResource( db, "mlvwm.desk", "Mlvwm.Desk",
								&str_type, &rm_value );
		if ((status == True) && (rm_value.size != 0))
			tmp_win->Desk = atoi(rm_value.addr);
		XrmDestroyDatabase (db);
		db = NULL;
	}
	if ((XGetWindowProperty(dpy, tmp_win->w, _XA_WM_DESKTOP, 0L, 1L, True,
							_XA_WM_DESKTOP, &atype, &aformat, &nitems,
							&bytes_remain, &prop))==Success){
		if(prop != NULL){
			tmp_win->Desk = *(unsigned long *)prop;
			XFree(prop);
		}
	}
	if( tmp_win->Desk<0 || tmp_win->Desk>=Scr.n_desktop )
		tmp_win->Desk = 0;
	
	if( tmp_win->flags&(TITLE|SBARV|SBARH|RESIZER) ){
		tmp_win->frame_w = tmp_win->attr.width+SBAR_WH+1+2;
		diff_x = 1;
		diff_y = title_height+1;
		if( !(tmp_win->flags&SBARV) )		tmp_win->frame_w-=(SBAR_WH+1);
		tmp_win->frame_h = tmp_win->attr.height+SBAR_WH+title_height+2+2;
		if( !(tmp_win->flags&SBARH) )		tmp_win->frame_h-=(SBAR_WH+1);
		if( Scr.flags&SYSTEM8 ){
			diff_x += 6;
			tmp_win->frame_w += 12;
			tmp_win->frame_h += 6;
		}
	}
	else{
		diff_x = (5+tmp_win->old_bw);
		diff_y = (5+tmp_win->old_bw);
		tmp_win->frame_w = tmp_win->attr.width+2+5*2;
		tmp_win->frame_h = tmp_win->attr.height+2+5*2;
	}

	if( !(tmp_win->flags & TRANSIENT) && 
	   !(tmp_win->hints.flags & USPosition) &&
	   !(tmp_win->hints.flags & PPosition) )
		SmartPlacement( tmp_win, tmp_win->frame_w+2, tmp_win->frame_h+2,
					   &(tmp_win->attr.x), &(tmp_win->attr.y) );

	tmp_win->diff_x = tmp_win->attr.x;
	tmp_win->diff_y = tmp_win->attr.y;

	tmp_win->frame_x = tmp_win->attr.x + tmp_win->old_bw-1;
	tmp_win->frame_y = tmp_win->attr.y + tmp_win->old_bw-1;

	if( !(tmp_win->flags&TRANSIENT) ){
		tmp_win->frame_x =
			tmp_win->frame_x+tmp_win->frame_w < Scr.MyDisplayWidth ?
				tmp_win->frame_x : Scr.MyDisplayWidth-tmp_win->frame_w-2;
		tmp_win->frame_x = tmp_win->frame_x<0 ? 0 : tmp_win->frame_x;
  
		tmp_win->frame_y =
			tmp_win->attr.y+tmp_win->frame_h < Scr.MyDisplayHeight ?
				tmp_win->frame_y : Scr.MyDisplayHeight-tmp_win->frame_h-2;
	}
	else{
		tmp_win->frame_x -= diff_x;
		tmp_win->frame_y -= diff_y;
	}

	if( tmp_win->frame_y<MENUB_H ) tmp_win->frame_y = MENUB_H;

	valuemask = CWCursor | CWEventMask | CWBackPixel;
	
	attributes.cursor = Scr.MlvwmCursors[TITLE_CURSOR];
	attributes.background_pixel = WhitePixel( dpy, Scr.screen );
	attributes.event_mask = (SubstructureRedirectMask | ButtonPressMask |
							 ButtonReleaseMask |EnterWindowMask |
							 ExposureMask | LeaveWindowMask );
	tmp_win->frame = XCreateWindow( dpy, Scr.Root,
								   tmp_win->frame_x, tmp_win->frame_y,
								   tmp_win->frame_w, tmp_win->frame_h, 1,
								   CopyFromParent, InputOutput, CopyFromParent,
								   valuemask, &attributes );
	attributes.cursor = Scr.MlvwmCursors[DEFAULT];
	attributes.background_pixel = WhitePixel( dpy, Scr.screen );
	tmp_win->Parent = XCreateWindow( dpy, tmp_win->frame,
									-1, title_height,
									tmp_win->attr.width, tmp_win->attr.height,
									1, CopyFromParent,
									InputOutput,CopyFromParent,
									valuemask,&attributes);  
	if( tmp_win->flags&TITLE )		create_titlebar( tmp_win );
	if( tmp_win->flags&RESIZER )		create_resizebox( tmp_win );
	if( tmp_win->flags&SBARH || tmp_win->flags&SBARV )
		create_scrollbar( tmp_win );
	if( !(tmp_win->flags&( TITLE | SBARV | SBARH | RESIZER )) )
		XMoveWindow( dpy, tmp_win->Parent, 5, 5 );
	XSetWindowBorderWidth( dpy, tmp_win->w, 0 );
	
	XSaveContext( dpy, tmp_win->frame, MlvwmContext, (caddr_t)tmp_win );
	XSaveContext( dpy, tmp_win->Parent, MlvwmContext, (caddr_t)tmp_win );
	XSaveContext( dpy, tmp_win->w, MlvwmContext, (caddr_t)tmp_win );
	XAddToSaveSet( dpy, win );
	
	XUnmapWindow( dpy, tmp_win->w );
	XRaiseWindow( dpy, tmp_win->Parent );
	if( tmp_win->flags&RESIZER )
		XRaiseWindow( dpy, tmp_win->resize_b );
	XReparentWindow( dpy, tmp_win->w, tmp_win->Parent, 0, 0 );
	
	valuemask = (CWEventMask | CWDontPropagate );
	attributes.event_mask = (StructureNotifyMask | PropertyChangeMask | 
							 VisibilityChangeMask |  EnterWindowMask | 
							 LeaveWindowMask | 
							 ColormapChangeMask | FocusChangeMask);
	attributes.do_not_propagate_mask = ( ButtonPressMask | ButtonReleaseMask );
	XChangeWindowAttributes( dpy, tmp_win->w, valuemask, &attributes );
	SetUpFrame( tmp_win, tmp_win->frame_x, tmp_win->frame_y, 
			   tmp_win->frame_w, tmp_win->frame_h, True );
	XMapSubwindows( dpy, tmp_win->Parent );
	XMapWindow( dpy, tmp_win->Parent );
	
	{
		Window JunkRoot, JunkChild;
		int JunkX, JunkY;
		unsigned int JunkWidth, JunkHeight, JunkBW, JunkDepth;
		int a, b;
		
		XGetGeometry(dpy, tmp_win->w, &JunkRoot, &JunkX, &JunkY,
					 &JunkWidth, &JunkHeight, &JunkBW, &JunkDepth);
		XTranslateCoordinates(dpy,tmp_win->frame,Scr.Root,JunkX,JunkY,
							  &a,&b,&JunkChild);
		tmp_win->diff_x -= a;
		tmp_win->diff_y -= b;
	}
	
	FetchWmProtocols (tmp_win);
	
	if( !(Scr.flags&FOLLOWTOMOUSE) ){
		XGrabButton(dpy, AnyButton, AnyModifier, tmp_win->frame,True,
					ButtonPressMask, GrabModeSync,GrabModeAsync,None,
					Scr.MlvwmCursors[SYS]);
	}
	
	tmp_win->win_w = tmp_win->frame_w;
	tmp_win->win_h = tmp_win->frame_h;
	
	GrabKeys( tmp_win );
	
	return tmp_win;
}
