/*
 * Included before every decompiled source (-include) in the port's build:
 * makes clang read Borland C++ 4.5's dialect as BCC32 did.
 *
 * - The standard headers the sources use come first, with the host's
 *   packing; then everything after (the game's own headers) is packed to
 *   bytes, as BCC32 packs by default. The game's structures mirror its data
 *   files and each other's sizes, so their layout must be the original's.
 * - Borland's calling-convention and memory-model keywords mean nothing
 *   (miniwin/types.h).
 * - The Borland runtime's extras (itoa, stricmp, getdisk...) come from
 *   miniwin/borland.h, and fopen and getcwd/chdir take the game's Windows
 *   paths.
 */

#ifndef MINIWIN_PRELUDE_H
#define MINIWIN_PRELUDE_H

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* (C++'s own headers, which the port's code includes after this one, undefine
   macros such as fopen, and where long is redefined below would break: they
   must come first.) */
#if defined(__cplusplus) && __SIZEOF_LONG__ == 8
#include <deque>
#include <string>
#include <utility>
#include <vector>
#endif

#include <dir.h>
#include <dos.h>
#include <malloc.h>
#include <mmsystem.h>
#include <new.h>
#include <windows.h>

#include "miniwin/borland.h"
#include "miniwin/types.h"

/* The standard functions that miniwin replaces for the game's paths. */
#define fopen miniwin::fopen
#define getcwd miniwin::getcwd
#define chdir miniwin::chdir
#if __SIZEOF_LONG__ == 8
#define sprintf miniwin::bcSprintf
#define fprintf miniwin::bcFprintf
#define printf miniwin::bcPrintf
#define sscanf miniwin::bcSscanf
#define vsprintf miniwin::bcVsprintf
#endif

#pragma pack(1)

/* Where long is 64 bits (LP64: Linux and macOS), the game's longs stay what
   they are in Borland C++, 32 bits: its structures mirror its data files, and
   its arithmetic wraps there. Everything above (the system's headers, miniwin's)
   has been read already, so only the game's code sees this; where it needs a
   pointer-sized integer it says LONG_PTR (which was declared above too). The
   standard text functions that take longs by pointer or format get wrappers
   (miniwin/borland.cpp). */
#if __SIZEOF_LONG__ == 8
#define long int
#endif

#endif
