/*
 * e2memory's functions and globals: the declarations only its code and
 * its callers need (shared types and the rest are in zoombinis.h).
 */

#ifndef E2MEMORY_H
#define E2MEMORY_H

extern short freeAtOnce; /* 1: e2memory's frees free at once, else mark purgeable */
extern unsigned long memoryPeak; /* @data 0x4b99c4: e2memory's use, most and now */
extern unsigned long memoryInUse; /* @data 0x4b99c8 */
extern unsigned long memoryPeak2; /* @data 0x4b99cc */
extern unsigned long memoryInUse2; /* @data 0x4b99d0 */
/* e2memory's messages, joined texts kept until freed */
extern char *shapeText; /* @data 0x4b9adc */
extern char *arrayText; /* @data 0x4b9ae0 */
extern char *arrayErrorText; /* @data 0x4b9ae4 */
extern char *singleShapeText; /* @data 0x4b9ae8 */
extern char *resourceText; /* @data 0x4b9aec */
extern char *resourceErrorText; /* @data 0x4b9af0 */
extern char *readErrorText; /* @data 0x4b9af4 */
extern char *shapeListText; /* @data 0x4b9af8 */
extern char *soundListText; /* @data 0x4b9afc */
extern char *paletteText; /* @data 0x4b9b00 */
extern long pendingShapeList; /* @data 0x4b9b04 */
extern char dataPath[256]; /* @data 0x4b99d6 */
extern short dataPathLength; /* @data 0x4b9ad6 */
extern char dataDrive; /* @data 0x4b9ad8 */
void freeLoadTexts();
short usedResourceHandle(long);
/* Loads resource `id` of type `type` (e.g. 'CURS') into *handle. */
void loadResourceAs(long *resource, long type, unsigned short id, const char *what, short required);
/* Creates the font `name` at `size` into *font. */
void loadFont(Font **font, const char *name, unsigned short size, unsigned short style);
char *resourceData(long resource); /* a resource's data */
void freeResource(long *);
/* Finds resource `id` of type `type`; 0 if there's none. */
long findMapResource(long type, short id, short note);
void purgeGameResource(long *resource); /* releases a resource */
char *lockResource(long resource); /* locks a resource */
void unlockResource(long resource); /* unlocks it */
short *resourceShorts(long resource);
void loadShapeListInfo(long *info, short id, short *count, const char *name);
void freeShapeListInfo(long *resource);
void loadShapeMember(long *resource, unsigned short first, unsigned short member, const char *name);
void loadPalette(long *resource, short id, const char *name, short);
void freePalette(long *resource);
void loadSoundList(long *resource, short id, const char *name);
void freeSoundListNow(long *resource);
void loadShapeList(ResourceList **list, short id, const char *what, short); /* loads a resource list */
void freeShapeList(ResourceList **list);
/* e2memory */
long currentMap(); /* 0x46be28 */
void e2AllocHandle(short *handle, unsigned long size, char *what); /* 0x46bf01 */
void e2FreeHandle(short *handle); /* 0x46bf43 */
void e2DisposeHandle(short *handle); /* 0x46bf85 */
void e2AllocPtr(void **pointer, unsigned long size, char *what); /* 0x46bfa5 */
void e2FreePtr(void **pointer); /* 0x46bfe6 */
long loadListedShape(long list, short member, const char *what); /* 0x46c0ff */
void loadShape(long *resource, short id, char *what); /* 0x46c1a3 */
void allocShapeList(ResourceList **list, short id, short count, const char *what); /* 0x46c23d */
void disposeShapeList(ResourceList **list); /* 0x46c341 */
void keepShapeList(ResourceList **list, short id); /* 0x46c361 */
void loadSingleShape(ResourceList *list, short id, const char *what); /* 0x46c381 */
void freeSingleShape(ResourceList *list); /* 0x46c3cf */
void disposeSingleShape(ResourceList *list); /* 0x46c3e2 */
short loadGameResource(long resource); /* 0x46c434 */
short findAndLoad(long *resource, long type, short id); /* 0x46c4b5 */
void readGameResource(void *buffer, unsigned long size, long type, unsigned short id,
                      short exact, const char *what); /* 0x46c622 */
void releaseShapeListInfo(long *info); /* 0x46c76d */
void applyPaletteResource(unsigned short *data); /* 0x46c79c */
void releasePalette(long *resource); /* 0x46c85d */
void freeSoundList(long *resource); /* 0x46c910 */
void getDataPath(char *path); /* 0x46c9c2 */
void openGameFile(long *map, const char *name); /* 0x46c9e7 */
void freeFont(Font **font); /* 0x46cb61 */
unsigned short trackResource(long resource, short purgeable, short force); /* 0x46cb8d */
unsigned short trackHandle(short handle, short purgeable, short force); /* 0x46cbc6 */
void trackMemory(unsigned long size, short freed); /* 0x46cbff */
void setCurrentMap(long value);
short getFreeAtOnce();
short setFreeAtOnce(short value);
void setDataPath(const char *path);
void closeGameFile(long *handle);

#endif
