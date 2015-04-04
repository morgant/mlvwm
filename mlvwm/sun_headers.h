/**************************************************************************/
/* Prototypes that don't exist on suns */
/* If I do ALL this, I can compile OK with -Wall -Wstrict-prototypes on the
 * Sun's */
#ifndef _SUNHEADERS_
#define _SUNHEADERS_
#if defined(sun) && !defined(SVR4) && !defined(__NetBSD__)
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>


extern int select(int, fd_set *, fd_set *, fd_set *, struct timeval *);

extern int system(char *);

/* stdio things */
extern int sscanf(char *, char *, ...);
extern int printf(char *, ...);
extern int fprintf(FILE *,char *, ...);
extern int fclose(FILE *);

/* string manipulation */
extern int putenv(char *);
extern int fork( void );

/* sunOS defines SIG_IGN, but they get it wrong, as far as GCC
 * is concerned */
#ifdef SIG_IGN
#undef SIG_IGN
#endif
#define SIG_IGN         (void (*)(int))1
int wait3(int *, int, struct rusage *);
int sigsetmask(int);
int sigblock(int);
int bzero(char *, int);
  
int gethostname(char *name, int namelen);

/**************************************************************************/
#endif
#endif
