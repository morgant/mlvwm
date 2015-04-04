/****************************************************************************/
/* This module is all original code                                         */
/* by TakaC Hasegawa (tak@bioele.nuee.nagoya-u.ac.jp                        */
/* Copyright 1996, TakaC Hasegawa                                           */
/*     You may use this code for any purpose, as long as the original       */
/*     copyright remains in the source code and all documentation           */
/****************************************************************************/
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>
#include "garbage.xpm"

Display *dpy;
Window win;
Pixmap pixmap;
Pixmap mask;
XpmAttributes attributes;
GC gc;

void HandleEvents()
{
	XEvent ev;

	XNextEvent( dpy, &ev );
	switch( ev.type ){
	  case Expose:
		printf("Copy\n");
		XCopyArea( dpy, pixmap, win, gc, 0, 0,
				  attributes.width, attributes.height, 0, 0 );
		break;
	  case ButtonPress:
		exit( 1 );
		break;
	}
}

void main()
{
	Window root;
	unsigned long valuemask;
	XSetWindowAttributes attr;

	dpy = XOpenDisplay( NULL );
	root = RootWindow( dpy, 0 );
	if( XpmCreatePixmapFromData( dpy, root, garbage_xpm,
							&pixmap, &mask, &attributes ) != XpmSuccess ){
		printf("Erroe\n");
		exit( -1 );
	}
/*	valuemask = CWBackPixmap | CWEventMask;*/
	valuemask = CWBackPixel | CWEventMask;
	attr.event_mask = ( ButtonPressMask | ExposureMask );
/*	attr.background_pixmap = pixmap;*/
	attr.background_pixel = WhitePixel( dpy, 0 );
	win = XCreateWindow( dpy, root, 0, 0, 30, 30, 1,
						CopyFromParent, InputOutput, CopyFromParent,
						valuemask, &attr );
	gc = DefaultGC( dpy, 0 );
	XMapWindow( dpy, win );
	XFlush( dpy );
	while( 1 )			HandleEvents();
}
