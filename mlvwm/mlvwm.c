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
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "mlvwm.h"
#include "screen.h"
#include "event.h"
#include "add_window.h"
#include "config.h"
#include "functions.h"
#include "misc.h"

#include <X11/Xproto.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <X11/extensions/shape.h>
#include <X11/xpm.h>
#ifdef USE_LOCALE
#include <X11/Xlocale.h>
#endif

static unsigned char start_mesh[] = { 0x03, 0x03, 0x0c, 0x0c};

ScreenInfo Scr;
Display *dpy;
int xfd;
XContext MlvwmContext;
XContext MenuContext;
XClassHint NoClass;

static char **g_argv;
int ShapeEventBase, ShapeErrorBase;

void CreateCursors( void )
{
	Scr.MlvwmCursors[DEFAULT] = XCreateFontCursor( dpy, XC_left_ptr );
	Scr.MlvwmCursors[SYS] = XCreateFontCursor( dpy, XC_X_cursor );
	Scr.MlvwmCursors[TITLE_CURSOR] = XCreateFontCursor( dpy, XC_hand2 );
	Scr.MlvwmCursors[RESIZE] =
		XCreateFontCursor( dpy, XC_bottom_right_corner );
	Scr.MlvwmCursors[MOVE] = XCreateFontCursor( dpy, XC_fleur );
	Scr.MlvwmCursors[MENU] = XCreateFontCursor( dpy, XC_sb_left_arrow );
	Scr.MlvwmCursors[WAIT] = XCreateFontCursor( dpy, XC_watch );
	Scr.MlvwmCursors[SELECT] = XCreateFontCursor( dpy, XC_dot );
	Scr.MlvwmCursors[DESTROY] = XCreateFontCursor( dpy, XC_pirate );
	Scr.MlvwmCursors[SBARH_CURSOR] =
		XCreateFontCursor( dpy, XC_sb_h_double_arrow );
	Scr.MlvwmCursors[SBARV_CURSOR] =
		XCreateFontCursor( dpy, XC_sb_v_double_arrow );
	Scr.MlvwmCursors[MINMAX_CURSOR] = XCreateFontCursor( dpy, XC_sizing );
	Scr.MlvwmCursors[SHADER_UP_CURSOR] =
		XCreateFontCursor( dpy, XC_based_arrow_up );
	Scr.MlvwmCursors[SHADER_DOWN_CURSOR] =
		XCreateFontCursor( dpy, XC_based_arrow_down );
}

void LoadDefaultFonts( void )
{
#ifdef USE_LOCALE
	char **miss, *def;
	int n_miss, lp;

	Scr.MenuBarFs =	XCreateFontSet( dpy, DEFAULTFS, &miss, &n_miss, &def );
	if( n_miss>0 ){
		for( lp=0; lp<n_miss; lp++ )
			DrawErrMsgOnMenu( "Can't load default font ", miss[lp] );
		XFreeStringList( miss );
	}
	Scr.MenuFs = XCreateFontSet( dpy, DEFAULTFS, &miss, &n_miss, &def );
	if( n_miss>0 ){
		for( lp=0; lp<n_miss; lp++ )
			DrawErrMsgOnMenu( "Can't load default font ", miss[lp] );
		XFreeStringList( miss );
	}
	Scr.WindowFs = XCreateFontSet( dpy, DEFAULTFS, &miss, &n_miss, &def );
	if( n_miss>0 ){
		for( lp=0; lp<n_miss; lp++ )
			DrawErrMsgOnMenu( "Can't load default font ", miss[lp] );
		XFreeStringList( miss );
	}
	Scr.BalloonFs = XCreateFontSet( dpy, DEFAULTFS, &miss, &n_miss, &def );
	if( n_miss>0 ){
		for( lp=0; lp<n_miss; lp++ )
			DrawErrMsgOnMenu( "Can't load default font ", miss[lp] );
		XFreeStringList( miss );
	}
#else
	if(( Scr.MenuBarFont = XLoadQueryFont( dpy, DEFAULTFONT )) == NULL )
		DrawErrMsgOnMenu( "Can't load default font ", "MenuBar" );
	if(( Scr.MenuFont =	XLoadQueryFont( dpy, DEFAULTFONT )) == NULL )
		DrawErrMsgOnMenu( "Can't load default font ", "Menu" );
	if(( Scr.WindowFont = XLoadQueryFont( dpy, DEFAULTFONT )) == NULL )
		DrawErrMsgOnMenu( "Can't load default font ", "Window" );
	if(( Scr.BalloonFont = XLoadQueryFont( dpy, DEFAULTFONT )) == NULL )
		DrawErrMsgOnMenu( "Can't load default font ", "Balloon" );
#endif
}

void FreeFont( void )
{
#ifdef USE_LOCALE
	XFreeFontSet( dpy, Scr.MenuBarFs );
	XFreeFontSet( dpy, Scr.MenuFs );
	XFreeFontSet( dpy, Scr.WindowFs );
#else
	XFreeFont( dpy, Scr.MenuBarFont );
	XFreeFont( dpy, Scr.MenuFont );
	XFreeFont( dpy, Scr.WindowFont );
#endif
}

void InitScrParams( void )
{
	unsigned char mask[] = {0x01, 0x02};
	
	Scr.d_depth = DefaultDepth( dpy, Scr.screen );
	Scr.n_desktop = 1;
	Scr.Restarting = False;
	{
		Atom atype;
		int aformat;
		unsigned long nitems, bytes_remain;
		unsigned char *prop;
    
		Scr.currentdesk = 0;
		if ((XGetWindowProperty(dpy, Scr.Root, _XA_WM_DESKTOP,
			 0L, 1L, True, _XA_WM_DESKTOP, &atype, &aformat,
			 &nitems, &bytes_remain, &prop))==Success){
			if(prop != NULL){
				Scr.Restarting = True;
				Scr.currentdesk = *(unsigned long *)prop;
			}
		}
	}
	Scr.MlvwmRoot.w = Scr.Root;
	Scr.MlvwmRoot.prev = NULL;
	Scr.MlvwmRoot.next = NULL;
	Scr.LastActive = calloc( 1, sizeof(MlvwmWindow));
	Scr.MyDisplayWidth = DisplayWidth( dpy, Scr.screen );
	Scr.MyDisplayHeight = DisplayHeight( dpy, Scr.screen );
	Scr.MenuLabelRoot = NULL;
	Scr.MenuRoot = NULL;
	Scr.ActiveMenu = NULL;
	Scr.IconMenu.m_item = NULL;
	Scr.ActiveWin = NULL;
	Scr.root_pushes = 0;
	Scr.pushed_window = &Scr.MlvwmRoot;
	Scr.style_list = NULL;
	Scr.ShortCutRoot = NULL;
	Scr.double_click_time = 300;
	if( Scr.flags&SYSTEM8 )
		Scr.bar_width = 16;
	else
		Scr.bar_width = 19;
	Scr.flush_time = 100000;
	Scr.flush_times = 2;
	Scr.zoom_wait = 10000;
	Scr.IconPath = NULL;
	Scr.BalloonOffStr = NULL;
	Scr.BalloonOnStr = NULL;
	Scr.mask = XCreatePixmapFromBitmapData( dpy, Scr.Root, mask, 2, 2, 
										   WhitePixel( dpy, Scr.screen ),
										   BlackPixel( dpy, Scr.screen ),
										   Scr.d_depth );
	Scr.StartFunc = NULL;
	Scr.flags |= STARTING;
	Scr.resist_x = 0;
	Scr.resist_y = 0;
}

void InitGCs( void )
{
	XGCValues gcv;
	unsigned long gcm;

	gcm = GCFunction|GCForeground|GCSubwindowMode|GCLineWidth|GCLineStyle;
	gcv.function = GXxor;
	gcv.subwindow_mode = IncludeInferiors;
	gcv.line_width = 1;

	if( Scr.d_depth>1 )
		gcv.foreground = GetColor( "#777777" );
	else
		gcv.foreground = WhitePixel( dpy, Scr.screen );
	gcv.line_style = FillSolid;

	Scr.RobberGC = XCreateGC( dpy, Scr.Root, gcm, &gcv );

	gcm = GCFunction | GCPlaneMask | GCForeground | GCBackground | GCTile;
	gcv.function = GXcopy;
	gcv.plane_mask = AllPlanes;
	gcv.line_width = 0;
	gcv.fill_style = FillSolid;
	gcv.tile = Scr.mask;

	gcv.foreground = BlackPixel( dpy, Scr.screen );
	gcv.background = WhitePixel( dpy, Scr.screen );
	Scr.BlackGC = XCreateGC( dpy, Scr.Root, gcm, &gcv );

	gcv.foreground = WhitePixel( dpy, Scr.screen );
	gcv.background = BlackPixel( dpy, Scr.screen );
	Scr.WhiteGC = XCreateGC( dpy, Scr.Root, gcm, &gcv );

	if( Scr.d_depth>1 )	gcv.foreground = GetColor( "#444444" );
	gcv.background = WhitePixel( dpy, Scr.screen );
	Scr.Gray1GC = XCreateGC( dpy, Scr.Root, gcm, &gcv );

	if( Scr.d_depth>1 )	gcv.foreground = GetColor( "#777777" );
	gcv.background = WhitePixel( dpy, Scr.screen );
	Scr.Gray2GC = XCreateGC( dpy, Scr.Root, gcm, &gcv );

	if( Scr.d_depth>1 )		gcv.foreground = GetColor( "#bbbbbb" );
	if( Scr.d_depth>1 )		gcv.foreground = GetColor( "#aaaaaa" );
	gcv.background = WhitePixel( dpy, Scr.screen );
	Scr.Gray3GC = XCreateGC( dpy, Scr.Root, gcm, &gcv );

	if( Scr.d_depth>1 )	gcv.foreground = GetColor( "#e0e0e0" );
	gcv.background = WhitePixel( dpy, Scr.screen );
	Scr.Gray4GC = XCreateGC( dpy, Scr.Root, gcm, &gcv );

	if( Scr.d_depth>1 )	gcv.foreground = GetColor( "#3333ff" );
	gcv.background = WhitePixel( dpy, Scr.screen );
	Scr.MenuSelectBlueGC = XCreateGC( dpy, Scr.Root, gcm, &gcv );

	if( Scr.d_depth>1 )	gcv.foreground = GetColor( "#dddddd" );
	gcv.background = WhitePixel( dpy, Scr.screen );
	Scr.MenuBlueGC = XCreateGC( dpy, Scr.Root, gcm, &gcv );

	if( Scr.d_depth>1 )	gcv.foreground = GetColor( "#ccccff" );
	gcv.background = WhitePixel( dpy, Scr.screen );
	Scr.ScrollBlueGC = XCreateGC( dpy, Scr.Root, gcm, &gcv );
}

void InitVariables( void )
{
	MlvwmContext = XUniqueContext();

	InitScrParams();
	InitGCs();

	NoClass.res_name = NoName;
	NoClass.res_class = NoName;

	AddMenuItem( &(Scr.IconMenu), "Hide Active", "HideActive",
				NULL, NULL, NULL, STRGRAY );
	AddMenuItem( &(Scr.IconMenu), "Hide Others", "HideOthers",
				NULL, NULL, NULL, STRGRAY );
	AddMenuItem( &(Scr.IconMenu), "Show All", "ShowAll",
				NULL, NULL, NULL, STRGRAY );
	AddMenuItem( &(Scr.IconMenu), "", "Nop", NULL, NULL, NULL, STRGRAY );
}

Atom _XA_MIT_PRIORITY_COLORS;
Atom _XA_WM_CHANGE_STATE;
Atom _XA_WM_STATE;
Atom _XA_WM_COLORMAP_WINDOWS;
Atom _XA_WM_PROTOCOLS;
Atom _XA_WM_TAKE_FOCUS;
Atom _XA_WM_DELETE_WINDOW;
Atom _XA_WM_DESKTOP;

void InternUsefulAtoms (void)
{
	/* 
	 * Create priority colors if necessary.
	 */
	_XA_MIT_PRIORITY_COLORS = XInternAtom(dpy, "_MIT_PRIORITY_COLORS", False);   
	_XA_WM_CHANGE_STATE = XInternAtom (dpy, "WM_CHANGE_STATE", False);
	_XA_WM_STATE = XInternAtom (dpy, "WM_STATE", False);
	_XA_WM_COLORMAP_WINDOWS = XInternAtom (dpy, "WM_COLORMAP_WINDOWS", False);
	_XA_WM_PROTOCOLS = XInternAtom (dpy, "WM_PROTOCOLS", False);
	_XA_WM_TAKE_FOCUS = XInternAtom (dpy, "WM_TAKE_FOCUS", False);
	_XA_WM_DELETE_WINDOW = XInternAtom (dpy, "WM_DELETE_WINDOW", False);
	_XA_WM_DESKTOP = XInternAtom (dpy, "WM_DESKTOP", False);

	return;
}

int MappedNotOverride( Window w )
{
    XWindowAttributes wa;

    XGetWindowAttributes(dpy, w, &wa);
    return ((wa.map_state != IsUnmapped) && (wa.override_redirect != True));
}

void RepaintAllWindows( Window w )
{
	Window parent, *children;
	XWindowAttributes attributes;
	unsigned nchildren, lp;
	MlvwmWindow *t;

	XQueryTree( dpy, Scr.Root, &Scr.Root, &parent, &children, &nchildren );
	for( lp=0; nchildren > lp; lp++ ){
		if( children[lp]==w || children[lp]==Scr.MenuBar )	continue;
		XGetWindowAttributes( dpy, children[lp], &attributes );
		if( IsUnmapped ==attributes.map_state )	continue;
		if( children[lp] && MappedNotOverride(children[lp]) ){
			HandleMapRequest( children[lp] );
			if( XFindContext( dpy, children[lp],
							 MlvwmContext, (caddr_t *)&t)!=XCNOENT)
				DrawStringMenuBar( t->name );
		}
		XLowerWindow( dpy, children[lp] );
	}
	XFree( children );
}

void Reborder( Bool restart )
{
	MlvwmWindow *tmp;

	XGrabServer (dpy);
	InstallWindowColormaps( &Scr.MlvwmRoot );	/* force reinstall */
	for (tmp = (MlvwmWindow *)Scr.MlvwmRoot.next; tmp != NULL;
		 tmp = (MlvwmWindow *)tmp->next){
		XUnmapWindow(dpy,tmp->frame);
		RestoreWithdrawnLocation( tmp, restart );
    }
	XUngrabServer (dpy);
	XSetInputFocus (dpy, PointerRoot, RevertToPointerRoot,CurrentTime);
}

void SaveDesktopState( void )
{
	MlvwmWindow *t;
	unsigned long data[1];

	for (t = Scr.MlvwmRoot.next; t != NULL; t = t->next){
		data[0] = (unsigned long) t->Desk;
		XChangeProperty (dpy, t->w, _XA_WM_DESKTOP, _XA_WM_DESKTOP, 32,
						 PropModeReplace, (unsigned char *) data, 1);
    }

	data[0] = (unsigned long) Scr.currentdesk;
	XChangeProperty (dpy, Scr.Root, _XA_WM_DESKTOP, _XA_WM_DESKTOP, 32,
					 PropModeReplace, (unsigned char *) data, 1);

	XSync(dpy, 0);
}

void Done( int restart, char *command )
{
	char *my_argv[10];
	int i,done;

	strcpy( Scr.ErrorFunc, "Start Done" );

	FreeMenu();
	strcpy( Scr.ErrorFunc, "FreeMenu Done" );

	FreeShortCut();
	strcpy( Scr.ErrorFunc, "FreeShortCut Done" );

	FreeStyles();
	strcpy( Scr.ErrorFunc, "FreeStyles Done" );

	FreeFont();
	strcpy( Scr.ErrorFunc, "FreeFont Done" );

	if( Scr.LastActive )		free( Scr.LastActive );
	if( Scr.Balloon!=None )		XDestroyWindow( dpy, Scr.Balloon );

	Reborder( restart==0?False:True );
	strcpy( Scr.ErrorFunc, "Reborder Done" );

	if(restart){
		i=0;
		done = 0;
		while((g_argv[i] != NULL)&&(i<8)){
			if(strcmp(g_argv[i],"-s")==0)
				done = 1;
			my_argv[i] = g_argv[i];
			i++;
        }
		if(!done)
			my_argv[i++] = "-s";
		while(i<10)
			my_argv[i++] = NULL;
		SaveDesktopState();
		XSelectInput(dpy, Scr.Root, 0 );
		XSync(dpy, 0);
		XCloseDisplay(dpy);

		sleep( 1 );
		ReapChildren();

		if( command != NULL )
			execvp(command,g_argv);
		else
			execvp( *g_argv,g_argv);

		fprintf( stderr, "Call of '%s' failed!!!!", command);
		execvp( *g_argv, g_argv);
		fprintf( stderr, "Call of '%s' failed!!!!", g_argv[0]);
	}
	else{
		XCloseDisplay( dpy );
		exit(0);
	}
}

void SigDone(int nonsense)
{
	fprintf( stderr, "Catch Signal in [%s]\n", Scr.ErrorFunc );
	Done(0, NULL);
	return;
}

void setsighandle( int sig )
{
	if( signal( sig, SIG_IGN ) != SIG_IGN )
		signal( sig, SigDone );
}

void usage( void )
{
	fprintf( stderr, "Mlvwm Ver %s\n\n", VERSION );
	fprintf( stderr, "mlvwm [-d display] [-f config_file]");
	fprintf( stderr, " [-debug]\n" );
}

XErrorHandler CatchRedirectError(Display *err_dpy, XErrorEvent *event)
{
	fprintf( stderr, "MLVWM : another WM may be running.\n" );
	exit(1);
}

XErrorHandler MlvwmErrorHandler(Display *err_dpy, XErrorEvent *event)
{
	char err_msg[80];

	/* some errors are acceptable, mostly they're caused by 
	 * trying to update a lost  window */
	if((event->error_code == BadWindow) ||
	   (event->request_code == X_GetGeometry) ||
	   (event->error_code==BadDrawable) ||
	   (event->request_code==X_SetInputFocus) ||
	   (event->request_code == X_InstallColormap))
		return 0 ;

	XGetErrorText( err_dpy, event->error_code, err_msg, 80 );
	fprintf( stderr, "MLVWM : X Protocol error\n" );
	fprintf( stderr, "   Error detected : %s\n", err_msg );
	fprintf( stderr, "      Protocol Request : %d\n", event->request_code );
	fprintf( stderr, "      Error            : %d\n", event->error_code);
	fprintf( stderr, "      Resource ID      : 0x%x\n", 
			(unsigned int)event->resourceid );
	fprintf( stderr,"\n");
	return 0;
}

Window CreateStartWindow( void )
{
	Pixmap p_map;
	unsigned long valuemask;
	Window StartWin;
	XSetWindowAttributes attributes;

	if( Scr.flags & DEBUGOUT )
		fprintf( stderr, "Display Startup Screen !\n" ); 
	p_map = XCreatePixmapFromBitmapData( dpy, Scr.Root, start_mesh, 4, 4, 
								WhitePixel( dpy, Scr.screen ),
								BlackPixel( dpy, Scr.screen ), Scr.d_depth );
	if( Scr.flags & DEBUGOUT )
		fprintf( stderr, "Pixmap Create !\n" ); 
	valuemask = CWBackPixmap | CWBackingStore | CWCursor;
	attributes.background_pixmap = p_map;
	attributes.cursor = Scr.MlvwmCursors[WAIT];
	attributes.backing_store = NotUseful;
	StartWin = XCreateWindow (dpy, Scr.Root, 0, 0,
							  (unsigned int) Scr.MyDisplayWidth,
							  (unsigned int) Scr.MyDisplayHeight,
							  (unsigned int) 0,
							  CopyFromParent, (unsigned int) CopyFromParent,
							  (Visual *) CopyFromParent, valuemask,
							  &attributes);
	XMapRaised (dpy, StartWin);
	XFreePixmap( dpy, p_map );
	XFlush (dpy);

	return StartWin;
}

void SegFault( int nosense )
{
	fprintf( stderr,"Segmentation Fault\n" );
	fprintf( stderr,"\tin %s\n", Scr.ErrorFunc );
	exit( -1 );
}

void DoStartFunc( void )
{
	ShortCut *now, *prev;

	now = Scr.StartFunc;
	while( now ){
		ExecuteFunction( now->action );
		prev = now;
		now = now->next;
		free( prev->action );
		free( prev );
	}
}

int main( int argc, char *argv[] )
{
	char *display_name=NULL;
	char *display_screen;
	char *display_string;
	char *config_file=NULL;
	char message[255];
	char *cp;
	Window StartWin;
	XSetWindowAttributes attributes;
	int len, lp;
	Bool single = False;

	Scr.flags = 0;
	for( lp=1; lp<argc; lp++ ){
		if( !strncmp( argv[lp], "-d", 2 ) && strlen(argv[lp])==2 ){
			if( ++lp>=argc )	usage();
			else				display_name = argv[lp];
			continue;
		}
		if( !strncmp( argv[lp], "-f", 2 ) ){
			if( ++lp>=argc )	usage();
			else				config_file = argv[lp];
			continue;
		}
		if( !strncmp( argv[lp], "-s", 2 ) ){
			single = True;
			continue;
		}
		if( !strncmp( argv[lp], "-debug", 6 )){
			Scr.flags |= DEBUGOUT;
			continue;
		}
		usage();
	}
	if( Scr.flags & DEBUGOUT )
		fprintf( stderr, "Welcome to MLVWM World !\n" );
#ifdef USE_LOCALE
	if( setlocale( LC_CTYPE, "" )==NULL ){
		fprintf( stderr, "Can't figure out your locale.\n" );
		fprintf( stderr, "Check $LANG.\n" );
	}
	if( XSupportsLocale() == False ){
		fprintf( stderr, "Can't support your local.\n" );
		fprintf( stderr, "Use \"C\" locale.\n" );
		setlocale( LC_CTYPE, "C" );
	}
#endif
	if( !(dpy = XOpenDisplay( display_name )) ){
		fprintf( stderr, "can't open display %s\n",XDisplayName(display_name));
		exit( 1 );
	}
    xfd = XConnectionNumber(dpy);
    
    if( fcntl( xfd, F_SETFD, 1 ) == -1){
        fprintf( stderr, "Close-on-exec failed\n" );
        exit (1);
	}

	Scr.screen = DefaultScreen( dpy );

	g_argv = argv;

	setsighandle( SIGINT );
	setsighandle( SIGHUP );
	setsighandle( SIGQUIT );
	setsighandle( SIGTERM );
	signal( SIGSEGV, SegFault );
    Scr.NumberOfScreens = ScreenCount(dpy);

    if(!single){
        for(lp=0;lp<Scr.NumberOfScreens;lp++){
            if(lp!= Scr.screen){
                len = strlen(XDisplayString(dpy)) + 10;
                display_screen = calloc(len, sizeof(char));
                snprintf(display_screen, len, "%s", XDisplayString(dpy));
                /*
                 * Truncate the string 'whatever:n.n' to 'whatever:n',
                 * and then append the screen number.
                 */
                cp = strchr(display_screen, ':');
                if (cp != NULL)
                {
                  cp = strchr(cp, '.');
                  if (cp != NULL)
                  *cp = '\0';  /* truncate at display part */
                }
                snprintf(message, sizeof(message), "%s -d %s.%d -s", argv[0], display_screen, lp);
                free(display_screen);

                if( Scr.flags & DEBUGOUT)
                    snprintf(message + strlen(message), sizeof(message) - strlen(message), " -debug");
                if( config_file != NULL )
                    snprintf(message + strlen(message), sizeof(message) - strlen(message) ," -f %s", config_file);
                snprintf(message + strlen(message), sizeof(message) - strlen(message), " &\n");

                system(message);
			}
		}
	}
	len = strlen( XDisplayString( dpy ) );
	display_string = calloc( len+10, sizeof( char ) );
	snprintf( display_string, len+10, "DISPLAY=%s", XDisplayString(dpy) );
	putenv( display_string );

	XShapeQueryExtension( dpy, &ShapeEventBase, &ShapeErrorBase );
	InternUsefulAtoms();

	Scr.Root = RootWindow( dpy, Scr.screen );
	if( Scr.Root == None ){
		fprintf( stderr, "Root window don't exist\n" );
		exit( 1 );
	}
    XChangeProperty (dpy, Scr.Root, _XA_MIT_PRIORITY_COLORS,
                     XA_CARDINAL, 32, PropModeReplace, NULL, 0);
    XSetErrorHandler((XErrorHandler)CatchRedirectError);
	XSelectInput( dpy, Scr.Root,
				 PropertyChangeMask |
				 SubstructureRedirectMask | KeyPressMask |
				 SubstructureNotifyMask |
				 ButtonPressMask | ButtonReleaseMask );
	XSync( dpy, 0 );

    XSetErrorHandler((XErrorHandler)MlvwmErrorHandler);

	CreateCursors();
	InitVariables();
	XGrabServer( dpy );

    attributes.event_mask = KeyPressMask|FocusChangeMask;
    attributes.override_redirect = True;
    Scr.NoFocusWin=XCreateWindow(dpy,Scr.Root,-10, -10, 10, 10, 0, 0,
                                 InputOnly,CopyFromParent,
                                 CWEventMask|CWOverrideRedirect,
                                 &attributes);
    XMapWindow(dpy, Scr.NoFocusWin);
    XSetInputFocus (dpy, Scr.NoFocusWin, RevertToParent, CurrentTime);

	StartWin = CreateStartWindow();
	CreateMenuBar();
	LoadDefaultFonts();

	if( Scr.flags & DEBUGOUT )
		DrawStringMenuBar( "Read Config File !" );
	ReadConfigFile( config_file? config_file : CONFIGNAME );
	if( Scr.flags & DEBUGOUT ){
		DrawStringMenuBar( "Read Config File Success !" );
		XSynchronize(dpy,1);
	}
	XUngrabServer( dpy );
	if( !Scr.MenuLabelRoot )	CreateSimpleMenu();
	CreateMenuItems();
	for( Scr.iconAnchor = Scr.IconMenu.m_item;
		Scr.iconAnchor->next->next != NULL;
		Scr.iconAnchor = Scr.iconAnchor->next );
	RepaintAllWindows( StartWin );
	if( Scr.StartFunc ) DoStartFunc();

	XDestroyWindow (dpy, StartWin);

	sprintf( message, "Desk %d", Scr.currentdesk );
	DrawStringMenuBar( "" );
	ChangeDesk( message );
	Scr.flags &= ~STARTING;
	MapMenuBar( Scr.ActiveWin );

	while( True )		WaitEvents();
	return 0;
}
