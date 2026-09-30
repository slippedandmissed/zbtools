/* Borland's malloc.h: malloc and alloca (some hosts have no malloc.h). */

#ifndef MINIWIN_MALLOC_H
#define MINIWIN_MALLOC_H

#include <stdlib.h>
#if defined(_WIN32)
#include_next <malloc.h>
#else
#include <alloca.h>
#endif

#endif
