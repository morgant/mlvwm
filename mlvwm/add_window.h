/****************************************************************************
 * This module is all original code 
 * by TakaC Hasegawa (tac.hasegawa@gmail.com)
 * Copyright 1996, TakaC Hasegawa
 *     You may use this code for any purpose, as long as the original
 *     copyright remains in the source code and all documentation
 ****************************************************************************/
#ifndef _ADD_WINDOW_
#define _ADD_WINDOW_

extern void FetchWmProtocols( MlvwmWindow * );
extern styles *lookupstyles( char *, XClassHint * );
extern void create_resizebox( MlvwmWindow * );
extern void create_scrollbar( MlvwmWindow * );
extern void create_titlebar( MlvwmWindow * );
extern MlvwmWindow *AddWindow( Window );
extern void GetWindowSizeHints( MlvwmWindow * );

extern char NoName[];
#endif
