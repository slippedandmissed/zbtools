/*
 * Declarations shared by the decompiled modules: the game's globals, the types
 * worked out so far, and every decompiled function, so any module can use them.
 * Names follow CLAUDE.md: by address until their purpose is known. Types are
 * inferred from how the code uses them and grow as more of it is decompiled.
 */

#ifndef ZOOMBINIS_H
#define ZOOMBINIS_H

#include <windows.h>
#include <time.h>

/* Types */

/* A Mohawk resource type, built Mac-style from its four characters (so
   RESOURCE_TYPE('C','U','R','S') is 0x43555253; C++'s multi-character
   constants put the bytes the other way round in Borland C++). */
#define RESOURCE_TYPE(a, b, c, d) (((long)(a) << 24) | ((long)(b) << 16) | ((long)(c) << 8) | (d))

/* A function registered to be called back later (e.g. by fn_415604). */
typedef void (*Callback)();

/* A point, as the engine's QuickDraw-like graphics use them. */
struct Point
{
    short x;
    short y;
};

/* The display mode WinMain asks for (640x480, 256 colours). */
struct DisplayMode
{
    unsigned short width;
    unsigned short height;
    unsigned long colors;
    short unknown8; /* 0: `colors` is a minimum */
    char unknownA[2];
};

/* What fn_48e928 reports about memory; +0xc is free physical memory. */
struct MemoryInfo
{
    long unknown0;
    long unknown4;
    long unknown8;
    unsigned long freePhysical;
    long unknown10;
    long unknown14;
};

/* Something with flags at +0x20. */
struct Flagged
{
    char unknown0[0x20];
    long flags;
};

struct Triple
{
    short a;
    short b;
    short c;
};

/* Something with a mode at +0x28 and three counters at +0x30. */
struct Counters
{
    char unknown0[0x28];
    long mode;
    char unknown2c[4];
    Triple counters;
};

/* Something with a handle at +6. */
struct HasHandle
{
    char unknown0[6];
    long handle;
};

/* A 22-byte entry with a value at +4. */
struct Entry22
{
    long unknown0;
    long value;
    char unknown8[14];
};

/* List entry: an index at +0, a key at +4 and the next entry at +0xe. */
struct Entry
{
    short index;
    short unknown2;
    short key;
    char unknown6[8];
    Entry *next;
};

/* Doubly-linked list node: fields at +0 and +4. */
struct Link
{
    Link *prev;
    Link *next;
};

/* Something reference-counted, with its count at +8. */
struct Counted
{
    long unknown0;
    Counted *next;
    long references;
};

/* An object whose third virtual function takes a flag. */
class Releasable
{
public:
    virtual void __cdecl virtual0();
    virtual void __cdecl virtual1();
    virtual void __cdecl virtual2(int flag);
};

/* Something starting with the tag 'ksTI' (bytes in memory order). */
struct Tagged
{
    long tag;
};

/* Something that records a return address at +0x28. */
struct Resume
{
    char unknown0[0x28];
    long address;
};

/* The engine's file name class (4 bytes, no virtual functions). The engine
   wasn't compiled with -p, so its methods use the C convention. */
class fileSpec
{
public:
    __cdecl fileSpec(const char *path);
    __cdecl ~fileSpec();

private:
    long unknown0;
};

/* Globals, by address */

extern Entry *g_4a00a0;
extern long g_4a00dc[];
extern char emptyString[]; /* @data 0x4a01b8 */
extern Callback g_4a07ac;
extern long g_4a07b0;
extern long g_4a07b4;
extern time_t g_4a07b8;
extern Callback g_4a07c4;
extern Callback g_4a07e8;
extern long g_4a07ec;
extern short g_4a3e5c; /* set when the game data is found in INSTALLFROMDIR */
extern char installFromDirKey[]; /* @data 0x4a3f06 */
extern char dataDirName[]; /* @data 0x4a3f15 */
extern char installToDirKey[]; /* @data 0x4a3f1b */
extern char userFileName[]; /* @data 0x4a4900 */
extern char rosterFileName[]; /* @data 0x4a4920 */
extern short aboveWindows311; /* @data 0x4a494a */
extern long g_4a4a00;
extern HWND mainWindow; /* @data 0x4a4a04 */
extern char *appName; /* @data 0x4a4a08 */
extern short g_4a4a0c;
extern short g_4a4a10;
extern Callback g_4a4a14;
extern long g_4a4a18;
extern long g_4a4a1c;
extern char minimumOfText[]; /* @data 0x4a4a20 */
extern char colors256Text[]; /* @data 0x4a4a2e */
extern char svgaRequiredFormat[]; /* @data 0x4a4a39 */
extern char color16Text[]; /* @data 0x4a4a8d */
extern char color24Text[]; /* @data 0x4a4a9a */
extern long savedDisk; /* @data 0x4a4aa8 */
extern unsigned short resolutionWidths[4]; /* @data 0x4a4aac */
extern unsigned short resolutionHeights[4]; /* @data 0x4a4ab4 */
extern long buttonKeys[3]; /* @data 0x4a4abc */
extern UINT buttonUpMessages[3]; /* @data 0x4a4ac8 */
extern char messageLogName[]; /* @data 0x4a4ad8 */
extern short g_4a4b98;
extern char *g_4a4ba0;
extern short g_4a4ce6;
extern char msgRequiresQuickTime[]; /* @data 0x4a4dc7 */
extern char msgInitOs[]; /* @data 0x4a4e28 */
extern char msgInitTimer[]; /* @data 0x4a4e40 */
extern char msgInitHeap[]; /* @data 0x4a4e5b */
extern char msgNotEnoughMemory[]; /* @data 0x4a4e75 */
extern char msgNotEnoughPhysicalMemory[]; /* @data 0x4a4e8c */
extern char msgInitFileManager[]; /* @data 0x4a4eac */
extern char msgInitResourceManager[]; /* @data 0x4a4ece */
extern char msgInitConfiguration[]; /* @data 0x4a4ef4 */
extern char msgInitSound[]; /* @data 0x4a4f24 */
extern char msgNoWaveDevices[]; /* @data 0x4a4f3f */
extern char msgNoMidiDevices[]; /* @data 0x4a4f5e */
extern char msgOutOfMemory[]; /* @data 0x4a5063 */
extern char configFileName[]; /* @data 0x4a5149 */
extern short g_4a79c0;
extern unsigned long g_4a79c4;
extern unsigned long g_4a79c8;
extern short g_4a7b94;
extern long g_4a7f58;
extern Counted *g_4a8dcc;
extern short g_4aa428;
extern short g_4aa42a;
extern long g_4aa498;
extern long g_4aa4c4;
extern char allocationFailed; /* @data 0x4aa4c8 */
extern char g_4aa4c9;
extern short g_4aa5d6;
extern short g_4aa5d8;
extern short g_4aa79a;
extern short g_4aa79c;
extern long g_4aa7a4;
extern short g_4aa7cc;
extern long g_4aafe8;
extern short bitsPerPixel; /* @data 0x4aafec */
extern short debugMode; /* @data 0x4ab474 */
extern short g_4ab480;
extern short g_4ab482;
extern short g_4ab49c;
extern short g_4ab49e;
extern Entry22 *g_4ab64c;
extern short g_4af350;
extern short g_4af35a;
extern short g_4afb90;
extern short g_4aff9a[];
extern short g_4b0d4e;
extern short g_4b0d50;
extern short g_4b0d52;
extern short g_4b0d54;
extern short g_4b0d56;
extern char installDir[256]; /* @data 0x4b1828 */
extern long fonts[3]; /* @data 0x4b28c8 */
extern char moduleFileName[256]; /* @data 0x4b28d4 */
extern char g_4b29d4[];
extern unsigned short instanceAtom; /* @data 0x4b2ae0 */
extern short quickTimeReady; /* @data 0x4b2ae8 */
extern short g_4b2aea;
extern short g_4b2aec;
extern HINSTANCE appInstance; /* @data 0x4b2af0 */
extern HINSTANCE appPreviousInstance; /* @data 0x4b2af4 */
extern char *appCommandLine; /* @data 0x4b2af8 */
extern long appShowCommand; /* @data 0x4b2afc */
extern short g_4b2b00;
extern char programPath[0x100]; /* @data 0x4b2b06 */
extern char savedDirectory[]; /* @data 0x4b2c06 */
extern WNDCLASS windowClass; /* @data 0x4b2d06 */
extern short classRegistered; /* @data 0x4b2d2e */
extern short g_4b2d32;
extern short g_4b2d34;
extern short g_4b2d36;
extern short windowed; /* @data 0x4b2d38 */
extern short g_4b2d3c;
extern short g_4b2d42;
extern long g_4b2d44[0x400];
extern long g_4b3d44[0x400];
extern long g_4b4d44[0x400];
extern long g_4b5d44[0x400];
extern short g_4b6d44[0x400];
extern short g_4b754a;
extern short g_4b7b38;
extern short g_4b7b3a;
extern long g_4b7b68;
extern long cursors[6]; /* @data 0x4b80ac */
extern short g_4b80c4[6];
extern short g_4b80d2;
extern unsigned long g_4b80d8;
extern unsigned long g_4b80dc;
extern short g_4b80e0; /* ends the main loop when set */
extern short g_4b83e4[];
extern short g_4b99d4;
extern char dataPath[256]; /* @data 0x4b99d6 */
extern short dataPathLength; /* @data 0x4b9ad6 */
extern char dataDrive; /* @data 0x4b9ad8 */
extern short g_4b9cf0;
extern short g_4b9cf4;
extern short g_4b9cf6;
extern short g_4b9cf8;
extern long g_4b9cfc;
extern long g_4b9d00;
extern long g_4b9d04;
extern long g_4b9d08;
extern short g_4b9d4c;
extern long g_4b9d70;
extern long g_4b9d74;

/* Game functions not decompiled yet */

/* Reports an error, printf-style. */
void __cdecl fn_41541a(const char *format, ...);
void fn_4144d0(DisplayMode *mode, long);
/* Formats into `buffer` (of `size` bytes), printf-style. */
void __cdecl fn_4150c7(long size, char *buffer, const char *format, ...);
void fn_415910();
void fn_4124cc(Point *where, long);
void fn_4140d6(long);
short fn_4140f3();
unsigned long fn_41571f(); /* a tick count */
void fn_415880();
void fn_43ac20();
void fn_4624f4();
void fn_464d7d();
unsigned long fn_464d88();
void __cdecl fn_46db93(const char *format, ...);
void fn_41f195(const char *message);
void fn_41f2c8(long, long);
void fn_41f668();
void fn_41f6fc(long);
void fn_44695c();
void fn_454c8e();
void fn_454caa();
void fn_454f61();
void fn_455f66();
LRESULT CALLBACK fn_45605e(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
void fn_456914();
void fn_4625b8();
void fn_46310c();
void fn_456c67(long);
short fn_46beac(long);
/* Loads resource `id` of type `type` (e.g. 'CURS') into *handle. */
void fn_46c4fe(long *handle, long type, short id, long, long);
/* Creates the font `name` at `size` into *font. */
void fn_46cb10(long *font, const char *name, long size, long);
/* Initialises the Mohawk OS layer, with a work buffer. */
short fn_46ddaf(HINSTANCE instance, void *buffer, long size);
/* QuickTime (see quicktime.py) */
long __cdecl QTInitialize(long *version);
long qtim_0b();

/* Engine functions whose calling conventions aren't known yet: these
   declarations produce the calls the game makes. */

void fn_476622(long handle);
/* The engine's graphics follow Mac QuickDraw: a current port, and conversions
   between a port's coordinates and the screen's. */
long getPort(); /* 0x48b510 */
void setPort(long port); /* 0x48d960 */
void globalToLocal(Point *point); /* 0x48c4cc */
void localToGlobal(Point *point); /* 0x48c688 */
short fn_476d0a(); /* initialises sound */
short fn_480642(); /* initialises the configuration file */
short fn_483732(long); /* initialises the file manager */
void __cdecl fn_48ac68(DisplayMode *mode, long, long, long, long);
long fn_48b4a8();
short fn_48cab4(long, long);
short fn_48c9e8(DisplayMode *mode, long); /* sets the display mode */
void fn_48d4c4(long);
void fn_48da48(long);
void *fn_48e6b4(long size); /* allocates memory */
unsigned long fn_48e7ec(); /* free memory */
void fn_48e928(MemoryInfo *info);
void fn_48ea00(short);
short fn_48ec85(long, long); /* initialises the heap */
void fn_48f2b0(long (*callback)(long, long));
short fn_4922c6(); /* initialises the resource manager */
short fn_493096(); /* initialises the timer */

/* Reads `key` from `section` of the INI file `file` into buffer; non-zero if
   it couldn't. */
short fn_480790(const fileSpec &file, const char *section, const char *key, char *buffer,
                long size);
/* Zero if `file` exists (as the game uses it). */
short fn_483420(const fileSpec &file);
/* Opens `file` (mode 1 as the game uses it), returning a handle or 0. */
long fn_484b50(const fileSpec &file, long mode);
/* Closes a file opened by fn_484b50. */
void fn_48266c(long file, long);

void fn_4812bc(short handle);
void fn_48f660(long handle, long, long);

/* Decompiled functions, by address */

Entry *fn_4115f5(short key, long tag);
void fn_4117a8(HasHandle *object);
short fn_412b4d(void (*callback)(long));
void fn_413bcf(long value);
void fn_413c6d(void **block);
short fn_413dc0();
void __cdecl nextRingIndex(short *index);
void fn_414358(void **block);
void fn_414ce7(short *handle);
void fn_4153b0(Callback callback);
void fn_4153bf(long value);
void fn_4153ce(long value);
void fn_415514();
void fn_415604(Callback callback);
short fn_4157f3();
void fn_415811();
void fn_41581b(short flag);
unsigned short toLowerAscii(unsigned short c);
unsigned short toUpperAscii(unsigned short c);
void fn_415a11(Callback callback);
void fn_415a20(long value);
long fn_417906(long);
long fn_4196a8(long);
short fn_419f1a();
int fn_41d3e6(long, short value);
void fn_41d9e4(long);
void fn_41d9eb(long, long);
void fn_427e1a(Flagged *object, short code);
void fn_42c10b(long);
void fn_42c112(long, long);
void fn_42c6cb(short value);
short fn_42e693();
void fn_42e69a();
void fn_42fc89(Counters *object);
long fn_4320da(long);
void fn_4334f0(long, short value);
void fn_43595f(long);
void fn_435966(long, long);
int fn_43691d(long, short value);
short indexOfLargestExcept(short exclude);
short fn_437acb(short i);
short fn_4381bb();
void spliceList(Link *other, Link *list);
short fn_44027b(long, long);
void fn_446962(char *, const char *);
void findGameData();
short preferFirstFile(const char *first, const char *fallback);
void __cdecl fn_454ca4();
void mainLoopEvents();
void checkDisplayMode(DisplayMode *mode);
void handleWaitingMessage();
void waitWhilePaused();
short pumpMessage(MSG *message, unsigned short first, unsigned short last, unsigned short flags);
void handleSystemKey(MSG *message);
void handleMessage(MSG *message);
short createMainWindow(long, long);
short addModifierKeys(short modifiers);
void getCursorPosition(Point *where);
void setCursorPosition(short x, short y);
short isButtonStillDown(unsigned short button);
short allocateBlock(void **block, unsigned long size);
void getClockTime(char *hour, char *minute, char *second);
void enterProgramDirectory();
void restoreDirectory();
void brightenPalette(PALETTEENTRY *entries, short first, short count);
short isInputWaiting(long which);
void fn_4565c8(long a, long b, long c, short d, long e);
void dumpMessages();
long fn_455013(long, long);
int isMousePresent();
void freeAndClear(void **block);
char *intToDecimal(int value, char *buffer);
char *unsignedToDecimal(unsigned long value, char *buffer);
void fn_455e26(long);
void fn_455e2d(long);
long fn_455e85(long, long);
void fn_456a2f(Callback callback);
short fn_4568d8();
short mainLoopUpdate();
void fn_456a3e(long first, long second);
void fn_456a55(long value);
short fn_456bf6();
int fn_4572bf();
short fn_457fbb();
void fn_45b39a(short value);
void fn_45bfc0(long value);
void fn_465175();
long fn_46b07b(long);
void fn_46b747(long, short id);
void fn_46be2e(long value);
short fn_46bee2();
short fn_46bee9(short value);
void setDataPath(const char *path);
void fn_46ca9c(long *handle);
long __cdecl fn_46d827(Counted *object);
short fn_46d9c8();
void fn_46da35(short value);
int isAlignedPointer(void *pointer);
long atomicDecrement(long *value);
long atomicExchange(long *target, long value);
long atomicIncrement(long *value);
short debugBreak(short value);
void fn_46dc45();
long fn_46dd21();
long fn_46dd27();
long fn_46dd2d();
unsigned long currentTimeMs();
void fn_46dfd4(long, long);
void fn_46dfe2(long, long);
short fn_46dff0();
short fn_46dff7();
long fn_46e0d7(long value);
long fn_46e0ec(long);
void fn_46e1e7(short value);
long fn_46e1f8(long value);
Tagged *fn_46e202(Tagged *object);
char __cdecl fn_46e28e(char value);
int __cdecl highByte(unsigned short value);
long fn_46e5dc();
short fn_46e5ed();
long fn_46e5f4();
void fn_46e842(Releasable *object);
void fn_46eac8(Releasable *object);
long __cdecl fn_46f43a(long value);
void fn_46f771(Resume *resume, unsigned short depth);
void fn_46f78e(short value);

#endif
