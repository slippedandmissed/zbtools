/*
 * Included before every decompiled source (-include) in the port's build:
 * makes clang read Borland C++ 4.5's dialect as BCC32 did.
 *
 * - The standard headers the sources use come first, with the host's
 *   packing; then everything after (the game's own headers) is packed to
 *   bytes, as BCC32 packs by default. The game's structures mirror its data
 *   files and each other's sizes, so their layout must be the original's.
 * - <new> is left out: the game defines its own placement new (see new.h).
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

#include "miniwin/borland.h"
#include "miniwin/types.h"

/* The standard functions that miniwin replaces for the game's paths. */
#define fopen miniwin::fopen
#define getcwd miniwin::getcwd
#define chdir miniwin::chdir

#pragma pack(1)

#endif
