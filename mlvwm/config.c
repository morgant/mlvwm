/****************************************************************************/
/* This module is mostly all new                                            */
/* by TakaC Hasegawa (tac.hasegawa@gmail.com)                               */
/* Copyright 1996 TakaC Hasegawa. No restrictions are placed on this code,  */
/* as long as the copyright notice is preserved                             */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mlvwm.h"
#include "screen.h"
#include "menus.h"
#include "config.h"
#include "misc.h"

struct configure key_modifiers[]=
{
  {'S',ShiftMask},
  {'C',ControlMask},
  {'M',Mod1Mask},
  {'1',Mod1Mask},
  {'2',Mod2Mask},
  {'3',Mod3Mask},
  {'4',Mod4Mask},
  {'5',Mod5Mask},
  {'A',AnyModifier},
  {'N',0},
  {0,0}
};

char *NoTitleStyle( styles *tmp_style, char *str )
{
	tmp_style->flags &= ~TITLE;
	tmp_style->flags &= ~CLOSER;
	tmp_style->flags &= ~MINMAXR;

	return str+7;
}

char *NoHScrollStyle( styles *tmp_style, char *str )
{
	tmp_style->flags &= ~SBARH;
	if( !(tmp_style->flags&SBARV) )
		tmp_style->flags &= ~SCROLL;

	return str+7;
}

char *NoVScrollStyle( styles *tmp_style, char *str )
{
	tmp_style->flags &= ~SBARV;
	if( !(tmp_style->flags&SBARH) )
		tmp_style->flags &= ~SCROLL;

	return str+7;
}

char *NoResizeRStyle( styles *tmp_style, char *str )
{
	tmp_style->flags &= ~RESIZER;

	return str+9;
}

char *NoMinMaxRStyle( styles *tmp_style, char *str )
{
	tmp_style->flags &= ~MINMAXR;

	return str+9;
}

char *NoCloseRStyle( styles *tmp_style, char *str )
{
	tmp_style->flags &= ~CLOSER;

	return str+8;
}

char *NoShadeRStyle( styles *tmp_style, char *str )
{
	tmp_style->flags &= ~SHADER;

	return str+8;
}

char *NoWinListStyle( styles *tmp_style, char *str )
{
	tmp_style->flags |= NOWINLIST;

	return str+9;
}

char *SetOnTopStyle( styles *tmp_style, char *str )
{
	tmp_style->flags |= ONTOP;

	return str+9;
}

char *SetStickyStyle( styles *tmp_style, char *str )
{
	tmp_style->flags |= STICKY;

	return str+6;
}

char *SetSkipSelectStyle( styles *tmp_style, char *str )
{
	tmp_style->flags |= SKIPSELECT;

	return str+10;
}

char *SetScrollStyle( styles *tmp_style, char *str )
{
	if( tmp_style->flags & ( SBARV | SBARH ) )
		tmp_style->flags |= SCROLL;

	return str+12;
}

char *SetFocusStyle( styles *tmp_style, char *str )
{
	tmp_style->flags |= NOFOCUS;

	return str+7;
}


char *SetNormalStyle( styles *tmp_style, char *str )
{
	tmp_style->flags |= NORMALWIN;
	if( Scr.flags&SYSTEM8 )		tmp_style->flags |= SHADER;

	return str+16;
}

char *SetNoTransientDecorate( styles *tmp_style, char *str )
{
	tmp_style->flags |= NONTRANSIENTDECORATE;

	return str+20;
}

char *SetMaxmizeScale( styles *tmp_style, char *str )
{
	char *stop;

	if( (stop = strchr( str, ',' ))!=NULL )		*stop = '\0';
	if( strchr( SkipNonSpace(str), 'x' )){
		tmp_style->maxmizescale = 0;
		if( sscanf( str, "MaxmizeScale%dx%d",
			   &(tmp_style->maxmizesize_w), &(tmp_style->maxmizesize_h) )!=2 )
    	    DrawErrMsgOnMenu( "MaxmizeScale needs ", "width and height" );
	}
	else{
		if( sscanf( str, "MaxmizeScale%d", &(tmp_style->maxmizescale) )!=1 )
    	    DrawErrMsgOnMenu( "MaxmizeScale needs ", "size" );
	}
	if( stop )		stop++;
	else		stop = str+strlen(str);

	return stop;
}

char *SetMiniIcon( styles *tmp_style, char *str )
{
	char *stop, *name, *path;

	if( (stop = strchr( str, ',' ))!=NULL )
		*stop = '\0';
	name = calloc( strlen(str)-7, 1 );
	sscanf( str, "MiniIcon%s", name );

	tmp_style->iconname = name;
	if((path = LookUpFiles( Scr.IconPath, name, R_OK ))==NULL )
		DrawErrMsgOnMenu( "Can't Find file ", name );
	else
		free( path );

	if( stop )
		stop++;
	else
		stop = str+strlen(str);

	return stop;
}

char *SetMenuBarToWin( styles *tmp_style, char *str )
{
	char *stop, *name;
	Menu *tmp_m;

	if( (stop = strchr( str, ',' ))!=NULL )
		*stop = '\0';
	name = calloc( strlen(str)-7, 1 );
	sscanf( str, "MenuBar%s", name );

	for( tmp_m = Scr.MenuRoot; tmp_m && strcmp( name, tmp_m->name );
		tmp_m=tmp_m->next );
	tmp_style->menubar = tmp_m;
	if( !tmp_m )
		DrawErrMsgOnMenu( "Configuration Error(Style MenuBar) ", name );
	free( name );

	if( stop )		stop++;
	else		stop = str+strlen(str);

	return stop;
}

style_func style_config[]={
	{ "NormalDecoration", SetNormalStyle },
	{ "NoTitle", NoTitleStyle },
	{ "NoSBarH", NoHScrollStyle },
	{ "NoSBarV", NoVScrollStyle },
	{ "NoResizeR", NoResizeRStyle },
	{ "NoMinMaxR", NoMinMaxRStyle },
	{ "NoCloseR", NoCloseRStyle },
	{ "NoShadeR", NoShadeRStyle },
	{ "NoWinList", NoWinListStyle },
	{ "StayOnTop", SetOnTopStyle },
	{ "Sticky", SetStickyStyle },
	{ "SkipSelect", SetSkipSelectStyle },
	{ "EnableScroll", SetScrollStyle },
	{ "NoFocus", SetFocusStyle },
	{ "MaxmizeScale", SetMaxmizeScale},
	{ "MenuBar",	SetMenuBarToWin },
	{ "MiniIcon", SetMiniIcon},
	{ "NoTransientDecorate", SetNoTransientDecorate },
	{ NULL, 0}
};

void SetStyles( char *line, FILE *fp )
{
	char str[256], *top, *name;
	styles *tmp, *last, *check;
	int lp;

	for( last = Scr.style_list; last && last->next; last=last->next );
	while( fgetline( str, 256, fp )!=NULL && strncmp( str, "END", 3) ){
		if( str[0]=='#' )		continue;
		top = stripquote( str, &name );
		if( name==NULL ){
			DrawErrMsgOnMenu( "Configuration Error ", "SetStyle" );
			continue;
		}

		for( check=Scr.style_list; check && strcmp( check->name, name );
			check=check->next );

		if( check==NULL ){
			tmp = calloc( 1, sizeof( styles ) );
			tmp->name = name;
			tmp->flags = NORMALWIN;
			if( Scr.flags&SYSTEM8 )		tmp->flags |= SHADER;
			tmp->maxmizescale = 90;
			if( last == NULL )
				Scr.style_list = tmp;
			else
				last->next = tmp;
			last = tmp;
		}
		else{
			tmp = check;
			free( name );
		}

		while( *top!='\n' && *top!='\0' ){
			for( ; !isalpha( *top ) && *top!='\n' && *top!='\0'; top++ );
			if( *top=='\n' || *top=='\0' )		break;
			for( lp=0; style_config[lp].label!=NULL; lp++ ){
				if( !strncmp( top, style_config[lp].label,
							 strlen(style_config[lp].label) )){
					top=style_config[lp].action( tmp, top );
					break;
				}
			}
			if( !style_config[lp].label ){
				DrawErrMsgOnMenu( "Configuration Error(Style)! ", str );
				for( ; *top!=',' && *top!='\n' && *top!='\0'; top++ );
			}
		}
	}
}

void FreeStyles( void )
{
	styles *now, *next;

	now = Scr.style_list;
	for( ;now!=NULL; now = next ){
		next = now->next;
		free( now->name );
		if( now->miniicon ){
			XFreePixmap( dpy, now->miniicon->icon );
			XFreePixmap( dpy, now->miniicon->lighticon );
			if( now->miniicon->mask!=None )
				XFreePixmap( dpy, now->miniicon->mask );
			free( now->miniicon );
			now->miniicon = NULL;
			free( now->iconname );
		}
		free( now );
	}
}

void SetStartFunction( char *line, FILE *fp )
{
	char str[256], *top;
	ShortCut **new;

	if( (!strncmp( line, "InitFunction", 12 ) && Scr.Restarting) ||
	   (!strncmp( line, "RestartFunction", 15 ) && !Scr.Restarting)){
		while( fgetline( str, 256, fp )!=NULL && strncmp( str, "END", 3) );
		return;
	}

	new = &Scr.StartFunc;
	while( fgetline( str, 256, fp )!=NULL && strncmp( str, "END", 3) ){
		if( str[0]=='#' )		continue;
		*new = calloc( 1, sizeof( ShortCut ) );
		top = SkipSpace( str );
		(*new)->action = strdup( top );
		new = &(*new)->next;
	}
}

void SetShortCut( char *line, FILE *fp )
{
	char str[256], *top, *end;
	char keyname[256], modifiers[256], action[256];
	ShortCut *new;
	KeySym keysym;
	KeyCode keycode;
	int len, tag;

	while( fgetline( str, 256, fp )!=NULL && strncmp( str, "END", 3) ){
		if( str[0]=='#' )		continue;
		top = SkipSpace( str );
		end = SkipNonSpace( top );
		strncpy( keyname, top, end-top );
		keyname[end-top] = '\0';
		top = SkipSpace( end+1 );
		end = SkipNonSpace( top );
		strncpy( modifiers, top, end-top );
		modifiers[end-top] = '\0';
		top = SkipSpace( end+1 );
		strcpy( action, top );
		action[strlen(action)-1] = '\0';

		if ((keysym = XStringToKeysym(keyname)) == NoSymbol ||
			(keycode = XKeysymToKeycode(dpy, keysym)) == 0)
			continue;
		new = calloc( 1, sizeof( ShortCut ) );
		if( new==NULL ){
			fprintf( stderr, "Can not allocate memory for ShortCutKey\n" );
			continue;
		}
		new->next = Scr.ShortCutRoot;
		Scr.ShortCutRoot = new;
		len = 0;
		new->mods = 0;
		while( len<strlen( modifiers ) ){
			for( tag=0; key_modifiers[tag].key!=0; tag++ ){
				if( key_modifiers[tag].key==modifiers[len] )
					new->mods |= key_modifiers[tag].value;
			}
			len++;
		}
		new->keycode = keycode;
		new->action = strdup( action );
	}
}

void SetDeskTopNum( char *line, FILE *fp )
{
	int lp;
	char *label, *action;

	sscanf( line, "Desktopnum%d", &Scr.n_desktop );
	if( Scr.n_desktop<2 ){
		Scr.n_desktop = 1;
		return;
	}
	if( Scr.n_desktop>999 ){
		Scr.n_desktop = 999;
		DrawErrMsgOnMenu( "You must set Desktopnum", " less than 999." );
	}
	for( lp=0; lp<Scr.n_desktop; lp++ ){
		label = calloc( 16, 1 );
		action = calloc( 16, 1 );
		sprintf( label, "Desk %d", lp );
		sprintf( action, "Desk %d", lp );
		if( lp!=Scr.currentdesk )
			AddMenuItem( &(Scr.IconMenu), label, action,
						NULL, NULL, NULL, SELECTON );
		else
			AddMenuItem(&(Scr.IconMenu), label, action,
						NULL, NULL, NULL, CHECKON|SELECTON );
	}
	label = strdup( "\0" );
	action = strdup( "Nop" );
	AddMenuItem( &(Scr.IconMenu), label, action, NULL, NULL, NULL, STRGRAY );
	if( Scr.LastActive!=NULL )
		free( Scr.LastActive );
	Scr.LastActive = calloc( Scr.n_desktop, sizeof(MlvwmWindow));
}

void SetRstPrevState( char *line, FILE *fp )
{
	Scr.flags |= RSTPREVSTATE;
}

void SetShadeMap( char *line, FILE *fp )
{
	Scr.flags |= SHADEMAP;
}

void SetFollowToMouse( char *line, FILE *fp )
{
	Scr.flags |= FOLLOWTOMOUSE;
}

void SetSloppyFocus( char *line, FILE *fp )
{
	Scr.flags |= FOLLOWTOMOUSE;
	Scr.flags |= SLOPPYFOCUS;
}

void SetStickyHide( char *line, FILE *fp )
{
	Scr.flags |= STICKHIDE;
}

void SetStickyShade( char *line, FILE *fp )
{
	Scr.flags |= STICKSHADE;
}

void SetIconifyHide( char *line, FILE *fp )
{
	Scr.flags |= ICONIFYHIDE;
	Scr.flags &= ~ICONIFYSHADE;
}

void SetIconifyShade( char *line, FILE *fp )
{
	Scr.flags |= ICONIFYSHADE;
	Scr.flags &= ~ICONIFYHIDE;
}

void SetDoubleClickTime( char *line, FILE *fp )
{
	int click_time;

	sscanf( line, "DoubleClickTime%d", &click_time );
	Scr.double_click_time = click_time;
}

void SetBarWidth( char *line, FILE *fp )
{
	int bar_width;

	sscanf( line, "ScrollBarWidth%d", &bar_width );
	if( bar_width>0 )
		Scr.bar_width = bar_width;
}

void SetMenuFlush( char *line, FILE *fp )
{
	if( sscanf( line, "FlushMenu%d%d", &(Scr.flush_time), &(Scr.flush_times) )
		!=2 )
		DrawErrMsgOnMenu( "You must set FlushMenu", " length and times" );
	Scr.flush_time *= 1000;
}

void SetSystem8( char *line, FILE *fp )
{
	XGCValues gcv;
	XSetWindowAttributes attributes;
	unsigned long gcm;

	if( Scr.d_depth<2 )
		DrawErrMsgOnMenu( "Can't use option ", "System8" );
	else{
		Scr.flags |= SYSTEM8;
		gcm = GCForeground;
		if( !XGetGCValues( dpy, Scr.MenuBlueGC, gcm, &gcv ) ){
			fprintf( stderr, "Sorry Can not get GC Values MenuBlue\n");
			gcv.foreground = WhitePixel( dpy, Scr.screen );
		}
		attributes.background_pixel = gcv.foreground;
		gcm = CWBackPixel;
		XChangeWindowAttributes( dpy, Scr.MenuBar, gcm, &attributes );
	}
}

void SetZoomWait( char *line, FILE *fp )
{
	sscanf( line, "ZoomWait%d", &Scr.zoom_wait );
	Scr.zoom_wait *= 1000;
}

void SetRotate( char *line, FILE *fp )
{
	Scr.flags |= ROTATEDESK;
}

void SetOpaqueMove( char *line, FILE *fp )
{
	Scr.flags |= OPAQUEMOVE;
}

void SetOpaqueResize( char *line, FILE *fp )
{
	Scr.flags |= OPAQUERESIZE;
}

void SetOneClickMenu( char *line, FILE *fp )
{
	Scr.flags |= ONECLICKMENU;
}

void SetIconPath( char *line, FILE *fp )
{
	char *top;

	top = SkipSpace( line+8 );
	if( Scr.IconPath!=NULL ) free( Scr.IconPath );
	Scr.IconPath = calloc( strlen(top), 1 );
	strncpy( Scr.IconPath, top, strlen(top)-1 );
}

void SetDisplayDeskNum( char *line, FILE *fp )
{
	Scr.flags |= DISPDESK;
}

void SetIconMenuIcon( char *line, FILE *fp )
{
	char *iconname;

	iconname = calloc( strlen(line)-11, 1 );
	if( sscanf( line, "IconMenuIcon%s", iconname )!=1 ){
		DrawErrMsgOnMenu( "Fail Set Icon. ", line );
		free( iconname );
		return;
	}
	Scr.SystemIcon = ReadIcon( iconname, NULL, True );

	free( iconname );
}

void SetFont(
#ifdef USE_LOCALE
XFontSet *font,
#else
XFontStruct **font,
#endif
 char *fontname)
{
#ifdef USE_LOCALE
	char **miss, *def;
	int n_miss, lp;
	XFontSet newfont;
#else
	XFontStruct *newfont;
#endif

#ifdef USE_LOCALE
	newfont = XCreateFontSet( dpy, fontname, &miss, &n_miss, &def );
	if( n_miss>0 ){
		for( lp=0; lp<n_miss; lp++ )
			DrawErrMsgOnMenu( "Load miss font ", miss[lp] );
		XFreeStringList( miss );
	}
	if( newfont==NULL )
		DrawErrMsgOnMenu( "Can't load font ", fontname );
	else{
		XFreeFontSet( dpy, *font );
		*font =	newfont;
	}
#else
	if(( newfont = XLoadQueryFont( dpy, fontname )) == NULL )
		DrawErrMsgOnMenu( "Can't load font ", fontname );
	else{
		XFreeFont( dpy, *font );
		*font = newfont;
	}
#endif
}

void SetFontConfig( char *line, FILE *fp )
{
	char *fontname, *top;

	top = SkipSpace( SkipNonSpace(line) );
	fontname = strdup( top );
	fontname[strlen(fontname)-1] = '\0';

	if( !strncmp( line, "MenuBarFont", 11 ) )
		SetFont( &(MENUBARFONT), fontname );
	if( !strncmp( line, "MenuFont", 8 ) )
		SetFont( &(MENUFONT), fontname );
	if( !strncmp( line, "TitleBarFont", 12 ) )
		SetFont( &(WINDOWFONT), fontname );
	if( !strncmp( line, "BalloonFont", 11 ) )
		SetFont( &(BALLOONFONT), fontname );
	free( fontname );
}

void ReadNewConfigFile( char *line, FILE *fp )
{
	char *top, *config;

	top = SkipSpace( SkipNonSpace( line ) );
	config = strdup( top );
	if( config[strlen(config)-1]=='\n' )
		config[strlen(config)-1] = '\0';
	ReadConfigFile( config );
	free( config );
}

void SetCompatible( char *line, FILE *fp )
{
	char dots[] = { 3, 2 };
	XGCValues gcv;
	unsigned long gcm;

	gcm = GCForeground|GCLineStyle;

	gcv.foreground = WhitePixel( dpy, Scr.screen );
	gcv.line_style = LineOnOffDash;

	XChangeGC( dpy, Scr.RobberGC, gcm, &gcv );
	XSetDashes( dpy, Scr.RobberGC, 0, dots, 2 );

	Scr.flags |= COMPATIBLE;
}

void SetBalloonHelp( char *line, FILE *fp )
{
	char *top;
	char *iconname, *label, *action;
	Icon *BalloonIcon;
	MenuLabel **ml;

	top = stripquote( line, &Scr.BalloonOffStr );
	top = stripquote( top, &Scr.BalloonOnStr );

	if( Scr.BalloonOffStr==NULL || Scr.BalloonOnStr==NULL ){
		DrawErrMsgOnMenu( "Error Balloon Setting", line );
		Scr.BalloonOffStr = NULL;
		Scr.BalloonOnStr = NULL;
		return;
	}

	top = strchr( top, ',' );
	if( top==NULL || *top=='\n' || *top=='\0' ){
		DrawErrMsgOnMenu( "No Definition Icon(Balloon). ", line );
		return;
	}
	top = SkipSpace( top+1 );
	iconname = calloc( strlen(top), 1 );
	if( sscanf( top, "Icon%s", iconname )!=1 ){
		DrawErrMsgOnMenu( "Fail Set Icon(Balloon). ", top );
		free( iconname );
		return;
	}
	BalloonIcon = ReadIcon( iconname, NULL, True );
	if( iconname )	 free( iconname );
	
	if( !(Scr.MenuLabelRoot) )	ml=&Scr.MenuLabelRoot;
	else		for( ml=&(Scr.MenuLabelRoot); *ml; ml=&(*ml)->next );

	*ml = calloc( 1, sizeof( MenuLabel ) );
	(*ml)->flags = ACTIVE | STICKLABEL;
	(*ml)->xpm = BalloonIcon;
	(*ml)->name = strdup( "Balloon" );
	(*ml)->LabelStr = NULL;
	label = strdup( "About Balloon Help..." );
	AddMenuItem( *ml, label, NULL, NULL, NULL, NULL, STRGRAY );

	label = strdup( "\0" );
	action = strdup( "Nop" );
	AddMenuItem( *ml, label, action, NULL, NULL, NULL, STRGRAY );

	label = strdup( Scr.BalloonOffStr );
	action = strdup( "ToggleBalloon" );
	AddMenuItem( *ml, label, action, NULL, NULL, NULL, SELECTON );
}

void SetUseRootWin( char *line, FILE *fp )
{
	Scr.flags |= USEROOT;
	XSelectInput( dpy, Scr.Root,
				 PropertyChangeMask |
				 SubstructureRedirectMask | KeyPressMask |
				 SubstructureNotifyMask );
}

void SetEdgeResist( char *line, FILE *fp )
{
	if( sscanf( line, "EdgeResistance%d%d", &(Scr.resist_x), &(Scr.resist_y) )
		!=2 ){
		DrawErrMsgOnMenu( "EdgeResistance needs ", "x force and y force" );
		Scr.resist_x = 0;
		Scr.resist_y = 0;
	}
}

config_func main_config[]={
	{ "Desktopnum", SetDeskTopNum },
	{ "DoubleClickTime", SetDoubleClickTime },
	{ "DisplayDeskNumber", SetDisplayDeskNum },
    { "EdgeResistance",	SetEdgeResist },
	{ "FlushMenu", SetMenuFlush },
	{ "FollowToMouse", SetFollowToMouse },
	{ "Compatible", SetCompatible },
	{ "IconMenuIcon", SetIconMenuIcon },
	{ "IconPath", SetIconPath },
	{ "IconifyHide", SetIconifyHide },
	{ "IconifyShade", SetIconifyShade },
	{ "InitFunction", SetStartFunction },
	{ "RestartFunction", SetStartFunction },
	{ "MenuBarFont", SetFontConfig },
	{ "MenuBar", SetMenuBar },
	{ "MenuFont", SetFontConfig },
	{ "BalloonFont", SetFontConfig },
	{ "Menu", SetMenu},
	{ "Read", ReadNewConfigFile },
	{ "RestartPreviousState", SetRstPrevState },
	{ "ScrollBarWidth", SetBarWidth },
	{ "ShadeMap", SetShadeMap },
	{ "ShortCut", SetShortCut },
	{ "SloppyFocus", SetSloppyFocus },
	{ "StickyShade", SetStickyShade },
	{ "StickyHide", SetStickyHide },
	{ "Style",	SetStyles },
	{ "Swallow", SetSwallow },
	{ "System8", SetSystem8 },
	{ "OpaqueMove", SetOpaqueMove },
	{ "OpaqueResize", SetOpaqueResize },
	{ "OneClickMenu", SetOneClickMenu },
	{ "RotateDesk", SetRotate },
	{ "TitleBarFont", SetFontConfig },
	{ "UseBalloon", SetBalloonHelp },
	{ "UseRootWin", SetUseRootWin },
	{ "ZoomWait", SetZoomWait },
	{ NULL, 0 }
};

void ReadConfigFile( char *configfile )
{
	FILE *fp;
	char str[1024], *file, *cmp;
	int lp;
#ifdef MLVWMLIBDIR
	char *sysrc;
	size_t sysrc_size;
#endif

	if( (file = LookUpFiles( NULL, configfile, R_OK ))==NULL )
		file = LookUpFiles( getenv("HOME"), configfile, R_OK );
#ifdef MLVWMLIBDIR
	if( !file ){
		if( strcmp( configfile, CONFIGNAME) ){
			sysrc_size = strlen(MLVWMLIBDIR)+strlen(configfile)+2;
			sysrc = calloc( sysrc_size, 1 );
			snprintf( sysrc, sysrc_size, "%s/%s", MLVWMLIBDIR, configfile );
		}
		else{
			sysrc_size = strlen(MLVWMLIBDIR)+strlen(configfile)+9;
			sysrc = calloc( sysrc_size, 1 );
			snprintf( sysrc, sysrc_size, "%s/system%s", MLVWMLIBDIR, configfile );
		}
		file = LookUpFiles( NULL, sysrc, R_OK );
		free( sysrc );
	}
#endif
	if( file==NULL || (fp=fopen( file, "r" ))==NULL ){
		DrawErrMsgOnMenu( "Can't open your config file. ", configfile );
		if( file )	free( file );
		return;
	}
	while( fgetline( str, sizeof(str), fp )!=NULL ){
		if( Scr.flags & DEBUGOUT )
			DrawStringMenuBar( str );
		cmp = str;

		if( *cmp == '#' ) continue;
		cmp = SkipSpace( cmp );
		if( *cmp == '\n' )		continue;
		for( lp=0; main_config[lp].label!=NULL; lp++ ){
			if( !strncmp( cmp, main_config[lp].label,
						 strlen(main_config[lp].label) )){
				main_config[lp].action( str, fp );
				break;
			}
		}
		if( main_config[lp].label==NULL ){
			DrawErrMsgOnMenu( "Configuration Error!  ", str );
			continue;
		}
	}
	fclose( fp );
	free( file );

	return;
}
