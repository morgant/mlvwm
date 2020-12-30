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
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#include "mlvwm.h"
#include "menus.h"
#include "screen.h"

char *SkipSpace( char *str )
{
	for( ; (*str==' ' || *str=='\t'); str++ );
	return str;
}

char *fgetline( char *str, int size, FILE *fp )
{
	int readlen;
	char strtmp[256];

	if( fgets( str, size, fp )==NULL )
		return NULL;

	readlen=strlen(str)-2;

	while( str[readlen]=='\\'
		&& fgets( strtmp, sizeof(strtmp), fp)!=NULL ){
		str[readlen]='\0';
		snprintf( str, size, "%s%s", str, SkipSpace(strtmp) );
		readlen=strlen(str)-2;
	}
	return str;
}

char *LookUpFiles( char *path, char *filename, int mode )
{
	char *find, *separator;
	size_t find_size;

	if( !access( filename, mode ) ){
		find = strdup( filename );
		return find;
	}
	if( path==NULL )	return NULL;
	do{
		path=SkipSpace( path );
		find_size = strlen(path)+strlen(filename)+2;
		find = calloc( find_size, 1 );
		if( strchr( path, ':' )==NULL ){
			snprintf( find, find_size, "%s/%s", path, filename );
			path += strlen( path );
		}
		else{
			strcpy( find, path );
			separator = strchr( find, ':' );
			snprintf( separator, strlen(separator)+strlen(filename)+2, "/%s", filename );
			path = strchr( path, ':') + 1;
		}
		if( !access(find, mode ) ){
			return find;
		}
		free( find );
	}
	while( *path!='\0' && *path!='\n' );
	return NULL;
}

char *SkipNonSpace( char *str )
{
	for( ; *str!=' ' && *str!='\t' && *str; str++ );
	return str;
}

char *stripquote( char *str, char **copy )
{
	char *top, *last;
	int len;

	top = strchr( str, '"' );
	last = top!=NULL ? strchr( top+1, '"' ) : NULL;
	len = last-top;
	if( top==NULL || last==NULL ){
		*copy = NULL;
		return str+strlen(str)-1;
	}
	*copy = calloc( len, 1 );
	strncpy( *copy, top+1, len-1 );
	return last+1;
}

char *stripspace_num( char *str )
{
	char *action;

	for( ; (*str==' ' || *str=='\t' || isdigit(*str) ); str++ );
	action = calloc( strlen( str ), 1 );
	strncpy( action, str, strlen( str )-1 );
	return action;
}

void sleep_a_little(int n)
{
  struct timeval value;
  
  if (n <= 0)
    return;
  
  value.tv_usec = n % 1000000;
  value.tv_sec = n / 1000000;
  
  (void) select(1, 0, 0, 0, &value);
}

void DrawErrMsgOnMenu( char *str1, char *str2 )
{
	char *str;
	size_t str_size;
	static int call=0;
	int wait_s;

	call++;
	str_size = strlen(str1)+strlen(str2)+1;
	str = calloc( str_size, 1 );
	snprintf( str, str_size, "%s%s", str1, str2 );
	if( call<5 )	XBell( dpy, 30 );
	DrawStringMenuBar( str );
	wait_s = 3000000-call/5*500000;
	wait_s = wait_s>0? wait_s : 300000;
	sleep_a_little( wait_s );
	free( str );
}

Icon *ReadIcon( char *fn, Icon *icon, Bool err )
{
	XWindowAttributes root_attr;
	XpmAttributes attr;
	int x, y, xhot, yhot;
	Pixmap bitmap;
	Icon *newicon;
	char *path;

	if( icon==NULL )		newicon = (Icon *)calloc( 1, sizeof(Icon) );
	else		newicon = icon;
	if((path = LookUpFiles( Scr.IconPath, fn, R_OK ))==NULL ){
		if( err )
			DrawErrMsgOnMenu( "Can't Find file ", fn );
		return NULL;
	}
	XGetWindowAttributes(dpy,Scr.Root,&root_attr); 
	attr.colormap = root_attr.colormap;
	attr.closeness = 40000; /* Allow for "similar" colors */
	attr.valuemask = XpmSize | XpmReturnPixels | XpmColormap | XpmCloseness;
	if( XpmReadFileToPixmap( dpy, Scr.Root, path,
						   &newicon->icon, &newicon->mask,
						   &attr ) != XpmSuccess &&
       XReadBitmapFile( dpy, Scr.Root, path, &newicon->width, &newicon->height,
					   &bitmap, &xhot, &yhot) !=BitmapSuccess ){
 		if( err )
			DrawErrMsgOnMenu( "Can't Read ICON ", fn );
		if( icon==NULL )
			free( newicon );
		newicon = NULL;
	}
	else{
		if( newicon->icon ){
			newicon->width = attr.width;
			newicon->height = attr.height;
			newicon->kind = PIXMAP;
		}
		else{
			newicon->icon = XCreatePixmap( dpy, Scr.Root,
										  newicon->width, newicon->height,
										  Scr.d_depth );
			newicon->mask = XCreatePixmap( dpy, Scr.Root,
										  newicon->width, newicon->height,
										  Scr.d_depth );
			XCopyPlane( dpy, bitmap, newicon->icon, Scr.BlackGC, 0, 0,
					   newicon->width, newicon->height, 0, 0, 1 );
			XCopyPlane( dpy, bitmap, newicon->mask, Scr.WhiteGC, 0, 0,
					   newicon->width, newicon->height, 0, 0, 1 );

			newicon->kind = BITMAP;
			XFreePixmap( dpy, bitmap );
		}
		newicon->lighticon = XCreatePixmap( dpy, newicon->icon,
										   newicon->width, newicon->height,
										   Scr.d_depth );
		XCopyArea( dpy, newicon->icon, newicon->lighticon,
				  DefaultGC( dpy, Scr.screen ),
				  0, 0, attr.width, attr.height, 0, 0 );
		for( y=0; y<newicon->height; y++ )
			for( x=y%2; x<newicon->width; x+=2 )
				XDrawPoint( dpy, newicon->lighticon, Scr.WhiteGC, x, y );
	}
	if( path!=NULL )	free( path );

	XpmFreeAttributes( &attr );
	return newicon;
}

Pixel GetColor( char *name )
{
	XColor color;
	XWindowAttributes attributes;

	XGetWindowAttributes( dpy, Scr.Root, &attributes );
	color.pixel = 0;
	if( !XParseColor( dpy, attributes.colormap, name, &color )){
		fprintf( stderr, "Unknow color %s\n", name );
		exit( 1 );
	}
	else if( !XAllocColor( dpy, attributes.colormap, &color )){
		fprintf( stderr, "Can't allocate color %s\n", name );
		exit( 1 );
	}
	return color.pixel;
}
			
void SetSegment( int x1, int x2, int y1, int y2, XSegment *seg )
{
	seg->x1 = x1;
	seg->x2 = x2;
	seg->y1 = y1;
	seg->y2 = y2;
}

void RaiseMlvwmWindow( MlvwmWindow *win )
{
	MlvwmWindow *tmp, *lastwin=NULL;
	int count=4, set=3;
	Window *wins;

	for( tmp = Scr.MlvwmRoot.next; tmp!=NULL; tmp=tmp->next ){
		if( tmp->flags&TRANSIENT && tmp->transientfor==win->w && tmp!=win 
		   && !(tmp->flags&ONTOP) )
			count++;
		if( tmp->flags&ONTOP && tmp!=win )
			count++;
		if( tmp->next==NULL )
			lastwin = tmp;
	}
	wins = calloc( count, sizeof(Window) );
	wins[0] = Scr.MenuBar;
	wins[1] = Scr.lbCorner;
	wins[2] = Scr.rbCorner;

	for( tmp = Scr.MlvwmRoot.next; tmp!=NULL; tmp=tmp->next ){
		if( win->flags&TRANSIENT && win->transientfor==tmp->w
		   && tmp->flags&ONTOP ){
			wins[set++] = win->frame;
		}
		if( tmp!=win && tmp->flags&ONTOP )
			wins[set++] = tmp->frame;
	}
	for( tmp = lastwin; tmp!=&Scr.MlvwmRoot; tmp=tmp->prev )
		if( tmp->flags&TRANSIENT && tmp->transientfor==win->w && tmp!=win 
		   && !(tmp->flags&ONTOP))
			wins[set++] = tmp->frame;
	if( count!=set )
		wins[set++] = win->frame;

	XRaiseWindow( dpy, wins[0] );
	XRaiseWindow( dpy, wins[1] );
	XRaiseWindow( dpy, wins[2] );
	XRestackWindows( dpy, wins, set );

	free( wins );
}

char *WinListName( MlvwmWindow *mw )
{
	char *winname;
	size_t winname_size;

	winname_size = strlen( mw->name )+1+(Scr.flags&DISPDESK?3:0);
	winname = calloc( winname_size, 1 );
	if( Scr.flags&DISPDESK ){
		if( mw->flags&STICKY )
			snprintf( winname, winname_size, "S:%s", mw->name );
		else
			snprintf( winname, winname_size, "%d:%s", mw->Desk, mw->name );
	}
	else
		strcpy( winname, mw->name );

	return winname;
}

void StrWidthHeight(
#ifdef USE_LOCALE
XFontSet font,
#else
XFontStruct *font,
#endif
 int *width, int *height, int *offset, char *str, int length )
{
#ifdef USE_LOCALE
	XRectangle ink, logical;
#else
	int font_ascent, font_descent, direction;
	XCharStruct overall;
#endif

#ifdef USE_LOCALE
	XmbTextExtents( font, str, length, &ink, &logical );
	if( width  )			*width = logical.width;
	if( height )		*height = logical.height;
	if( offset )
		*offset = logical.height/2+logical.y;
#else
	XTextExtents( font, str, length,
				 &direction, &font_ascent, &font_descent, &overall );
	if( width )			*width = overall.width;
	if( height )		*height = font_ascent+font_descent;
	if( offset )
		*offset = (font_ascent+font_descent)/2-font_ascent;
#endif
}
