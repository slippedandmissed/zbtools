/*
 * memoryblocks (Mohawk engine): the memory manager's blocks and handle
 * table
 */

/* @flags -p -x- */

#include <string.h>
#include "zoombinis.h"

MemoryState heap;

/*
 * After an allocation failed: with 0x2777 (out of memory), purges handles'
 * blocks while that could free enough, then asks the grow-zone procedure.
 * Whether to try again (else sets the error).
 */
/* Not exact: the original's switch jumps past its case
   (`jne`); BCC32 4.5 jumps to it (`je`, `jmp`). */
/* @zoombi32 0x0048ea14 */
short recoverMemory(short error, unsigned long size)
{
    unsigned long available;
    unsigned long needed;

    switch (error) {
    case 0x2777:
        do {
            available = availableMemory(size);
            if (available >= size)
                return 1;
            needed = size - available;
        } while (needed <= purgeMemory(needed, 0));
    }
    if (growZone(size, error))
        return 1;
    setMemError(error);
    return 0;
}

/*
 * A new block: a moveable global block for a handle, else a fixed one
 * holding its own master pointer. 0 on error.
 *
 * A moveable block's master pointer is the Windows handle itself: Win32's
 * GlobalAlloc handles point at a pointer to the memory, which the engine
 * relies on.
 */
/* @zoombi32 0x0048ea6b */
Block allocBlock(unsigned long size, unsigned short moveable)
{
    Block block;
    Chunk *chunk;

    do {
        if (moveable)
            block = (Block)GlobalAlloc(GMEM_MOVEABLE, size + sizeof(Chunk));
        else
            block = (Block)GlobalAlloc(GMEM_FIXED, size + sizeof(Chunk) + sizeof(Chunk *));
        if (!block && !recoverMemory(0x2777, size))
            return 0;
    } while (!block);
    if (moveable)
        chunk = *block;
    else
        *block = chunk = (Chunk *)(block + 1);
    memset(chunk, 0, sizeof(Chunk));
    chunk->magic = 0x4d42; /* 'BM' */
    chunk->size = size;
    chunk->moveable = moveable;
    afterBlockChange(block);
    setMemError(0);
    return block;
}

/* Frees a block, emptying its handle. */
/* Not exact: the original keeps `entry` in eax; BCC32 4.5 gives it
   edi. */
/* @zoombi32 0x0048eb12 */
short freeBlock(Block block)
{
    Chunk *chunk;
    HandleEntry *entry;

    beforeBlockChange(block);
    chunk = *block;
    chunk->magic = 0;
    if (chunk->moveable && chunk->handle) {
        if ((entry = handleEntry(chunk->handle))->locks > 0)
            GlobalUnlock(entry->block);
        handleEntry(chunk->handle)->block = 0;
    }
    GlobalFree(block);
    return setMemError(0);
}

/* Resizes a block (which may move); 0 on error. */
/* Not exact: the original keeps `moveable` in edi and `size` on
   the stack; BCC32 4.5 does the opposite. */
/* @zoombi32 0x0048eb7a */
Block resizeBlock(Block block, unsigned long size)
{
    register unsigned short moveable;
    Block resized;
    Chunk *chunk;

    beforeBlockChange(block);
    moveable = (*block)->moveable;
    do {
        resized = (Block)GlobalReAlloc(block, (moveable ? 0 : sizeof(Chunk *)) + size + sizeof(Chunk),
                                       GMEM_MOVEABLE);
        if (!resized && !recoverMemory(0x2777, size)) {
            afterBlockChange(block);
            return 0;
        }
    } while (!resized);
    block = resized;
    if (moveable) {
        chunk = *block;
        handleEntry(chunk->handle)->block = block;
    } else
        *block = chunk = (Chunk *)(block + 1);
    chunk->size = size;
    afterBlockChange(block);
    setMemError(0);
    return block;
}

/* Resizes a fixed block without moving it; whether it could. */
/* @zoombi32 0x0048ec1e */
short resizeFixedBlock(Chunk *chunk, unsigned long size)
{
    if (!GlobalReAlloc(blockOf(chunk), size + sizeof(Chunk) + sizeof(Chunk *), 0))
        return 0;
    chunk->size = size;
    return 1;
}

/* The master pointer of a block's memory. */
/* @zoombi32 0x0048ec60 */
Block blockOf(Chunk *chunk)
{
    return chunk->moveable ? (Block)GlobalHandle(chunk) : (Block)chunk - 1;
}

/* Starts the memory manager with a table of `handles` handles (at least
   256). `size`, if given, may be at most 1 MB. */
/* @zoombi32 0x0048ec85 */
short initMemory(unsigned long size, unsigned short handles)
{
    if (size > 0 && size > 0x100000)
        return setMemError(0x2777);
    memset(&heap, 0, sizeof(heap));
    heap.purgeEnabled = 1;
    heap.table = (HandleTable *)GlobalAlloc(GMEM_FIXED, 4);
    if (heap.table) {
        heap.table->freeList = 0;
        heap.table->count = 0;
        if (handles <= 0x100)
            handles = 0x100;
        if (growHandleTable(handles)) {
            GlobalFree(heap.table);
            return heap.error;
        }
    } else
        return setMemError(0x2777);
    heap.ready = 1;
    return setMemError(0);
}

/* Adds `count` entries to the handle table (up to 65535 in all). */
/* Not exact: the original caches `heap`'s address in esi and
   keeps `count` in edi; BCC32 4.5 swaps them. */
/* @zoombi32 0x0048ed24 */
short growHandleTable(unsigned short count)
{
    HandleTable *table;
    short error;
    unsigned short free;
    long i;

    if (heap.table->count + count <= 0xffff || (count = 0xffff - heap.table->count) != 0) {
        for (;;) {
            table = (HandleTable *)GlobalReAlloc(
                heap.table, (heap.table->count + count) * sizeof(HandleEntry) + 4, GMEM_MOVEABLE);
            if (table) {
                heap.table = table;
                break;
            }
            if (purgeMemory(count * sizeof(HandleEntry), 0))
                continue;
            error = 0x2777;
            if (growZone(count * sizeof(HandleEntry), error))
                continue;
            return setMemError(error);
        }
        free = heap.table->freeList;
        for (i = heap.table->count + count; i-- > heap.table->count;) {
            heap.table->entries[i].used = 0;
            heap.table->entries[i].nextFree = free;
            free = entryIndex(&heap.table->entries[i]);
        }
        heap.table->freeList = free;
        heap.table->count += count;
        return setMemError(0);
    }
    return setMemError(0x2776);
}

/* Whether a handle's block may be purged: the purge procedure agrees and
   it's in use, purgeable, unlocked and not kept. */
/* @zoombi32 0x0048ee1b */
short canPurge(HandleEntry *entry, short purpose)
{
    if (heap.purgeProc && entry->block)
        return heap.purgeProc(entryIndex(entry), purpose) && entry->used && entry->block
               && entry->purgeable && !entry->locks && !entry->keep;
    return 1;
}

/* How big a buffer the memory manager needs (0 before initMemory). */
/* @zoombi32 0x0048ee96 */
short memoryBufferSize()
{
    return heap.ready ? 0x500 : 0;
}

/* @zoombi32 0x0048eeaa */
void closeMemory()
{
    if (!heap.ready) {
        setMemError(0x27a6);
        return;
    }
    GlobalFree(heap.table);
    heap.ready = 0;
}

/* Asks the grow-zone procedure for memory; whether to try again. */
/* @zoombi32 0x0048eed9 */
short growZone(unsigned long size, short error)
{
    return heap.growProc ? heap.growProc(size, error) : 0;
}

/* @zoombi32 0x0048eefb */
unsigned short entryIndex(HandleEntry *entry)
{
    return (unsigned long)((char *)entry - (char *)heap.table->entries) / sizeof(HandleEntry) + 1;
}

/* Called before changing a block; does nothing. */
/* @zoombi32 0x0048ef15 */
void beforeBlockChange(Block)
{
}

/* Called after changing a block; does nothing. */
/* @zoombi32 0x0048ef1c */
void afterBlockChange(Block)
{
}
