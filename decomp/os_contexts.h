/*
 * os_contexts's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef OS_CONTEXTS_H
#define OS_CONTEXTS_H

short initContext(Context *context, void (*proc)(LONG_PTR), LONG_PTR argument,
                  unsigned short stackSize); /* 0x46f5c0 */
short freeContext(Context *context); /* 0x46f68f */
void resumeContext(Context *context); /* 0x46f6c9 */
void abandonContext(Context *context); /* 0x46f6f9 */
void switchContext(Context *to, Context *save); /* 0x46f70e */
void switchStack(thread *to); /* 0x46f74f */
void recordReturn(Context *context, unsigned short depth); /* 0x46f771 */
short setThreadError(short error); /* 0x46f78e */

#endif
