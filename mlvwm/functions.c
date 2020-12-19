/****************************************************************************/
/* This module is all original code                                         */
/* by TakaC Hasegawa (tac.hasegawa@gmail.com)                               */
/* Copyright 1996, TakaC Hasegawa                                           */
/*     You may use this code for any purpose, as long as the original       */
/*     copyright remains in the source code and all documentation           */
/****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mlvwm.h"
#include "screen.h"
#include "functions.h"
#include "borders.h"
#include "event.h"
#include "misc.h"
#include "balloon.h"

extern char NoName[];

void IconZoom ( MlvwmWindow *mw, Bool on )
{
	int   x,y,w,h, dx,dy,dw,dh;
	float r,s;
	dx = (Scr.IconMenu.LabelWidth - Scr.IconMenu.xpm->width)/2 - mw->frame_x
		+ Scr.IconMenu.LabelX;
	dy = (MENUB_H - Scr.IconMenu.xpm->height)/2 - mw->frame_y;
	dw = Scr.IconMenu.xpm->width  - mw->frame_w;
	dh = Scr.IconMenu.xpm->height - mw->frame_h;

	XGrabServer( dpy );
	for( r=0.; r<=1.; r=r+.05 ){   
		s = on? 1. - r*r : 1. - (1.-r)*(1.-r);
		x = mw->frame_x + s * dx;
		y = mw->frame_y + s * dy;
		w = mw->frame_w + s * dw;
		h = mw->frame_h + s * dh;
		XDrawRectangle( dpy, Scr.Root, Scr.RobberGC, x, y, w, h );
		sleep_a_little( Scr.zoom_wait );
		XSync( dpy, 0 ); 
		XDrawRectangle( dpy, Scr.Root, Scr.RobberGC, x, y, w, h );
	}
	XSync( dpy, 0 ); 
	XUngrabServer( dpy );
}

void ShadeWindow( MlvwmWindow *mw )
{
	unsigned long valuemask;
	XSetWindowAttributes attr;

	mw->shade_h = mw->frame_h;
	mw->frame_h = TITLE_HEIGHT+2;
	mw->flags |= SHADE;
	if( mw->flags&SBARV )	XUnmapWindow( dpy, mw->scroll_v[0] );
	if( mw->flags&SBARH )   XUnmapWindow( dpy, mw->scroll_h[0] );
	if( mw->flags&RESIZER )	XUnmapWindow( dpy, mw->resize_b );
	XUnmapWindow(dpy, mw->Parent);

	if( mw->flags&SHADER ){
		attr.cursor = Scr.MlvwmCursors[SHADER_DOWN_CURSOR];
		valuemask = CWCursor;
		XChangeWindowAttributes( dpy, mw->shade_b, valuemask, &attr );
	}
	if( Scr.flags&ICONIFYSHADE )
		SetMapStateProp( mw, IconicState );
	SetUpFrame( mw,mw->frame_x,mw->frame_y,mw->frame_w,mw->frame_h, False );
	if( Scr.flags&SYSTEM8 )
		SetTitleBar( mw, True );
}

void UnShadeWindow( MlvwmWindow *mw )
{
	unsigned long valuemask;
	XSetWindowAttributes attr;

	mw->frame_h = mw->shade_h;
	mw->flags &= ~SHADE;
	if( mw->flags&SBARV )	XMapWindow( dpy, mw->scroll_v[0] );
	if( mw->flags&SBARH )	XMapWindow( dpy, mw->scroll_h[0] );
	if( mw->flags&RESIZER )	XMapWindow( dpy, mw->resize_b );
	XMapWindow(dpy, mw->Parent);

	if( Scr.flags&ICONIFYSHADE ){
		XMapWindow( dpy, mw->w );
		SetMapStateProp( mw, NormalState );
	}
	XSetInputFocus( dpy, mw->w, RevertToParent, CurrentTime );

	if( mw->flags&SHADER ){
		attr.cursor = Scr.MlvwmCursors[SHADER_UP_CURSOR];
		valuemask = CWCursor;
		XChangeWindowAttributes( dpy, mw->shade_b, valuemask, &attr );
	}
	SetUpFrame( mw,mw->frame_x,mw->frame_y,mw->frame_w,mw->frame_h, True );
}

void UnmapIt( MlvwmWindow *t )
{
	XWindowAttributes winattrs;
	unsigned long eventMask;
	/*
	 * Prevent the receipt of an UnmapNotify, since that would
	 * cause a transition to the Withdrawn state.
	 */
	XGetWindowAttributes(dpy, t->w, &winattrs);
	eventMask = winattrs.your_event_mask;
	XSelectInput(dpy, t->w, eventMask & ~StructureNotifyMask);
	if(t->flags & MAPPED)
		XUnmapWindow(dpy, t->frame);
	XSelectInput(dpy, t->w, eventMask);
}

void MapIt( MlvwmWindow *t )
{
	if( t->flags&MAPPED && !(t->flags&HIDED) ){
		if( !(t->flags&SHADE) ){
			XMapWindow(dpy, t->Parent);
			XMapWindow(dpy, t->frame);
		}
		else
			XMapWindow(dpy, t->frame);
	}
}

MlvwmWindow *ChoiseWindow( XEvent *ev, int cursor )
{
	Bool loop=True;
	MlvwmWindow *tmp_win;
	Window select_win;

	if( !GrabEvent( cursor ) ){
		XBell( dpy, 30 );
		return NULL;
	}
	while( loop ){
		XMaskEvent( dpy, ButtonPressMask | ButtonMotionMask | 
				   PointerMotionMask | ExposureMask |
				   VisibilityChangeMask, ev );
		if( ev->type==ButtonPress )
			loop = False;
		else
			HandleEvents( *ev );
	}
	UnGrabEvent();

	select_win = ev->xany.window;
	if((( select_win == Scr.Root ) || ( select_win == Scr.NoFocusWin ) )
	   && ( ev->xbutton.subwindow != (Window)0 ) )
		select_win = ev->xbutton.subwindow;
	if(XFindContext(dpy, select_win, MlvwmContext,	(caddr_t *)&tmp_win)==XCNOENT)
		tmp_win = NULL;
	return tmp_win;
}

void MoveWindowFunction( char *action )
{
	MlvwmWindow *tmp_win;
	XEvent ev;

	tmp_win = ChoiseWindow( &ev, MOVE );
	if( !tmp_win ){
		XBell( dpy, 30 );
		return;
	}
	MoveWindow( tmp_win, &ev, False );
}

void ResizeWindowFunction( char *action )
{
	MlvwmWindow *tmp_win;
	XEvent ev;

	tmp_win = ChoiseWindow( &ev, MOVE );
	if( !tmp_win ){
		XBell( dpy, 30 );
		return;
	}
	if( tmp_win->flags&SHADE )	return;
	ResizeWindow( tmp_win, &ev, False );
}

void KillWindowFunction( char *action )
{
	MlvwmWindow *tmp_win;
	XEvent ev;
	int JunkX, JunkY;
	Window JunkRoot;
	unsigned int JunkWidth, JunkHeight, JunkBW, JunkDepth;

	tmp_win = ChoiseWindow( &ev, DESTROY );
	if( !tmp_win ){
		XBell( dpy, 30 );
		return;
	}
	if ( tmp_win->flags & DoesWmDeleteWindow)
		send_clientmessage( tmp_win->w, _XA_WM_DELETE_WINDOW, CurrentTime);
	else if (XGetGeometry(dpy, tmp_win->w, &JunkRoot, &JunkX, &JunkY,
						  &JunkWidth, &JunkHeight, &JunkBW, &JunkDepth) == 0)
		Destroy( tmp_win );
	else
		XKillClient(dpy, tmp_win->w);
}

void ToggleScrollFunction( char *action )
{
	MlvwmWindow *tmp_win;
	XEvent ev;

	tmp_win = ChoiseWindow( &ev, SELECT );
	if( !tmp_win ){
		XBell( dpy, 30 );
		return;
	}
	if( tmp_win->flags & ( SBARV | SBARH ) ){
		if( tmp_win->flags&SCROLL ){
/* Scroll Mode Off */
			tmp_win->flags &= ~SCROLL;
			tmp_win->win_x = 0;
			tmp_win->win_y = 0;
			tmp_win->frame_w = tmp_win->win_w;
			tmp_win->frame_h = tmp_win->win_h;
			XUnmapWindow( dpy, tmp_win->scroll_v[3] );
			XUnmapWindow( dpy, tmp_win->scroll_h[3] );
			SetUpFrame( tmp_win, tmp_win->frame_x, tmp_win->frame_y,
					   tmp_win->frame_w, tmp_win->frame_h, True );
		}
		else{
/* Scroll Mode On */
			tmp_win->flags |= SCROLL;
			tmp_win->win_x = 0;
			tmp_win->win_y = 0;
			if( tmp_win->flags&MAXIMAIZED ){
				tmp_win->win_w = tmp_win->orig_w;
				tmp_win->win_h = tmp_win->orig_h;
			}
			else{
				tmp_win->win_w = tmp_win->frame_w;
				tmp_win->win_h = tmp_win->frame_h;
			}
			XMoveWindow( dpy, tmp_win->scroll_v[3], 0, SBAR_WH+2 );
			XMoveWindow( dpy, tmp_win->scroll_h[3], SBAR_WH+2, 0 );
			XMapWindow( dpy, tmp_win->scroll_v[3] );
			XMapWindow( dpy, tmp_win->scroll_h[3] );
		}
	}
	else{
		XBell( dpy, 30 );
		tmp_win->flags &= ~SCROLL;
	}
}

void NopFunction( char *action )
{
}

void AboutThisMachine( char *action )
{
}

void RefreshScreen( char *action )
{
	Window w;
	XSetWindowAttributes attributes;
	unsigned long valuemask;
	
	valuemask = CWBackingStore;
	attributes.background_pixel = BlackPixel( dpy, Scr.screen );
	attributes.backing_store = NotUseful;
	w = XCreateWindow (dpy, Scr.Root, 0, 0,
			   (unsigned int) Scr.MyDisplayWidth,
			   (unsigned int) Scr.MyDisplayHeight,
			   (unsigned int) 0,
			   CopyFromParent, (unsigned int) CopyFromParent,
			   (Visual *) CopyFromParent, valuemask,
			   &attributes);
	XMapWindow (dpy, w);
	XDestroyWindow (dpy, w);
	XFlush (dpy);
}

void ChangeDesk( char *action )
{
	MlvwmWindow *tmp, *Active=NULL;
	Bool LastActiveLive=False;
	int newdesk;
	char str[8], *oldwinname, *winname, builtaction[24];

	if( strchr( action, '+' ) ){
		newdesk = Scr.currentdesk+1;
		if( Scr.flags&ROTATEDESK && newdesk>Scr.n_desktop-1 )
			newdesk = 0;
		newdesk = min( Scr.n_desktop-1, newdesk );
	}
	else if( strchr( action, '-' ) ){
		newdesk = Scr.currentdesk-1;
		if( Scr.flags&ROTATEDESK && newdesk<0 )
			newdesk = Scr.n_desktop-1;
		newdesk = max( 0, newdesk );
	}
	else
		sscanf( action, "%s%d", str, &newdesk );

	if( newdesk==Scr.currentdesk )		return;

	if( (Scr.ActiveWin && 
	   ((Scr.flags&STICKSHADE && Scr.ActiveWin->flags&SHADE) ||
		Scr.ActiveWin->flags&STICKY)) &&
	   !(Scr.ActiveWin->flags&SKIPSELECT) )
		Active = Scr.ActiveWin;

	Scr.LastActive[Scr.currentdesk] = Scr.ActiveWin;

	if( !Active )
		Active = Scr.LastActive[newdesk];

	for (tmp = (MlvwmWindow *)Scr.MlvwmRoot.next; tmp != NULL;
		 tmp = (MlvwmWindow *)tmp->next){
		if( (Scr.flags&STICKSHADE && tmp->flags&SHADE) || tmp->flags&STICKY ){
			sprintf( builtaction, "Select %lX", (unsigned long)tmp );
			oldwinname = WinListName( tmp );
			tmp->Desk = newdesk;
			winname = WinListName( tmp );
			if( Scr.flags&DISPDESK && !(tmp->flags&NOWINLIST) )
				ChangeMenuItemLabel( "ICON", oldwinname, winname,
									builtaction, M_ALLSET, M_AND );
			free( winname );
			free( oldwinname );
		}

		if( newdesk==tmp->Desk ){
			MapIt( tmp );
			if( tmp==Active )
				LastActiveLive = True;
		}
		else
			UnmapIt( tmp );
    }

	sprintf( str, "Desk %d", Scr.currentdesk );
	ChangeMenuItemLabel( "ICON", str, str, NULL, SELECTON, M_COPY );
	sprintf( str, "Desk %d", newdesk );
	ChangeMenuItemLabel( "ICON", str, str, NULL, SELECTON|CHECKON, M_COPY );

	Scr.currentdesk=newdesk;
	if( !LastActiveLive )		Active = NULL;
	SetFocus( Active );

	XUngrabServer (dpy);
/*
	if( !(Scr.flags&STARTING) ){
		if( !(Scr.flags & FOLLOWTOMOUSE) && Active )
*/
	if( !(Scr.flags&STARTING) && Active ){
		RaiseMlvwmWindow( Active );
/*
		else
			KeepOnTop();
*/
	}
	return;
}

void ShowBalloon( char *action )
{
	BalloonHelp ();
}

void ShadeUnShadeActiveWindow( char *action )
{
	if( Scr.ActiveWin==NULL || !(Scr.ActiveWin->flags&TITLE) )	return;

	if( Scr.ActiveWin->flags&SHADE )
		UnShadeWindow( Scr.ActiveWin );
	else
		ShadeWindow( Scr.ActiveWin );
}

void SelectWindow( char *action )
{
	char str[7], desk[8], *oldwinname, *winname;
	MlvwmWindow *tmp, *testwin;
	int newdesk, hided=0;

	sscanf( action, "%s%lX", str, (unsigned long *)&tmp );
	if( Scr.flags&STICKHIDE && tmp->flags&HIDED ){
		oldwinname = WinListName( tmp );
		tmp->Desk = Scr.currentdesk;
		winname = WinListName( tmp );
		if( Scr.flags&DISPDESK )
			ChangeMenuItemLabel( "ICON", oldwinname, winname,
								action, M_ALLSET, M_AND );
		free( winname );
		free( oldwinname );
	}
	if( Scr.currentdesk != tmp->Desk ){
		newdesk = tmp->Desk;
		sprintf( desk, "Desk %d", newdesk );
		ChangeDesk( desk );
	}
	if( tmp->flags&HIDED ){
		for( testwin = (MlvwmWindow *)Scr.MlvwmRoot.next; testwin != NULL;
			testwin = (MlvwmWindow *)testwin->next)
			if( (testwin->flags&TRANSIENT &&
			   testwin->transientfor==tmp->w 
			   && testwin!=tmp) || testwin==tmp ){
				testwin->flags &= ~HIDED;
				MapIt( testwin );
				if( Scr.flags&ICONIFYHIDE ){
					XMapWindow( dpy, testwin->w );
					SetMapStateProp( testwin, NormalState );
				}
			}
	}
	if( !(Scr.flags&SHADEMAP) && tmp->flags&SHADE )
		UnShadeWindow( tmp );
	if( Scr.flags&FOLLOWTOMOUSE )
		XWarpPointer( dpy,None,tmp->frame,0,0,0,0,
					 tmp->frame_w/2,TITLE_HEIGHT/2 );
	else
		SetFocus( tmp );
	XSetInputFocus( dpy, tmp->w, RevertToParent, CurrentTime );
	RaiseMlvwmWindow( tmp );
	for (tmp = (MlvwmWindow *)Scr.MlvwmRoot.next; tmp != NULL;
		 tmp = (MlvwmWindow *)tmp->next)
		if( tmp->flags&HIDED )	hided++;
	if( hided==0 )
		ChangeMenuItemLabel( "ICON", "Show All", "Show All",
							NULL, STRGRAY, M_COPY );
}

void SelectNextWindow( char *action )
{
	MlvwmWindow *tmp, *last, *start, *lastactive=NULL;
	char desk[8];
	int newdesk, lastdesk=0;
	Bool samedesk=False;

	if( !Scr.MlvwmRoot.next )	return;

	if( !strcmp( action, "NextSameDeskWindow" ))		samedesk = True;

	for( last = Scr.MlvwmRoot.next; last && last->next!=NULL;
		last = last->next );

	if( Scr.ActiveWin==NULL || Scr.ActiveWin->next==NULL ){
		tmp = Scr.MlvwmRoot.next;
		start = last;
	}
	else{
		tmp = Scr.ActiveWin->next;
		start = Scr.ActiveWin;
	}

	while( (tmp->flags&HIDED || tmp->flags&SKIPSELECT ||
		  (samedesk && Scr.currentdesk!=tmp->Desk)) && tmp!=start ){
		tmp = tmp->next;
		if( tmp==NULL )		tmp = Scr.MlvwmRoot.next;
	}

	if( tmp==Scr.ActiveWin || (samedesk && Scr.currentdesk!=tmp->Desk) )
		return;

	if( Scr.currentdesk != tmp->Desk ){
		newdesk = tmp->Desk;
		sprintf( desk, "Desk %d", newdesk );
		Scr.LastActive[newdesk] = tmp;
		if( Scr.ActiveWin && Scr.ActiveWin->flags&STICKY ){
			lastactive = Scr.ActiveWin;
			lastdesk = Scr.currentdesk;
			SetFocus( NULL );
		}
		ChangeDesk( desk );
		if( lastactive )
			Scr.LastActive[lastdesk] = lastactive;
	}
	else
		RaiseMlvwmWindow( tmp );

	if( Scr.flags&FOLLOWTOMOUSE )
		XWarpPointer( dpy,None,tmp->frame,0,0,0,0,
					 tmp->frame_w/2,TITLE_HEIGHT/2 );
	else
		SetFocus( tmp );
}

void SelectPreviousWindow( char *action )
{
	MlvwmWindow *tmp, *last, *start, *lastactive=NULL;
	char desk[8];
	int newdesk, lastdesk=0;
	Bool samedesk=False;

	if( !Scr.MlvwmRoot.next )	return;

	if( !strcmp( action, "PreviousSameDeskWindow" ))
		samedesk = True;

	for( last = Scr.MlvwmRoot.next; last && last->next!=NULL;
		last = last->next );

	if( Scr.ActiveWin==NULL || Scr.ActiveWin->prev==&Scr.MlvwmRoot ){
		tmp = last;
		start = Scr.MlvwmRoot.next;
	}
	else{
		tmp = Scr.ActiveWin->prev;
		start = Scr.ActiveWin;
	}
	while( (tmp->flags&HIDED || tmp->flags&SKIPSELECT ||
		  (samedesk && Scr.currentdesk!=tmp->Desk)) &&
		  tmp!=start ){
		tmp = tmp->prev;
		if( tmp==&Scr.MlvwmRoot )	tmp = last;
	}
	if( tmp==Scr.ActiveWin || (samedesk && Scr.currentdesk!=tmp->Desk) )
		return;
	if( Scr.currentdesk != tmp->Desk ){
		newdesk = tmp->Desk;
		sprintf( desk, "Desk %d", newdesk );
		Scr.LastActive[newdesk] = tmp;
		if( Scr.ActiveWin && Scr.ActiveWin->flags&STICKY ){
			lastactive = Scr.ActiveWin;
			lastdesk = Scr.currentdesk;
			SetFocus( NULL );
		}
		ChangeDesk( desk );
		if( lastactive )
			Scr.LastActive[lastdesk] = lastactive;
	}
	else
		RaiseMlvwmWindow( tmp );
	if( Scr.flags&FOLLOWTOMOUSE )
		XWarpPointer( dpy,None,tmp->frame,0,0,0,0,
					 tmp->frame_w/2,TITLE_HEIGHT/2 );
	else
		SetFocus( tmp );
}

void HideWindow( MlvwmWindow *mw )
{
	char builtaction[24];
	char *winname;

	winname = WinListName( mw );
	sprintf( builtaction, "Select %lX", (unsigned long)mw );
	ChangeMenuItemLabel( "ICON", winname, winname,
						builtaction, ICONGRAY|SELECTON, M_COPY );
	if( mw->Desk==Scr.currentdesk ){
		UnmapIt( mw );
		if( mw->flags & MAPPED && !(mw->flags & HIDED) )
			IconZoom( mw, False );
	}
	mw->flags |= HIDED;
	if( Scr.flags&ICONIFYHIDE ){
		XMapWindow( dpy, mw->w );
		SetMapStateProp( mw, IconicState );
	}
	free( winname );
}

void HideActiveWindow( char *action )
{
	MlvwmWindow *NextActive, *testwin;

	if( Scr.ActiveWin ){
		NextActive = NextActiveWin( Scr.ActiveWin );
		for( testwin = (MlvwmWindow *)Scr.MlvwmRoot.next; testwin != NULL;
			testwin = (MlvwmWindow *)testwin->next)
			if( testwin->flags&TRANSIENT &&
			   testwin->transientfor==Scr.ActiveWin->w 
			   && testwin!=Scr.ActiveWin )
				HideWindow( testwin );

		HideWindow( Scr.ActiveWin );
		if( !(Scr.flags & FOLLOWTOMOUSE) ){
			SetFocus( NextActive );
			if( NextActive )
				RaiseMlvwmWindow( NextActive );
		}
		ChangeMenuItemLabel( "ICON", "Show All", "Show All",
							NULL, SELECTON, M_COPY );
	}
	else
		XBell( dpy, 30 );
}

void HideOtherWindow( char *action )
{
	MlvwmWindow *tmp;
	int hidewin=0;

	if( !Scr.ActiveWin )		return;
	for (tmp = (MlvwmWindow *)Scr.MlvwmRoot.next; tmp != NULL;
		 tmp = (MlvwmWindow *)tmp->next){
		if( tmp!=Scr.ActiveWin ){
			HideWindow( tmp );
			hidewin++;
		}
    }
	if( hidewin )
		ChangeMenuItemLabel( "ICON", "Show All", "Show All",
							NULL, SELECTON, M_COPY );
}

void ShowAllWindow( char *action )
{
	MlvwmWindow *tmp, *ActiveWin=NULL;
	char builtaction[24], *winname;

	for (tmp = (MlvwmWindow *)Scr.MlvwmRoot.next; tmp != NULL;
		 tmp = (MlvwmWindow *)tmp->next){
		sprintf( builtaction, "Select %lX", (unsigned long)tmp );
		winname = WinListName( tmp );
		ChangeMenuItemLabel( "ICON", winname, winname,
							builtaction, SELECTON, M_COPY );
		free( winname );
		if( tmp->flags&HIDED ) {
/*
			IconZoom( tmp, True );
*/
			tmp->flags &= ~HIDED;
		}
		if( tmp->Desk==Scr.currentdesk ){
			MapIt( tmp );
			ActiveWin=tmp;
		}
		if( Scr.flags&ICONIFYHIDE ){
			XMapWindow( dpy, tmp->w );
			SetMapStateProp( tmp, NormalState );
		}
		if( !(Scr.flags&SHADEMAP) && tmp->flags&SHADE )
			UnShadeWindow( tmp );
    }
	ChangeMenuItemLabel( "ICON", "Show All", "Show All",
						NULL, STRGRAY, M_COPY );
	if( ActiveWin ){
		if( Scr.flags&FOLLOWTOMOUSE )
			XWarpPointer( dpy,None,ActiveWin->frame,0,0,0,0,
						 ActiveWin->frame_w/2,TITLE_HEIGHT/2 );
		else
			SetFocus( ActiveWin );
		XSetInputFocus( dpy, ActiveWin->w, RevertToParent, CurrentTime );
		RaiseMlvwmWindow( ActiveWin );
	}
}

void ExecSystems( char *action )
{
	char *comm;

	if( strchr( action, '"' ) )
		comm = strrchr( action, '"' )+1;
	else
		comm = action+5;

	if( Scr.flags & DEBUGOUT )
		fprintf( stderr, "Exec System <%s> !\n", comm );
	if (!(fork())){ /* child process */
		if( Scr.flags & DEBUGOUT )
			fprintf( stderr, "Fork Successful %s\n", comm );
		if (execl("/bin/sh", "/bin/sh", "-c", comm, (char *)0)==-1){
			fprintf( stderr, "Exec fail %s\n", comm );
			exit(100);
		}
	}
}

void RestartSystem( char *action )
{
	char *top;

	top = SkipSpace( SkipNonSpace( action+6 ));
	if( strncmp( top, "mlvwm", 5 ) )
		Done( 1, top );
	else
		Done( 1, NULL );
}

void ExitSystem( char *action )
{
	Done( 0, NULL );
}

extern struct configure key_modifiers[];

void SendMessage( char *action )
{
	XKeyEvent ev;
	KeySym keysym;
	KeyCode keycode=0;
	Bool quota=False;
	char *mem, *top, *mod, *key;
	char oldchar, code[2];
	int lp, tag;

	if( !(Scr.ActiveWin) )		return;
	mem = top = strdup( SkipSpace( SkipNonSpace( action ) ) );
	while( *top!='\n' && *top!='\0' ){
		key = top;
		top = SkipNonSpace( top );
		oldchar = *top;
		*top = '\0';
		mod = NULL;
		if( strchr( key, '+' ) ){
			mod = key;
			key = strchr( key, '+' );
			*key = '\0';
			key++;
		}
		ev.state = 0;
		for( lp=0; mod && mod[lp]!='\0'; lp++ ){
			for( tag=0; key_modifiers[tag].key!=0; tag++ ){
				if( key_modifiers[tag].key==mod[lp] )
					ev.state |= key_modifiers[tag].value;
			}
		}
		if( key[0]=='"' )		quota = True;
		code[1] = '\0';
		do{
			if( quota ){
				key++;
				keycode = 0;
				if( *key=='"' )
					quota = False;
				else{
					code[0] = *key;
					if((keysym = XStringToKeysym(code)) != NoSymbol )
						keycode = XKeysymToKeycode(dpy, keysym);
				}
			}
			else{
				if((keysym = XStringToKeysym(key)) != NoSymbol )
					keycode = XKeysymToKeycode(dpy, keysym);
			}
			ev.keycode = keycode;
			ev.type = KeyPress;
			ev.window = Scr.ActiveWin->w;
			if( ev.keycode != 0 )
				XSendEvent(dpy,Scr.ActiveWin->w,False,
						   KeyPressMask,(XEvent *)&ev);
		}
		while( quota );
		*top = oldchar;
		top = SkipSpace( top );
	}
	free( mem );
}

void ToggleBalloon( char *action )
{
	if( Scr.flags&BALLOONON ){
		Scr.flags &= ~BALLOONON;
		if( Scr.BalloonOffStr && Scr.BalloonOnStr){
			ChangeMenuItemLabel( "Balloon", Scr.BalloonOnStr,
								Scr.BalloonOffStr, NULL, SELECTON, M_COPY );
		}
	}
	else{
		Scr.flags |= BALLOONON;
		if( Scr.BalloonOffStr && Scr.BalloonOnStr){
			ChangeMenuItemLabel( "Balloon", Scr.BalloonOffStr, 
								Scr.BalloonOnStr, NULL, SELECTON, M_COPY );
		}
	}
}

void WaitMap( char *action )
{
	XEvent ev;
	XClassHint class;
	MlvwmWindow *tmp_win;
	Bool done=True;
	char *name, *waitname, *point;
	Atom *protocols = NULL;
	XTextProperty text_prop;
#ifdef USE_LOCALE
	int num;
	char **list;
#endif

	waitname = SkipSpace( action+5 );
	point=SkipNonSpace(waitname);
	if( *point!='\0' )
		*point = '\0';
	else
		*(--point)='\0';

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
			if( !strcmp( name, waitname ) ||
			   !strcmp( class.res_name, waitname ) ||
			   !strcmp( class.res_class, waitname ) ){
				done = False;
				if (protocols) XFree ((char *) protocols);
			}
		}
		HandleEvents( ev );
		if( Scr.flags&STARTING && ev.type==MapNotify &&
		   XFindContext( dpy, ev.xany.window,
						MlvwmContext, (caddr_t *)&tmp_win )!=XCNOENT)
			DrawStringMenuBar( tmp_win->name );
	}
	while( done );
}

void DebugWinList( char *action )
{
	MlvwmWindow *tmp;

	printf("Start\n");
	for( tmp = Scr.MlvwmRoot.next; tmp!=NULL; tmp = tmp->next ){
		printf("Name %s[%lX] Desk %d",tmp->name,(unsigned long)tmp,tmp->Desk);
		if( tmp==Scr.ActiveWin )			printf("<=\n");
		else	printf("\n");
	}
	printf("End\n");
}

builtin_func funcs[] = {
	{ "DebugWinList", DebugWinList },
	{ "About", AboutThisMachine },
	{ "Desk", ChangeDesk },
	{ "Exec", ExecSystems },
	{ "Exit", ExitSystem },
	{ "ShowBalloon", ShowBalloon },
	{ "HideActive", HideActiveWindow },
	{ "HideOthers", HideOtherWindow },
	{ "KillWindow", KillWindowFunction },
	{ "MoveWindow", MoveWindowFunction },
	{ "NextWindow", SelectNextWindow },
	{ "NextSameDeskWindow", SelectNextWindow },
	{ "Nop", NopFunction },
	{ "PreviousWindow", SelectPreviousWindow },
	{ "PreviousSameDeskWindow", SelectPreviousWindow },
	{ "Refresh", RefreshScreen },
	{ "ResizeWindow", ResizeWindowFunction },
	{ "Restart", RestartSystem },
	{ "Select", SelectWindow },
	{ "SendMessage", SendMessage },
	{ "ShadeUnShadeActive", ShadeUnShadeActiveWindow },
	{ "ShowAll", ShowAllWindow },
	{ "ToggleScroll", ToggleScrollFunction },
	{ "ToggleBalloon", ToggleBalloon },
	{ "Wait", WaitMap },
	{ NULL, 0 }
};

void ExecuteFunction( char *act )
{
	int lp;

	if( act==NULL )		return;
	for( lp=0; funcs[lp].label!=NULL; lp++ )
		if( !strncmp( act, funcs[lp].label, strlen( funcs[lp].label ) ) )
			funcs[lp].action( act );
}
