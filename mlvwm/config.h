/****************************************************************************/
/* This module is all original code                                         */
/* by TakaC Hasegawa (tak@bioele.nuee.nagoya-u.ac.jp                        */
/* Copyright 1996, TakaC Hasegawa                                           */
/*     You may use this code for any purpose, as long as the original       */
/*     copyright remains in the source code and all documentation           */
/****************************************************************************/
#ifndef _CONFIG_
#define _CONFIG_

#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef struct styles
{
	char *name;
	unsigned long flags;
	int maxmizesize_w;
	int maxmizesize_h;
	int maxmizescale;
	Icon *miniicon;
	char *iconname;
	Menu *menubar;
	struct styles *next;
}styles;

typedef struct config_func
{
	char *label;
	void (*action)( char *, FILE * );
}config_func;

typedef struct style_func
{
	char *label;
	char *(*action)( styles *, char * );
}style_func;

struct configure
{
  char key;
  int  value;
};

extern void SetDeskTopNum( char *, FILE * );
extern void SetFollowToMouse( char *, FILE * );
extern void ReadConfigFile( char * );
extern void FreeStyles( void );

#endif
