/****************************************************************************/
/* This module is all original code                                         */
/* by TakaC Hasegawa (tac.hasegawa@gmail.com)                               */
/* Copyright 1996, TakaC Hasegawa                                           */
/*     You may use this code for any purpose, as long as the original       */
/*     copyright remains in the source code and all documentation           */
/****************************************************************************/
#ifndef _BORDERS_
#define _BORDERS_

#define SHADOW_BOTTOM   1
#define SHADOW_LEFT     2
#define SHADOW_RIGHT    4
#define SHADOW_TOP      8
#define SHADOW_ALL      (SHADOW_BOTTOM|SHADOW_LEFT|SHADOW_RIGHT|SHADOW_TOP)

extern void SetShape( MlvwmWindow *, int );
extern void SetUpFrame( MlvwmWindow *, int, int, int, int, Bool );
extern void SetTitleBar( MlvwmWindow *, Bool );
extern void DrawArrow( Window, int, GC, GC );
extern void DrawSbarAnk( MlvwmWindow *, int, Bool );
extern void DrawSbarArrow( MlvwmWindow *, int, Bool );
extern void DrawSbarBar( MlvwmWindow *, int, Bool );
extern void DrawResizeBox( MlvwmWindow *, Bool );
extern void DrawAllDecorations( MlvwmWindow *, Bool );
extern void DrawFrameShadow( MlvwmWindow *, Bool );
extern void SetFocus( MlvwmWindow * );
extern void DrawShadowBox( int, int, int, int, Window, int, GC, GC, char );
extern void DrawMinMax( MlvwmWindow *, Bool );
extern void DrawCloseBox( MlvwmWindow *, Bool );
extern void DrawShadeR( MlvwmWindow *, Bool );
#endif
