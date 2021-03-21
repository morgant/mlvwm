/****************************************************************************
 * This module is based on Twm, but has been siginificantly modified 
 * by Rob Nation (nation@rocket.sanders.lockheed.com)
 ****************************************************************************/
/*
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifndef _SCREEN_
#define _SCREEN_

#include "menus.h"
#include "config.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

#define DEFAULT 0
#define SYS 1
#define TITLE_CURSOR 2
#define MOVE 3
#define RESIZE 4
#define WAIT 5
#define MENU 6
#define SELECT 7
#define DESTROY 8
#define SBARH_CURSOR 9
#define SBARV_CURSOR 10
#define MINMAX_CURSOR 11
#define SHADER_UP_CURSOR 12
#define SHADER_DOWN_CURSOR 13
#define MAX_CURSORS 14

#define MAX_WINDOW_WIDTH 32767
#define MAX_WINDOW_HEIGHT 32767

typedef struct ScreenInfo
{
	int d_depth;
	int screen;
	int n_desktop;
	int currentdesk;
	int MyDisplayWidth;
	int MyDisplayHeight;
	int NumberOfScreens;
	MlvwmWindow MlvwmRoot;
	MenuLabel *MenuLabelRoot;
	MenuLabel IconMenu;
	Menu *MenuRoot;
	Menu *ActiveMenu;
	MenuItem *iconAnchor;
	Window Root;
	Window MenuBar;
	Window NoFocusWin;
	Window lbCorner;
	Window rbCorner;
	MlvwmWindow *ActiveWin;
	MlvwmWindow **LastActive;
	MlvwmWindow *PreviousActive;
	ShortCut *ShortCutRoot;
	int root_pushes;
	MlvwmWindow *pushed_window;
	Cursor MlvwmCursors[MAX_CURSORS];
	styles *style_list;
	GC RobberGC;
	GC BlackGC;
	GC WhiteGC;
	GC Gray1GC;
	GC Gray2GC;
	GC Gray3GC;
	GC Gray4GC;
	GC ScrollBlueGC;
	GC MenuBlueGC;
	GC MenuSelectBlueGC;
#ifdef USE_LOCALE
	XFontSet MenuBarFs;
	XFontSet MenuFs;
	XFontSet WindowFs;
	XFontSet BalloonFs;
#else
	XFontStruct *MenuBarFont;
	XFontStruct *MenuFont;
	XFontStruct *WindowFont;
	XFontStruct *BalloonFont;
#endif
	unsigned int flags;
	int double_click_time;
	int bar_width;
	int flush_time;
	int flush_times;
	int resist_x, resist_y;
	int zoom_wait;

	char *IconPath;

	Icon *SystemIcon;
	char ErrorFunc[30];

	Window Balloon;
	char   *BalloonOffStr;
	char   *BalloonOnStr;
	Pixmap mask;

	ShortCut *StartFunc;

	Bool Restarting;
} ScreenInfo;

#define FOLLOWTOMOUSE       0x00000001
#define SLOPPYFOCUS         0x00000002
#define STICKSHADE          0x00000004
#define STICKHIDE           0x00000008
#define SHADEMAP            0x00000010
#define ICONIFYHIDE         0x00000020
#define ICONIFYSHADE        0x00000040
#define COMPATIBLE          0x00000080 
#define BALLOONON           0x00000100
#define STARTING            0x00000200
#define SYSTEM8             0x00000400
#define RSTPREVSTATE        0x00000800
#define ROTATEDESK          0x00001000
#define USEROOT				0x00002000
#define OPAQUEMOVE			0x00004000
#define OPAQUERESIZE	 	0x00008000
#define ONECLICKMENU	 	0x00010000
#define DEBUGOUT            0x00020000
#define ROUNDEDCORNERS      0x00040000
#define SYSTEM6             0x00080000

#ifdef USE_LOCALE
#define MENUBARFONT Scr.MenuBarFs
#define MENUFONT    Scr.MenuFs
#define WINDOWFONT  Scr.WindowFs
#define BALLOONFONT Scr.BalloonFs
#define XDRAWSTRING XmbDrawString
#else
#define MENUBARFONT Scr.MenuBarFont
#define MENUFONT    Scr.MenuFont
#define WINDOWFONT  Scr.WindowFont
#define BALLOONFONT Scr.BalloonFont
#define XDRAWSTRING( d, w, f, g, x, y, s, l ) XSetFont( d, g, f->fid );\
                         XDrawString( d, w, g, x, y, s, l );
#endif

extern ScreenInfo Scr;
#endif
