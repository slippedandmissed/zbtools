/*
 * module_49060c (Mohawk engine): preloading resources (reading them ahead
 * of use, a little at a time or on a thread)
 */

/* @flags -p -x- */

#include <stdlib.h>

#include "zoombinis.h"

/* Cancels resource `id`'s preloads. */
/* @zoombi32 0x0049060c */
short cancelPreloads(long id)
{
    ResourceMap *map;
    FileTableEntry *entry;
    PreloadRequest *request;

    if (!findEntry(id, &map, &entry))
        return setResourceError(0x28d5);
    lockHandle(id);
    if (lockFile(map->file, -1))
        return setResourceError(fileError());
    if (resourceCounts(map, resourceIndex(id))[1] > 0)
        while ((request = findPreload(map, id, 1)) != 0)
            if (disposePreload(request))
                goto done;
    setResourceError(0);
done:
    unlockFile(map->file);
    unlockHandle(id);
    return resources.error;
}

/* Runs preloads for up to `time` ms (0xffffffff: until all are done), going
   round the maps that have some; the number still pending. */
/* @zoombi32 0x004906b4 */
unsigned short servicePreloads(unsigned long time)
{
    unsigned long start;
    ResourceMap *map;

    if (time > 0 && resources.preloads > 0) {
        start = currentTimeMs();
        do {
            map = (ResourceMap *)handleData(resources.preloadMap);
            if (map->async && time != 0xffffffff)
                resources.preloadMap = map->nextPreload;
            else
                runMapPreloads((unsigned short)resources.preloadMap, time);
        } while (time == 0xffffffff && resources.preloads > 0
                 || currentTimeMs() - start < time && resources.syncPreloads > 0);
        if (resources.asyncPreloads > 0 && fn_46e605(resources.preloadThread) == 1)
            yieldThread(0);
    }
    return resources.preloads;
}

/* Runs a map's preloads for up to `time` ms (0xffffffff: all of them);
   whether any are left. */
/* Not exact: `map` and `time` swap registers (ebx and esi). */
/* @zoombi32 0x00490744 */
unsigned short runMapPreloads(long handle, unsigned long time)
{
    ResourceMap *map;
    unsigned long start;
    unsigned long elapsed;
    PreloadRequest *request;

    if ((map = resourceMap(handle)) == 0) {
        setResourceError(0x28d4);
        return 0xffff;
    }
    if (!time) {
        setResourceError(0);
        return map->preloads > 0;
    }
    if (map->preloads > 0) {
        start = currentTimeMs();
        lockHandle(handle);
        if (!map->async || time == 0xffffffff) {
            switch (lockFile(map->file, time)) {
            case 300:
                return setResourceError(300);
            case 0:
                seekPreloads(map, seekFile(map->file, 0, 1));
                for (elapsed = 0; map->preloads > 0
                                  && (time == 0xffffffff
                                      || (elapsed = currentTimeMs() - start) < time); ) {
                    request = map->preload;
                    if (stepPreload(request, time == 0xffffffff ? time : time - elapsed))
                        disposePreload(request);
                }
                unlockFile(map->file);
                break;
            }
        }
        unlockHandle(handle);
    }
    setResourceError(0);
    return map->preloads > 0;
}

/* Reads more of a preload, for up to `time` ms (0xffffffff: all of it), in
   chunks sized to the time left; whether it's finished. */
/* @zoombi32 0x00490862 */
unsigned short stepPreload(PreloadRequest *request, unsigned long time)
{
    PreloadRequest *checked;
    short handle;
    ResourceMap *map;
    unsigned long start;
    unsigned long chunk;
    unsigned long elapsed;
    short error;

    if ((checked = checkRequest(request)) == 0) {
        setResourceError(0x28d5);
        return 0xffff;
    }
    if (checked->finished || !time) {
        setResourceError(0);
        return checked->finished;
    }
    setResourceError(0);
    handle = checked->id;
    map = (ResourceMap *)lockHandle(handle);
    checked->busy++;
    elapsed = 0;
    start = currentTimeMs();
    if (!map->async || time == 0xffffffff) {
        switch (lockFile(map->file, time)) {
        case 300:
            return setResourceError(300);
        case 0:
            if (!checked->finished) {
                resources.preloadMap = handle;
                map->preload = checked;
                if (!checked->buffer
                    && (checked->buffer = (char *)callProvider(checked, PRELOAD_GET_BUFFER)) == 0)
                    checked->finished = 1;
                while (!checked->finished
                       && (time == 0xffffffff || time > (elapsed = currentTimeMs() - start))) {
                    unsigned long remaining = checked->length - checked->done;
                    if (time == 0xffffffff)
                        chunk = remaining;
                    else {
                        chunk = (time - elapsed) * 100;
                        chunk = (chunk + 0x800) & 0xfffff800;
                        chunk = chunk < remaining ? chunk : remaining;
                    }
                    if (readResourceBytes(checked->id, checked->buffer + checked->done, &chunk,
                                          checked->offset + checked->done)) {
                        error = resources.error;
                        checked->finished = 1;
                        callProvider(checked, PRELOAD_RELEASE_BUFFER);
                        checked->buffer = 0;
                        callProvider(checked, PRELOAD_FAILED);
                        resources.error = error;
                        break;
                    }
                    checked->done += chunk;
                    if ((checked->finished = checked->done == checked->length) != 0) {
                        callProvider(checked, PRELOAD_RELEASE_BUFFER);
                        checked->buffer = 0;
                        callProvider(checked, PRELOAD_DONE);
                        setResourceError(0);
                        break;
                    }
                }
            }
            unlockFile(map->file);
            break;
        }
    }
    checked->busy--;
    unlockHandle(handle);
    return checked->finished;
}

/* A preload of resource `id` in a map's ring (`idle`: one not finished and
   not being worked on). */
/* @zoombi32 0x00490a5e */
PreloadRequest *findPreload(ResourceMap *map, long id, short idle)
{
    FileTableEntry *entry = &map->fileTable.entries[resourceIndex(id) - 1];
    short after;
    PreloadRequest *request;
    PreloadRequest *next;

    if (map->preloads) {
        request = map->preload;
        after = 0;
        do {
            if (entry->offset > request->position) {
                after = 1;
                next = request->next;
                if (request->position > next->position)
                    break;
            } else if (entry->offset < request->position) {
                next = request->prev;
                if (after || request->position < next->position)
                    break;
            } else {
                next = request->prev;
                if (next->id != id || comparePreloads(next, request) > 0
                    || request->prev == map->preload) {
                    if (!idle)
                        return request;
                    while (request->id == id) {
                        if (!request->finished && !request->busy)
                            return request;
                        if (map->preloads == 1)
                            break;
                        request = request->next;
                    }
                    return 0;
                }
            }
        } while ((request = next) != map->preload);
    }
    return 0;
}

/* Finishes resource `id`'s preloads now. */
/* @zoombi32 0x00490b36 */
short finishPreloads(long id)
{
    ResourceMap *map;
    FileTableEntry *entry;
    PreloadRequest *request;

    if (!findEntry(id, &map, &entry))
        return setResourceError(0x28d5);
    lockHandle(id);
    if (lockFile(map->file, -1))
        return setResourceError(fileError());
    if (resourceCounts(map, resourceIndex(id))[1] > 0)
        while ((request = findPreload(map, id, 1)) != 0)
            if (stepPreload(request, 0xffffffff) == 0xffff || disposePreload(request))
                goto done;
    setResourceError(0);
done:
    unlockFile(map->file);
    unlockHandle(id);
    return resources.error;
}

/* Cancels a preload (unless it's finished) and, once nothing is working on
   it, takes it out of its map's ring (and the map out of the ring of maps,
   if that was its last) and frees it. */
/* @zoombi32 0x00490bef */
short disposePreload(PreloadRequest *request)
{
    ResourceMap *map;
    FileTableEntry *entry;
    PreloadRequest *checked;

    if ((checked = checkRequest(request)) == 0)
        return setResourceError(0x28d5);
    findEntry(checked->id, &map, &entry);
    lockHandle(checked->id);
    checked->busy++;
    if (!checked->finished) {
        if (lockFile(map->file, -1))
            return setResourceError(fileError());
        if (!checked->finished) {
            checked->finished = 1;
            if (checked->buffer) {
                callProvider(checked, PRELOAD_RELEASE_BUFFER);
                checked->buffer = 0;
            }
            callProvider(checked, PRELOAD_CANCELLED);
        }
        unlockFile(map->file);
    }
    if (checked->busy > 1) {
        checked->busy--;
        unlockHandle(checked->id);
        return setResourceError(0);
    }
    resources.preloads--;
    if (!map->async)
        resources.syncPreloads--;
    else if (!--resources.asyncPreloads && fn_46e5dc() != resources.preloadThread) {
        deleteThread(resources.preloadThread);
        resources.preloadThread = 0;
    }
    if (--map->preloads) {
        checked->next->prev = checked->prev;
        checked->prev->next = checked->next;
        if (checked == map->preload)
            map->preload = checked->next;
        if (checked == map->lastPreload)
            map->lastPreload = checked->next;
    } else {
        map->preload = 0;
        map->lastPreload = 0;
        ResourceMap *next = (ResourceMap *)handleData(map->nextPreload);
        ResourceMap *prev = (ResourceMap *)handleData(map->prevPreload);
        next->prevPreload = map->prevPreload;
        prev->nextPreload = map->nextPreload;
        map->nextPreload = 0;
        map->prevPreload = 0;
        if (resources.preloadMap == (short)checked->id)
            resources.preloadMap = prev->nextPreload;
    }
    resourceCounts(map, resourceIndex(checked->id))[1]--;
    unlockHandle(checked->id);
    checked->tag = 0;
    free(checked);
    return setResourceError(0);
}

/* Puts a preload into its map's ring, in order of position in the file
   (and the map into the ring of maps, if it's its first). */
/* @zoombi32 0x00490dd4 */
void insertPreload(ResourceMap *map, PreloadRequest *request)
{
    PreloadRequest *at;

    if (map->preloads++) {
        at = map->lastPreload;
        if (map->preloads > 1) {
            do {
                switch (comparePreloads(at, request)) {
                case -1:
                    at = at->next;
                    if (comparePreloads(at, at->prev) < 0)
                        goto insert;
                    break;
                case 0:
                    goto insert;
                case 1:
                    if (comparePreloads(at->prev, request) <= 0 || comparePreloads(at->prev, at) > 0)
                        goto insert;
                    at = at->prev;
                    break;
                }
            } while (at != map->lastPreload);
        }
    insert:
        request->next = at;
        request->prev = at->prev;
        at->prev->next = request;
        at->prev = request;
    } else {
        request->next = request;
        request->prev = request;
        map->preload = request;
        short handle = request->id;
        if (resources.preloadMap) {
            ResourceMap *ring = (ResourceMap *)handleData(resources.preloadMap);
            ResourceMap *last = (ResourceMap *)handleData(ring->prevPreload);
            map->nextPreload = last->nextPreload;
            last->nextPreload = handle;
            map->prevPreload = ring->prevPreload;
            ring->prevPreload = handle;
        } else {
            resources.preloadMap = handle;
            map->nextPreload = handle;
            map->prevPreload = handle;
        }
    }
    map->lastPreload = request;
    resources.preloads++;
    if (map->async)
        resources.asyncPreloads++;
    else
        resources.syncPreloads++;
}

/* Starts preloading resource `id`; `callback` hears when it's done (or
   failed). The request, or 0 if it finished at once. */
/* Not exact: the original stores the callback through a copy of `request` in eax
   (perhaps an inline helper); BCC32 4.5 stores through esi. */
/* @zoombi32 0x00490edf */
PreloadRequest *startPreload(long id, PreloadProc callback, long data)
{
    ResourceMap *map;
    FileTableEntry *entry;
    short error;
    PreloadRequest *request;

    if (!findEntry(id, &map, &entry)) {
        setResourceError(0x28d5);
        return 0;
    }
    lockHandle(id);
    if (entry->flags & RESOURCE_LOADED) {
    loaded:
        if (callback)
            callback(PRELOAD_DONE, id, &data);
        request = 0;
        setResourceError(0);
    } else if (!makeLong(entry->sizeLow, entry->sizeHigh)) {
        short handle = loadResource(id, 0);
        error = resources.error;
        if (callback)
            callback(handle ? PRELOAD_DONE : PRELOAD_FAILED, id, &data);
        request = 0;
        resources.error = error;
    } else if ((request = newPreloadRequest(id, (PreloadProc)defaultPreloadProc, 0,
                                            makeLong(entry->sizeLow, entry->sizeHigh), 0))
               != 0) {
        request->callback = callback;
        request->callbackData = data;
    } else if (!resources.error && entry->flags & RESOURCE_LOADED)
        goto loaded;
    unlockHandle(id);
    return request;
}

/* The provider startPreload uses: reads into a new handle, which becomes
   the resource's data. Its data is the handle, followed in the request by
   its caller's callback. */
/* @zoombi32 0x00490ff3 */
void *defaultPreloadProc(long event, long id, short *handle)
{
    ResourceMap *map;
    FileTableEntry *entry;
    PreloadRequest *request;
    short swapped;
    void *buffer;

    findEntry(id, &map, &entry);
    lockHandle(id);
    request = (PreloadRequest *)((char *)handle - 0x14);
    buffer = 0;
    switch (event) {
    case PRELOAD_GET_BUFFER:
        if (entry->flags & RESOURCE_LOADED) {
            if (request->callback)
                request->callback(PRELOAD_DONE, id, &request->callbackData);
            buffer = 0;
            break;
        }
        if ((*handle = newHandle(makeLong(entry->sizeLow, entry->sizeHigh))) != 0)
            buffer = lockHandle(*handle);
        break;
    case PRELOAD_RELEASE_BUFFER:
        unlockHandle(*handle);
        break;
    case PRELOAD_DONE:
        if (entry->flags & RESOURCE_LOADED)
            disposeResourceHandle(*handle);
        else {
            if (entry->flags & RESOURCE_PURGED) {
                swapped = swapHandleData(*handle, entry->handle);
                disposeResourceHandle(*handle);
                if (swapped) {
                    if (request->callback)
                        request->callback(PRELOAD_FAILED, id, &request->callbackData);
                    break;
                }
                *handle = entry->handle;
            }
            attachHandle(entry, *handle);
        }
        if (request->callback)
            request->callback(PRELOAD_DONE, id, &request->callbackData);
        break;
    case PRELOAD_CANCELLED:
    case PRELOAD_FAILED:
        if (*handle)
            disposeResourceHandle(*handle);
        if (request->callback)
            request->callback(event, id, &request->callbackData);
        break;
    }
    unlockHandle(id);
    return buffer;
}

/* A new preload of `length` bytes of resource `id` from `offset`, through
   `proc`; started at once (if the resource is loaded) or on the preload
   thread (for files that can be read in the background). */
/* @zoombi32 0x0049117e */
PreloadRequest *newPreloadRequest(long id, PreloadProc proc, long data, unsigned long length,
                                  unsigned long offset)
{
    ResourceMap *map;
    FileTableEntry *entry;
    PreloadRequest *request;

    if (!findEntry(id, &map, &entry)) {
        setResourceError(0x28d5);
        return 0;
    }
    lockHandle(id);
    request = 0;
    unsigned long size = makeLong(entry->sizeLow, entry->sizeHigh);
    if (offset > size || length + offset > size)
        setResourceError(0x28a0);
    else if ((request = (PreloadRequest *)malloc(0x38)) == 0)
        setResourceError(memError());
    else {
        memset(request, 0, 0x38);
        request->tag = 0x52515271;
        request->id = id;
        request->proc = proc;
        request->data = data;
        request->offset = offset;
        request->length = length;
        request->position = entry->offset;
        unsigned char *counts = resourceCounts(map, resourceIndex(id));
        if (counts[1] < 0xff)
            counts[1]++;
        else {
            /* (returning the freed request) */
            free(request);
            setResourceError(0x28d2);
            goto done;
        }
        insertPreload(map, request);
        if (entry->flags & RESOURCE_LOADED) {
            stepPreload(request, 0xffffffff);
            disposePreload(request);
            request = 0;
        } else if (map->async && !resources.preloadThread) {
            if ((resources.preloadThread = createThread(preloadThread, 0, 0x1000, 1)) != 0) {
                request->busy++;
                resumeThread(resources.preloadThread);
                yieldThread(0);
                request->busy--;
                if (request->finished) {
                    disposePreload(request);
                    request = 0;
                }
                setResourceError(0);
            } else {
                short error = fn_46e5ed();
                disposePreload(request);
                setResourceError(error);
                request = 0;
            }
        }
    }
done:
    unlockHandle(id);
    return request;
}

/* Moves a map's next preload to where the file's position is. */
/* Not exact: the original keeps `map` in ecx and `position` in esi, and the
   previous request's end in edx; BCC32 4.5 allocates them differently. */
/* @zoombi32 0x00491319 */
void seekPreloads(ResourceMap *map, unsigned long position)
{
    PreloadRequest *request;

    if (map->preloads > 1) {
        request = map->preload;
        if (position > request->position + request->done) {
            while (position > request->position + request->done) {
                request = request->next;
                if (request->prev->position > request->position || request == map->preload)
                    break;
            }
        } else if (position < request->position + request->done) {
            do {
                unsigned long end = request->prev->position + request->prev->done;
                if (position > end || end > request->position + request->done)
                    break;
                request = request->prev;
            } while (request != map->preload);
        }
        map->preload = request;
    }
}

/* The preload thread: runs preloads on files that can be read in the
   background while there are any. */
/* @zoombi32 0x0049138e */
void preloadThread(long)
{
    short handle;
    ResourceMap *map;
    PreloadRequest *request;

    while (resources.asyncPreloads) {
        fn_46eadd(resources.preloadThread, 2);
        handle = resources.preloadMap;
        map = (ResourceMap *)handleData(handle);
        if (!map->async || !map->preloads)
            resources.preloadMap = map->nextPreload;
        else {
            if (lockFile(map->file, -1))
                continue;
            map = (ResourceMap *)lockHandle(handle);
            if (map->preloads) {
                seekPreloads(map, seekFile(map->file, 0, 1));
                request = map->preload;
                stepPreload(request, 0xffffffff);
                map->preload = request->next;
                disposePreload(request);
            }
            unlockHandle(handle);
            unlockFile(map->file);
        }
        fn_46eadd(resources.preloadThread, 1);
        yieldThread(0);
    }
    long thread = resources.preloadThread;
    resources.preloadThread = 0;
    deleteThread(thread);
}

/* @zoombi32 0x00491461 */
unsigned char *resourceCounts(ResourceMap *map, unsigned short index)
{
    return (unsigned char *)handleData(map->counters) + index * 2 - 2;
}

/* Frees a resource's handle (it's in the resources' handle state, which
   disposeHandle refuses). */
/* @zoombi32 0x00491480 */
short disposeResourceHandle(short handle)
{
    unsigned short state;
    short error;

    if ((state = handleState(handle)) == 0xffff)
        error = memError();
    else if ((error = setHandleState(handle, 0)) == 0 && (error = disposeHandle(handle)) != 0)
        setHandleState(handle, state);
    return error;
}

/* The request, if it is one; else 0. */
/* @zoombi32 0x004914cd */
PreloadRequest *checkRequest(PreloadRequest *request)
{
    if (request && request->tag == 0x52515271)
        return request;
    return 0;
}

/* @zoombi32 0x004914e5 */
void *callProvider(PreloadRequest *request, long event)
{
    return request->proc(event, request->id, &request->data);
}

/* Orders preloads by position in the file, then offset, then longest first,
   the default provider's first. */
/* Not exact: the original keeps `order` in ecx to a single exit; BCC32 4.5
   returns from each branch in eax. */
/* @zoombi32 0x004914fc */
long comparePreloads(PreloadRequest *a, PreloadRequest *b)
{
    long order;

    if (a->position < b->position)
        order = -1;
    else if (a->position > b->position)
        order = 1;
    else if (a->offset < b->offset)
        order = -1;
    else if (a->offset > b->offset)
        order = 1;
    else if (a->length > b->length)
        order = -1;
    else if (a->length < b->length)
        order = 1;
    else if ((PreloadProc)defaultPreloadProc == a->proc && (PreloadProc)defaultPreloadProc == b->proc)
        order = 0;
    else if ((PreloadProc)defaultPreloadProc == a->proc)
        order = -1;
    else
        order = (PreloadProc)defaultPreloadProc == b->proc ? 1 : 0;
    return order;
}
