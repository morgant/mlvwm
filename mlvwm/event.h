/****************************************************************************/
/* This module is all original code                                         */
/* by TakaC Hasegawa (tac.hasegawa@gmail.com)                               */
/* Copyright 1996, TakaC Hasegawa                                           */
/*     You may use this code for any purpose, as long as the original       */
/*     copyright remains in the source code and all documentation           */
/****************************************************************************/
#ifndef _EVENT_
#define _EVENT_

extern void InstallWindowColormaps (MlvwmWindow * );
extern Bool GrabEvent( int );
extern void UnGrabEvent( void );
extern void RestoreWithdrawnLocation( MlvwmWindow *, Bool );
extern int GetContext( MlvwmWindow *, XEvent *, Window * );
extern void Destroy( MlvwmWindow * );
extern void HandleDestroy( XEvent * );
extern void handle_configure_request( XConfigureRequestEvent );
extern void MoveWindow( MlvwmWindow *, XEvent *, Bool );
extern void DisplayPush( Window );
extern void CloseWindow( MlvwmWindow *, XEvent * );
extern void DrawResizeFrame( int, int, int, int );
extern void ResizeWindow( MlvwmWindow *, XEvent *, Bool );
extern void MinMaxWindow( MlvwmWindow *, XEvent * );
extern void HandleMapRequest( Window );
extern void handle_button_press( XEvent * );
extern void handle_expose( XEvent * );
extern void HandleEnterNotify( XEvent * );
extern void HandleLeaveNotify( XEvent * );
extern void HandleShapeNotify ( XEvent * );
extern void HandleEvents( XEvent );
extern void send_clientmessage (Window, Atom, Time);
extern void WaitEvents( void );
extern MlvwmWindow *NextActiveWin( MlvwmWindow * );
extern void SetMapStateProp( MlvwmWindow *, int );
extern void GetWindowSizeHints( MlvwmWindow * );
#endif
