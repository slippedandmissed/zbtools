/*
 * purgememory (Mohawk engine): purgeMemory
 */

/* @flags -p -x- */

#include "zoombinis.h"

/*
 * Frees purgeable handles' blocks, least recently used first, until
 * `needed` bytes are free; how many were. With `needed` 0, how many could
 * be.
 *
 * Each pass lowers every purgeable entry's age by the lowest age seen in
 * the pass before, and purges those reaching 0 (if canPurge agrees).
 */
/* Not exact: the original keeps the first loop's index and entry
   in edx and eax (it calls nothing); BCC32 4.5 uses saved registers. */
/* @zoombi32 0x0048efbc */
unsigned long purgeMemory(unsigned long needed, short purpose)
{
    unsigned short next;
    unsigned short lowest;
    int i;
    HandleEntry *entry;
    unsigned long freed;
    int j;

    if (!needed) {
        for (i = 0; i < heap.table->count; i++) {
            entry = &heap.table->entries[i];
            if (entry->used && entry->block && entry->purgeable && !entry->locks && !entry->keep)
                needed += (*entry->block)->size;
        }
        return needed;
    }
    freed = 0;
    next = 0;
    do {
        lowest = next;
        next = 16;
        for (j = 0; j < heap.table->count && freed < needed; j++) {
            if (!heap.purgeEnabled)
                return freed;
            entry = &heap.table->entries[j];
            if (entry->used && entry->age >= lowest && entry->block && entry->purgeable
                && !entry->locks && !entry->keep) {
                entry->age -= lowest;
                if (entry->age) {
                    if (entry->age < next)
                        next = entry->age;
                } else if (canPurge(entry, purpose) && entry->used && entry->block
                           && entry->purgeable && !entry->locks && !entry->keep) {
                    freed += (*entry->block)->size;
                    freeBlock(entry->block);
                } else
                    entry->age = 0;
            }
        }
    } while (freed < needed && next <= 15);
    return freed;
}
