/*
 * os_localmem's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef OS_LOCALMEM_H
#define OS_LOCALMEM_H

extern short localMemErrorCode; /* @data 0x4b9cf0 */
short localMemError();
void *localAlloc(unsigned short size); /* 0x46d95c */
void localFree(void *block); /* 0x46d998 */
void *localReAlloc(void *block, unsigned short size); /* 0x46d9cf */
void setLocalMemError(short value);
int isAlignedPointer(void *pointer);

#endif
