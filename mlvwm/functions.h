/****************************************************************************/
/* This module is all original code                                         */
/* by TakaC Hasegawa (tac.hasegawa@gmail.com)                               */
/* Copyright 1996, TakaC Hasegawa                                           */
/*     You may use this code for any purpose, as long as the original       */
/*     copyright remains in the source code and all documentation           */
/****************************************************************************/
#ifndef _FUNCTION_
#define _FUNCTION_

typedef struct builtin_func
{
	char *label;
	void (*action)();
} builtin_func;

extern void ShadeWindow( MlvwmWindow * );
extern void UnShadeWindow( MlvwmWindow * );
extern void HideWindow( MlvwmWindow * );
extern void NopFunction( char * );
extern void AboutThisMachine( char * );
extern void RefreshScreen( char * );
extern void ChangeDesk( char * );
extern void SelectWindow( char * );
extern void ExecSystems( char * );
extern void RestartSystem( char * );
extern void ExitSystem( char * );
extern void ExecuteFunction( char * );
#endif
