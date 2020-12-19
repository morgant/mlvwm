/****************************************************************************/
/* This module is all original code                                         */
/* by TakaC Hasegawa (tac.hasegawa@gmail.com)                               */
/* Copyright 1996, TakaC Hasegawa                                           */
/*     You may use this code for any purpose, as long as the original       */
/*     copyright remains in the source code and all documentation           */
/****************************************************************************/
#ifndef _MISC_
#define _MISC_
#include <unistd.h>

extern char *fgetline( char *, int, FILE * );
extern char *SkipNonSpace( char * );
extern char *LookUpFiles( char *, char *, int );
extern void sleep_a_little( int );
extern char *stripquote( char *, char ** );
extern char *SkipSpace( char * );
extern char *stripspace_num( char * );
extern Icon *ReadIcon( char *, Icon *, Bool );
extern void DrawErrMsgOnMenu( char *, char * );
extern void SetSegment( int, int, int, int, XSegment * );
extern void RaiseMlvwmWindow( MlvwmWindow * );
extern Pixel GetColor( char * );
extern char *WinListName( MlvwmWindow * );

extern void StrWidthHeight(
#ifdef USE_LOCALE
XFontSet ,
#else
XFontStruct *,
#endif
 int *, int *, int *, char *, int  );

#endif /* _MISC_ */
