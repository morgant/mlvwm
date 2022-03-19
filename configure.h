XCOMM Imakefile for MLVWM

XCOMM Do you want to use Locale?
XCOMM If you use Solaris 1 and your X11 didn't compile with X_LOCALE,
XCOMM sometime, you can't find multi-byte character.

#define HasLocale YES

XCOMM After version 6.0-current, the name of default rc file is changed.
XCOMM Do you want to use .desktop to rc file?

#define OldRCFile NO

XCOMM Do you want to install sample rc file to MlvwmLibDir?

#define InstallSampleRC NO

XCOMM The directory name of bindir and libdir.

XCOMM #define MlvwmBinDir /home/hase/bin
XCOMM #define MlvwmLibDir /home2/hase/bin/mlvwm-lib

XCOMM
XCOMM Cannot modify as follows:
XCOMM

#ifdef MlvwmBinDir
    MLVWMBINDIR = MlvwmBinDir
#else
    MLVWMBINDIR = $(BINDIR)
#endif

#ifdef MlvwmLibDir
    MLVWMLIBDIR = MlvwmLibDir
#else
    MLVWMLIBDIR = $(USRLIBDIR)/X11/mlvwm
#endif

#ifndef HasLocale
#define HasLocale NO
#endif

#ifndef OldRCFile
#define OldRCFile NO
#endif

#if HasLocale
    Locale_DEFINES = -DUSE_LOCALE
#if defined(FreeBSDArchitecture) && OSMajorVersion>1 && OSMinorVersion>1
    Locale_LIBRARIES = -lxpg4
#endif /* End FreeBSD */
#endif /* End HasLocale */

#if defined(SunArchitecture)
    CC = gcc
#endif

#if OldRCFile
    MLVWMRC = .desktop
#else
    MLVWMRC = .mlvwmrc
#endif

#if Compatible
    COMPATIBLE = -DCOMPATIBLE
#else
    COMPATIBLE =
#endif

XCOMM 
      CDEBUGFLAGS = -g -Wall -Wshadow
XCOMM            CDEBUGFLAGS = -O2 -Wall
XCOMM            CDEBUGFLAGS = -g -Wall
XCOMM            CDEBUGFLAGS = -g

ICONPATH = /usr/X11R6/include/pixmaps

MLVWMDEFINES = $(Locale_DEFINES) $(COMPATIBLE)
MLVWMLIBRARIES = $(Locale_LIBRARIES)
