/****************************************************************************/
/* This module is all original code                                         */
/* by TakaC Hasegawa (tak@bioele.nuee.nagoya-u.ac.jp                        */
/* Copyright 1996, TakaC Hasegawa                                           */
/*     You may use this code for any purpose, as long as the original       */
/*     copyright remains in the source code and all documentation           */
/****************************************************************************/
#ifndef _MENUS_
#define _MENUS_

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>

#define MENUB_H			25

/* Definitions of Menu Item */
#define STRGRAY  1 /* Draw String Black or Gray? */
#define ICONGRAY 2 /* Draw Icon Gray */
#define CHECKON  4 /* Draw Check Mark or Not? */
#define SELECTON 8 /* Can Select or Not? */
#define M_ALLSET (STRGRAY|CHECKON|SELECTON)

/* Definitions of ChangeMenuItemLabel mode */
#define M_COPY 0
#define M_AND 1
#define M_OR 2

/* Definitions the way of unmap menu */
#define UNMAP_LABEL  1
#define UNMAP_WINDOW 2
#define UNMAP_ALL    UNMAP_LABEL|UNMAP_WINDOW

typedef struct ShortCut
{
	struct ShortCut *next;
	KeyCode keycode;
	int mods;
	char *action;
}
ShortCut;

typedef struct MenuItem
{
	struct MenuItem *next;
	struct MenuLabel *submenu;
	Icon *xpm;
	char *iconname;
	char *label;
	char *action;
	int mode;
}MenuItem;

/* Definitions of Menu Label */
#define ACTIVE     1
#define LEFTSIDE   2
#define SWALLOW    4
#define CANDELETE  8
#define STICKLABEL 16

typedef struct MenuLabel
{
	struct MenuLabel *next;
	Window LabelWin;
	int LabelWidth;
	int LabelHeight;
	char *LabelStr;
	char *name;
	char *action; /* use for swallow */
	int flags;
	Icon *xpm;
	int LabelX; /* Position of Label */
	MenuItem *m_item;
	Window PullWin;
	int SelectNum;
	int MenuX;
	int MenuY;
	int MenuWidth;
	int MenuHeight;
	int ItemHeight;
	int IconWidth;
} MenuLabel;

typedef struct MenuLink
{
	struct MenuLink *next;
	MenuLabel *link;
} MenuLink;

typedef struct Menu
{
	struct Menu *next;
	char *name;
	MenuLink *link;
} Menu;

typedef struct
{
	char *label;
	char *(*action)( MenuLabel *, char * );
}menu_func;

typedef struct
{
	char *label;
	char *(*action)( char *, char **, int *, char **, MenuLabel ** );
}menu_item_func;

extern void RedrawMenu( MenuLabel *, Bool );
extern void RedrawMenuBar( void );
extern Bool isRect( int, int, int, int, int, int );
extern void press_menu( MenuLabel * );
extern void CreateMenuLabel( MenuLabel * );
extern void AddMenuItem(MenuLabel *, char *, char *, char *, Icon *, MenuLabel *, int);
extern void DelMenuItem( MenuLabel *, char * );
extern void CreateIconMenu( void );
extern void CreateMenuItems( void );
extern void SetMenu( char *, FILE * );
extern void SetMenuBar( char *, FILE * );
extern void SetSwallow( char *, FILE * );
extern void CreateMenuBar( void );
extern void MapMenuBar( MlvwmWindow * );
extern void FreeMenu( void );
extern void KeepOnTop( void );
extern void ChangeMenuLabel( MenuLabel *, char *, Icon * );
extern void ChangeMenuItemLabel( char *, char *, char *, char *, int, int );
extern void FreeShortCut( void );
extern void DrawStringMenuBar( char * );
extern void CreateSimpleMenu( void );
#endif
