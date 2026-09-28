/*
 * e2memory (0x46be28-0x46cca0): the game's layer over the engine's memory and
 * resources ('e2AllocHandle error: memHandle already in use', 'e2GetShapes
 * error'): handles and pointers, resources (asking for the CD when it's
 * missing), shapes and lists of them, palettes, sound lists and fonts,
 * counting the memory they use
 */

#include <string.h>
#include "zoombinis.h"

char *shapeText;
char *arrayText;
char *arrayErrorText;
char *singleShapeText;
char *resourceText;
char *resourceErrorText;
char *readErrorText;
char *shapeListText;
char *soundListText;
char *paletteText;
long pendingShapeList;
unsigned long memoryPeak;
unsigned long memoryInUse;
unsigned long memoryPeak2;
unsigned long memoryInUse2;

/* The resource map searched for the game's resources. */
/* @zoombi32 0x0046be28 */
long currentMap()
{
    return g_4a7f58;
}

/* @zoombi32 0x0046be2e */
void fn_46be2e(long value)
{
    g_4a7f58 = value;
}

/* @zoombi32 0x0046be3d */
void fn_46be3d()
{
    freeText((void **)&shapeText);
    freeText((void **)&arrayText);
    freeText((void **)&arrayErrorText);
    freeText((void **)&singleShapeText);
    freeText((void **)&resourceText);
    freeText((void **)&resourceErrorText);
    freeText((void **)&readErrorText);
    freeText((void **)&shapeListText);
    freeText((void **)&soundListText);
    freeText((void **)&paletteText);
    fn_46c77c(&pendingShapeList);
}

/* A loaded resource's handle (a fatal error if it isn't loaded). */
/* @zoombi32 0x0046beac */
short fn_46beac(long resource)
{
    short handle = resourceHandle(resource);

    if (!handle || setResourcePurgeable(resource, 0) == 1)
        fatalError("attempted to use unloaded data");
    return handle;
}

/* @zoombi32 0x0046bee2 */
short fn_46bee2()
{
    return g_4b99d4;
}

/* Sets g_4b99d4, returning its old value. */
/* @zoombi32 0x0046bee9 */
short fn_46bee9(short value)
{
    short old = g_4b99d4;
    g_4b99d4 = value;
    return old;
}

/* @zoombi32 0x0046bf01 */
void e2AllocHandle(short *handle, unsigned long size, char *what)
{
    if (*handle)
        fatalError("e2AllocHandle error: memHandle already in use");
    if ((*handle = newHandle(size)) == 0)
        reportJoinedError(what);
    trackHandle(*handle, 0, 1);
}

/* Marks a handle purgeable (or, with g_4b99d4 at 1, disposes of it). */
/* @zoombi32 0x0046bf43 */
void e2FreeHandle(short *handle)
{
    if (*handle) {
        setHandleLocks(*handle, 0);
        trackHandle(*handle, 1, 0);
        if (g_4b99d4 == 1) {
            disposeHandle(*handle);
            *handle = 0;
        }
    }
}

/* @zoombi32 0x0046bf85 */
void e2DisposeHandle(short *handle)
{
    short saved = fn_46bee9(1);

    e2FreeHandle(handle);
    fn_46bee9(saved);
}

/* @zoombi32 0x0046bfa5 */
void e2AllocPtr(void **pointer, unsigned long size, char *what)
{
    if (*pointer)
        fatalError("e2AllocPtr error: memPtr already in use");
    if ((*pointer = newPtr(size)) == 0)
        reportJoinedError(what);
    trackMemory(ptrSize(*pointer), 0);
}

/* (Counts the memory freed as used, as e2AllocPtr does.) */
/* @zoombi32 0x0046bfe6 */
void e2FreePtr(void **pointer)
{
    if (*pointer) {
        trackMemory(ptrSize(*pointer), 0);
        disposePtr(*pointer);
        *pointer = 0;
    }
}

/*
 * Loads a list of shapes: the list resource ('tCNT', else 'SHPL' with its
 * palette), each shape, and the list's palette ('tPAL'); with `release`,
 * the list and palette resources are released once used.
 */
/* @zoombi32 0x0046c011 */
void fn_46c011(ResourceList **list, short id, const char *what, short release)
{
    short count;
    unsigned short first;

    loadingAnimation = 1;
    fn_46c6db(&pendingShapeList, id, &count, what);
    allocShapeList(list, id, count, what);
    (*list)->list = pendingShapeList;
    pendingShapeList = 0;
    loadingAnimation = 0;
    mainLoopEvents();
    first = swapShort(*(unsigned short *)fn_46cafb((*list)->list));
    if (release)
        releaseShapeListInfo(&(*list)->list);
    for (short i = 0; i < count; i++)
        fn_46c148(&(*list)->resources[i], first, i + 1, what);
    fn_46c808(&(*list)->palette, id, what, 0);
    if (release)
        releasePalette(&(*list)->palette);
}

/* One shape (from 1) of a list resource, loaded. */
/* @zoombi32 0x0046c0ff */
long loadListedShape(long list, short member, const char *what)
{
    long shape = 0;

    fn_46c148(&shape, swapShort(*(unsigned short *)fn_46cafb(list)), member, what);
    return shape;
}

/* Loads shape `member` (from 1) of a list whose first shape is `first`. */
/* Not exact: the original subtracts the 1 as `add bx, 0xffff`. */
/* @zoombi32 0x0046c148 */
void fn_46c148(long *resource, unsigned short first, unsigned short member, const char *name)
{
    char text[16];

    formatText(14, text, "shape #%d", member);
    joinText(&shapeText, name, text);
    loadShape(resource, member + first - 1, shapeText);
    freeText((void **)&shapeText);
}

/* Loads a shape ('tBMP', else 'SHAP') and decompresses it, trying up to
   four times. */
/* Not exact: the original keeps `resource` in esi and `error` in ebx; BCC32
   4.5 swaps them. */
/* @zoombi32 0x0046c1a3 */
void loadShape(long *resource, short id, char *what)
{
    short error;
    short tries;

    tries = 4;
    error = 1;
    g_4a4974 = 1;
    for (; error && tries; tries--) {
        fn_46c4fe(resource, RESOURCE_TYPE('t', 'B', 'M', 'P'), id, what, 0);
        if (!*resource)
            fn_46c4fe(resource, RESOURCE_TYPE('S', 'H', 'A', 'P'), id, what, 1);
        if ((error = decompressImage(fn_46beac(*resource))) == 0)
            error = getPortError();
        if (error) {
            fn_46c602(resource);
            if (tries == 1)
                reportJoinedError(what);
        }
    }
    g_4a4974 = 0;
}

/* @zoombi32 0x0046c23d */
void allocShapeList(ResourceList **list, short id, short count, const char *what)
{
    if (*list) {
        if (id != (*list)->id) {
            joinText(&arrayErrorText, what, "e2GetShapes error: e2ShapeArray already in use by ");
            reportJoinedError(arrayErrorText);
        }
    } else if (!allocateBlock((void **)list, (count - 1) * 4 + sizeof(ResourceList))) {
        joinText(&arrayText, what, "shape array");
        reportJoinedError(arrayText);
    }
    (*list)->id = id;
    (*list)->count = count;
    (*list)->palette = 0;
    for (short i = 0; i < count; i++)
        (*list)->resources[i] = 0;
}

/* @zoombi32 0x0046c2db */
void fn_46c2db(ResourceList **list)
{
    releaseShapeListInfo(&pendingShapeList);
    if (*list) {
        releasePalette(&(*list)->palette);
        releaseShapeListInfo(&(*list)->list);
        for (short i = 0; i < (*list)->count; i++)
            fn_46c5b7(&(*list)->resources[i]);
        if (g_4b99d4 == 1)
            freeAndClear((void **)list);
    }
}

/* @zoombi32 0x0046c341 */
void disposeShapeList(ResourceList **list)
{
    short saved = fn_46bee9(1);

    fn_46c2db(list);
    fn_46bee9(saved);
}

/* Disposes of a list unless it's list `id`. */
/* @zoombi32 0x0046c361 */
void keepShapeList(ResourceList **list, short id)
{
    if (*list && (*list)->id != id)
        disposeShapeList(list);
}

/* Loads a single shape as a list of one. */
/* @zoombi32 0x0046c381 */
void loadSingleShape(ResourceList *list, short id, const char *what)
{
    joinText(&singleShapeText, what, "shape");
    loadShape(&list->resources[0], id, singleShapeText);
    freeText((void **)&singleShapeText);
    list->count = 1;
    list->list = list->palette = 0;
    list->id = id;
}

/* @zoombi32 0x0046c3cf */
void freeSingleShape(ResourceList *list)
{
    fn_46c5b7(&list->resources[0]);
}

/* @zoombi32 0x0046c3e2 */
void disposeSingleShape(ResourceList *list)
{
    short saved = fn_46bee9(1);

    freeSingleShape(list);
    fn_46bee9(saved);
}

/* A resource of the current map (0 if there's none; then, with `note`,
   loadFailed is set). */
/* Not exact: the original keeps `resource` in ebx, BCC32 4.5 in eax. */
/* @zoombi32 0x0046c402 */
long fn_46c402(long type, short id, short note)
{
    long resource = findResource(type, id, g_4a7f58);

    if (!resource && note)
        loadFailed = 1;
    return resource;
}

/* Loads a resource, asking for the CD while it's missing. */
/* Not exact: the original keeps `resource` in ebx and `handle` in esi; BCC32
   4.5 swaps them. */
/* @zoombi32 0x0046c434 */
short loadGameResource(long resource)
{
    short handle = resourceHandle(resource);

    if (!handle) {
        short retry;

        fn_41585f();
        do {
            retry = 0;
            if ((handle = loadResource(resource, 0)) != 0) {
                trackResource(resource, 0, 1);
            } else if (resourceError() == 0x284c) {
                retry = 1;
                warning("Insert the %s CD into drive %c:", appName, dataDrive);
            }
        } while (retry);
        mainLoopEvents();
    } else {
        trackResource(resource, 0, 0);
    }
    return handle;
}

/* @zoombi32 0x0046c4b5 */
short findAndLoad(long *resource, long type, short id)
{
    short handle = 0;

    if ((*resource = fn_46c402(type, id, 1)) != 0) {
        if ((handle = loadGameResource(*resource)) == 0) {
            *resource = 0;
            if (!outOfMemory)
                loadFailed = 1;
        }
    }
    return handle;
}

/*
 * Loads resource `id` of a type into *resource; if it's missing, reports an
 * error when `required` (or when it failed to load). A resource already
 * there must be the same one.
 */
/* @zoombi32 0x0046c4fe */
void fn_46c4fe(long *resource, long type, unsigned short id, const char *what, short required)
{
    char text[20];
    long old = *resource;

    formatText(20, text, "id #%u", (unsigned short)id);
    joinText(&resourceText, what, text);
    if (!findAndLoad(resource, type, id) && (required || !loadFailed))
        reportJoinedError(resourceText);
    loadFailed = 0;
    if (old && *resource && old != *resource) {
        fn_46c602(&old);
        joinText(&resourceErrorText, resourceText, "e2GetRsrc error: resRef already in use by ");
        reportJoinedError(resourceErrorText);
    }
    freeText((void **)&resourceText);
}

/* Marks a resource purgeable (or, with g_4b99d4 at 1, releases it). */
/* @zoombi32 0x0046c5b7 */
void fn_46c5b7(long *resource)
{
    short handle;

    if (*resource) {
        if ((handle = resourceHandle(*resource)) != 0)
            setHandleLocks(handle, 0);
        trackResource(*resource, 1, 0);
        if (g_4b99d4 == 1) {
            releaseResource(*resource, 0);
            *resource = 0;
        }
    }
}

/* @zoombi32 0x0046c602 */
void fn_46c602(long *resource)
{
    short saved = fn_46bee9(1);

    fn_46c5b7(resource);
    fn_46bee9(saved);
}

/* Reads `size` bytes of a resource into `buffer` (all of them, with
   `exact`). */
/* @zoombi32 0x0046c622 */
void readGameResource(void *buffer, unsigned long size, long type, unsigned short id,
                      short exact, const char *what)
{
    char text[20];
    unsigned long read;
    long resource;

    formatText(20, text, "id #%u", id);
    joinText(&resourceText, what, text);
    if ((resource = findResource(type, id, g_4a7f58)) == 0) {
        loadFailed = 1;
        reportJoinedError(resourceText);
    }
    read = size;
    fn_41585f();
    readResourceBytes(resource, buffer, &read, 0);
    if (exact && size != read) {
        joinText(&readErrorText, "End of data reached for ", resourceText);
        reportJoinedError(readErrorText);
    }
    mainLoopEvents();
    freeText((void **)&resourceText);
}

/* Loads a shape list's resource ('tCNT', else 'SHPL', which carries a
   palette too) and its count. */
/* Not exact: the original swaps the count's bytes through edx, BCC32 4.5
   through esi. */
/* @zoombi32 0x0046c6db */
void fn_46c6db(long *info, short id, short *count, const char *name)
{
    joinText(&shapeListText, name, "shape list");
    fn_46c4fe(info, RESOURCE_TYPE('t', 'C', 'N', 'T'), id, shapeListText, 0);
    if (!*info) {
        fn_46c4fe(info, RESOURCE_TYPE('S', 'H', 'P', 'L'), id, shapeListText, 1);
        applyPaletteResource((unsigned short *)(fn_46cafb(*info) + 4));
    }
    *count = swapShort(((unsigned short *)fn_46cafb(*info))[1]);
    freeText((void **)&shapeListText);
}

/* @zoombi32 0x0046c76d */
void releaseShapeListInfo(long *info)
{
    fn_46c5b7(info);
}

/* @zoombi32 0x0046c77c */
void fn_46c77c(long *resource)
{
    short saved = fn_46bee9(1);

    releaseShapeListInfo(resource);
    fn_46bee9(saved);
}

/* Colours from a palette resource (big-endian first colour and count, then
   the entries), into g_4aa7e8. */
/* @zoombi32 0x0046c79c */
void applyPaletteResource(unsigned short *data)
{
    unsigned short first = swapShort(data[0]);
    unsigned short count = swapShort(data[1]);

    memcpy(&g_4aa7e8[first], data + 2, count * sizeof(PALETTEENTRY));
    brightenPalette(g_4aa7e8, first, count);
}

/* @zoombi32 0x0046c808 */
void fn_46c808(long *resource, short id, const char *name, short required)
{
    joinText(&paletteText, name, "palette");
    fn_46c4fe(resource, RESOURCE_TYPE('t', 'P', 'A', 'L'), id, paletteText, required);
    freeText((void **)&paletteText);
    if (*resource)
        applyPaletteResource((unsigned short *)fn_46cafb(*resource));
}

/* @zoombi32 0x0046c85d */
void releasePalette(long *resource)
{
    fn_46c5b7(resource);
}

/* @zoombi32 0x0046c86c */
void fn_46c86c(long *resource)
{
    short saved = fn_46bee9(1);

    releasePalette(resource);
    fn_46bee9(saved);
}

/* Loads a sound list ('SNDL': a count, then sound ids) and its sounds. */
/* Not exact (like freeSoundList): the original reads the count through ebx
   and keeps each key in a stack variable. */
/* @zoombi32 0x0046c88c */
void fn_46c88c(long *resource, short id, const char *name)
{
    short handle;
    short count;

    joinText(&soundListText, name, "sound list");
    fn_46c4fe(resource, RESOURCE_TYPE('S', 'N', 'D', 'L'), id, soundListText, 1);
    freeText((void **)&soundListText);
    handle = fn_46beac(*resource);
    count = *(short *)handleData(handle);
    for (short i = 0; i < count; i++) {
        short key = ((short *)handleData(handle))[1 + i];

        fn_411382(key, RESOURCE_TYPE(0, 'S', 'N', 'D'));
    }
}

/* Not exact: the original reads the count through ebx and keeps each key in a
   stack variable. */
/* @zoombi32 0x0046c910 */
void freeSoundList(long *resource)
{
    short key;
    short handle;
    short count;
    short i;

    if (*resource) {
        handle = fn_46beac(*resource);
        count = *(short *)handleData(handle);
        for (i = 0; i < count; i++) {
            key = ((short *)handleData(handle))[1 + i];
            unloadSound(key, RESOURCE_TYPE(0, 'S', 'N', 'D'));
        }
        fn_46c5b7(resource);
    }
}

/* @zoombi32 0x0046c970 */
void fn_46c970(long *resource)
{
    short saved = fn_46bee9(1);

    freeSoundList(resource);
    fn_46bee9(saved);
}

/* Sets the directory the game's data files are read from. */
/* @zoombi32 0x0046c990 */
void setDataPath(const char *path)
{
    strcpy(dataPath, path);
    dataDrive = dataPath[0];
    dataPathLength = strlen(dataPath);
}

/* @zoombi32 0x0046c9c2 */
void getDataPath(char *path)
{
    dataPath[dataPathLength] = 0;
    strcpy(path, dataPath);
}

/* Opens a resource file in the data directory (fatal if it can't). */
/* Not exact: the original turns the test into a boolean (`sete`) before
   destroying the temporary fileSpec. */
/* @zoombi32 0x0046c9e7 */
void openGameFile(long *map, const char *name)
{
    if (*map)
        fatalError("e2OpenMap error: (%s) resFile already in use", name);
    dataPath[dataPathLength] = 0;
    strcat(dataPath, name);
    fn_41585f();
    if ((*map = openResourceFile(dataPath, 0)) == 0) {
        resourceError();
        fatalError("unable to open %s", name);
    }
    mainLoopEvents();
}

/* @zoombi32 0x0046ca9c */
void fn_46ca9c(long *handle)
{
    if (*handle) {
        closeResourceFile(*handle, 0, 0);
        *handle = 0;
    }
}

/* @zoombi32 0x0046cabc */
char *fn_46cabc(long resource)
{
    return (char *)lockHandle(fn_46beac(resource));
}

/* @zoombi32 0x0046cad1 */
void fn_46cad1(long resource)
{
    unlockHandle(fn_46beac(resource));
}

/* @zoombi32 0x0046cae6 */
short *fn_46cae6(long resource)
{
    return (short *)fn_48ea00(fn_46beac(resource));
}

/* @zoombi32 0x0046cafb */
char *fn_46cafb(long resource)
{
    return (char *)handleData(fn_46beac(resource));
}

/* @zoombi32 0x0046cb10 */
void fn_46cb10(Font **font, const char *name, unsigned short size, unsigned short style)
{
    if (*font)
        fatalError("Font already in use: %s %d", name, size);
    if ((*font = newFont(name, size, style)) == 0)
        fatalError("unable to load %s %d", name, size);
}

/* @zoombi32 0x0046cb61 */
void freeFont(Font **font)
{
    if (*font) {
        if (getFont() == *font)
            setFont(0);
        disposeFont(*font);
        *font = 0;
    }
}

/* Sets a resource purgeable or not, counting its memory as freed or used
   when that changes (or with `force`). */
/* @zoombi32 0x0046cb8d */
unsigned short trackResource(long resource, short purgeable, short force)
{
    unsigned short old = setResourcePurgeable(resource, purgeable);

    if (force || purgeable ^ old)
        trackMemory(resourceSize(resource), purgeable);
    return old;
}

/* @zoombi32 0x0046cbc6 */
unsigned short trackHandle(short handle, short purgeable, short force)
{
    unsigned short old = setPurgeable(handle, purgeable);

    if (force || purgeable ^ old)
        trackMemory(handleSize(handle), purgeable);
    return old;
}

/* @zoombi32 0x0046cbff */
void trackMemory(unsigned long size, short freed)
{
    if (freed) {
        memoryInUse -= size;
        memoryInUse2 -= size;
    } else {
        memoryInUse += size;
        memoryInUse2 += size;
        if (memoryInUse > memoryPeak)
            memoryPeak = memoryInUse;
        if (memoryInUse2 > memoryPeak2)
            memoryPeak2 = memoryInUse2;
    }
}
