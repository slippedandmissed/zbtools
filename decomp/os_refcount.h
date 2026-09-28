/*
 * os_refcount's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef OS_REFCOUNT_H
#define OS_REFCOUNT_H

void __cdecl clearLock(DeferLock *lock); /* 0x46d7f8 */
long __cdecl enterLock(DeferLock *lock); /* 0x46d827 */
void __cdecl leaveLock(DeferLock *lock); /* 0x46d838 */
void __cdecl initLock(DeferLock *lock, short listed); /* 0x46d8af */
void __cdecl removeLock(DeferLock *lock); /* 0x46d8e8 */
void __cdecl deferCall(DeferLock *lock, Deferred *call); /* 0x46d91c */

#endif
