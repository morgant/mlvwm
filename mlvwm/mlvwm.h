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

#ifndef _MLVWM_
#define _MLVWM_

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/xpm.h>

#include "sun_headers.h"

#ifndef max
#define max( x, y )		((x)>(y)? (x) : (y) )
#endif
#ifndef min
#define min( x, y )		((x)<(y)? (x) : (y) )
#endif

#include <sys/wait.h>
#if defined(SYSV) || defined(SVR4)
#define ReapChildren()  while ((waitpid(-1, NULL, WNOHANG)) > 0);
#else
#define ReapChildren()  while ((wait3(NULL, WNOHANG, NULL)) > 0);
#endif

#define TITLE_HEIGHT (Scr.flags&SYSTEM8?20:17)
#define BOXSIZE (Scr.flags&SYSTEM8?13:11)
#define SBAR_WH Scr.bar_width

extern Display *dpy;
extern XContext MlvwmContext;
extern XContext MenuContext;
extern XClassHint NoClass;

#define C_NO_CONTEXT			0x00000000
#define C_WINDOW				0x00000001
#define C_FRAME					0x00000002
#define C_TITLE					0x00000004
#define C_ICON					0x00000008
#define C_ROOT					0x00000010
#define C_CLOSE					0x00000020
#define C_MINMAX				0x00000040
#define C_RESIZE				0x00000080
#define C_SHADE					0x00000100
#define C_SBAR_UP				0x00000200
#define C_SBAR_DOWN				0x00000400
#define C_SBAR_H				0x00000800
#define C_SBAR_H_AN				0x00001000
#define C_SBAR_RIGHT			0x00002000
#define C_SBAR_LEFT				0x00004000
#define C_SBAR_V				0x00008000
#define C_SBAR_V_AN				0x00010000

typedef struct Icon
{
	Pixmap icon;
	Pixmap lighticon;
	Pixmap mask;
	int width, height;
	enum {PIXMAP,BITMAP} kind;
} Icon;

typedef struct MlvwmWindow
{
	struct MlvwmWindow *next;
	struct MlvwmWindow *prev;
	Window w;
	Window frame;
	Window Parent;
	Window title_w;
	Window scroll_h[4];
	Window scroll_v[4];
	Window close_b;
	Window minmax_b;
	Window resize_b;
	Window shade_b;
	Window transientfor;
	int wShaped;
	int frame_x, frame_y;
	int frame_w, frame_h;
	int size_w, size_h;
	char *name;
	XWindowAttributes attr;
	XSizeHints hints;
	XWMHints *wmhints;
	XClassHint class;
	Icon *miniicon;
	unsigned int flags;

	int old_bw;
	int diff_x, diff_y;
/* Use for Window Shade */
	int shade_h;
/* Use for Max(Min)imaize Operation */
	int orig_x, orig_y;
	int orig_w, orig_h;
/* Use for scroll bar */
	int win_x, win_y;
	int orig_win_x, orig_win_y;
	int win_w, win_h;
	int Desk;
} MlvwmWindow;

/*
   define flags
*/
#define ONTOP			0x00000001
#define TITLE			0x00000002
#define SBARH			0x00000004
#define SBARV			0x00000008
#define RESIZER			0x00000010
#define CLOSER			0x00000020
#define MINMAXR			0x00000040
#define SHADER          0x00000080
#define NOWINLIST		0x00000100
#define MAPPED			0x00000200
#define ICON			0x00000400
#define SHADE			0x00000800
#define TRANSIENT		0x00001000
#define VISIBLE			0x00002000
#define MAXIMAIZED		0x00004000
#define HIDED			0x00008000
#define STICKY			0x00010000
#define SKIPSELECT		0x00020000
#define SCROLL			0x00040000
#define NOFOCUS         0x00080000
#define DISPDESK		0x00100000
#define DoesWmTakeFocus         0x00200000
#define DoesWmDeleteWindow      0x00400000
#define STARTICONIC             0x00800000
#define NONTRANSIENTDECORATE    0x01000000

#define NORMALWIN		(TITLE | SBARH | SBARV | RESIZER | CLOSER | MINMAXR)

extern Atom _XA_MIT_PRIORITY_COLORS;
extern Atom _XA_WM_CHANGE_STATE;
extern Atom _XA_WM_STATE;
extern Atom _XA_WM_COLORMAP_WINDOWS;
extern Atom _XA_WM_PROTOCOLS;
extern Atom _XA_WM_TAKE_FOCUS;
extern Atom _XA_WM_SAVE_YOURSELF;
extern Atom _XA_WM_DELETE_WINDOW;
extern Atom _XA_WM_DESKTOP;

extern void Done( int, char * );
#ifdef USE_LOCALE
#define DEFAULTFS "-adobe-*-*-r-*-*-12-*-*-*-p-*-*-*,\
-*-*-*-r-*-*-12-*-*-*-*-*-*-*"
#else
#define DEFAULTFONT "-adobe-*-*-r-*-*-12-*-*-*-p-*-*-*"
#endif
#endif
