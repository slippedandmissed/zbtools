/*
 * Declarations shared by the decompiled modules: the game's globals, the types
 * worked out so far, and every decompiled function, so any module can use them.
 * Names follow CLAUDE.md: by address until their purpose is known. Types are
 * inferred from how the code uses them and grow as more of it is decompiled.
 */

#ifndef ZOOMBINIS_H
#define ZOOMBINIS_H

#include <windows.h>
#include <mmsystem.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

/* Types */

/* Big-endian values (the Mac's byte order, as in the game's data) and back. */
inline unsigned short swapShort(unsigned short value)
{
    unsigned char *bytes = (unsigned char *)&value;
    return (unsigned short)(bytes[1] | bytes[0] << 8);
}

inline unsigned long swapLong(unsigned long value)
{
    unsigned char *bytes = (unsigned char *)&value;
    return (bytes[3] | (unsigned short)bytes[2] << 8)
           | (unsigned long)(bytes[1] | (unsigned short)bytes[0] << 8) << 16;
}

/* A Mohawk resource type, built Mac-style from its four characters (so
   RESOURCE_TYPE('C','U','R','S') is 0x43555253; C++'s multi-character
   constants put the bytes the other way round in Borland C++). */
#define RESOURCE_TYPE(a, b, c, d) (((long)(a) << 24) | ((long)(b) << 16) | ((long)(c) << 8) | (d))

/* A function registered to be called back later (e.g. by fn_415604). */
class basePort;
class Palette;
class Font;
typedef void (*Callback)();
/* Told when the application is activated or deactivated. */
typedef void (*ActivateHook)(short active);
typedef short (*FileCallback)(const char *name, void *data);

/* A point, as the engine's QuickDraw-like graphics use them. */
struct Point
{
    short x;
    short y;
};

/* An input event, as the event queue holds them (Mac-style). */
struct Event
{
    short type; /* 1 a key, 2 a mouse button */
    Point where; /* for mouse buttons */
    long unknown6;
    short code; /* the key or the button */
    short unknownC;
};

/*
 * The focus module (focus.cpp) moves a focus over on-screen items,
 * arranged in groups (each with its own handlers) and lists of groups, by
 * keyboard or mouse. Its state lives in globals from 0x4aa490 (see
 * InputState).
 */

/* Where the focus is; the last field is the item's index in its group. */
struct Cursor
{
    Point a;
    Point b;
    Point c; /* c.y: the item's index */
};

/* A rectangle of shorts (Windows' order). */
struct ShortRect
{
    short left;
    short top;
    short right;
    short bottom;
};

/* Resources (images) by index, from 1. */
struct ResourceList
{
    long unknown0;
    long unknown4;
    long resources[1];
};

/* Text joined from two texts (joinText); a text is either a string or one of
   these, told apart by the first byte. */
struct JoinNode
{
    char tag; /* 0xff (negative), where a string's first character isn't */
    const char *first;
    const char *second;
};

/* A group of graphic buttons' images (6 bytes). Images come in pairs, off
   and on, one pair per state; kind 1 has only the "on" images. */
struct ButtonGroup
{
    ResourceList *images;
    short kind;
};

/* A sprite in an animation (8 bytes). */
struct SpriteBits
{
    unsigned short mode : 4; /* drawing mode */
    unsigned short other : 12;
};

struct Sprite
{
    short image; /* cast member, from 1; 0: none */
    union
    {
        unsigned short all;
        SpriteBits bits;
    } flags;
    short x;
    short y;
};

struct Anim;
typedef void (*AnimCallback)(Anim *anim, short value);

/*
 * A running animation (0x358 bytes, then its cast): a script of opcodes
 * moving up to 32 sprites over a background, drawn into a port and copied to
 * the screen through lists of changed rectangles.
 */
struct Anim
{
    basePort *port; /* drawn into */
    basePort *screen; /* shown on */
    basePort *background; /* a saved copy of what's under it, or 0 */
    ShortRect bounds;
    ShortRect changed;
    long script; /* the resource */
    unsigned char *pc;
    union
    {
        short count; /* frames left at this rate */
        unsigned long until; /* when a pause ends */
        char value; /* the MIDI value waited for */
    } wait;
    short drawn;
    unsigned short frame;
    unsigned short frameTicks;
    unsigned long frameTime;
    short counts[2];
    ShortRect rects[2][32]; /* to show, and to erase */
    AnimCallback callbacks[4];
    Sprite sprites[32];
    long sounds; /* resource: a count, then sound ids */
    short noOverlap;
    short unknown34c;
    long unknown34e;
    long unknown352;
    unsigned short castCount;
    long cast[1];
};

struct AnimFlags
{
    unsigned char saveBackground : 1;
    unsigned char keepMidi : 1;
    unsigned char finishOnInterrupt : 1;
    unsigned char interruptible : 1; /* by a click or key */
    unsigned char waitForSounds : 1;
    unsigned char restoreBackground : 1;
    unsigned char noOverlap : 1;
};

/* How to play an animation (0xc bytes). */
struct AnimSpec
{
    Anim *anim;
    unsigned short id;
    unsigned char idOffset;
    AnimFlags flags;
    unsigned short firstColor; /* colours set from g_4aa7e8 */
    unsigned short colorCount;
};

/* A palette fade in progress (0xc16 bytes). */
struct Fade
{
    long step; /* 16.16 levels (of 256) per ms */
    long progress; /* 16.16 */
    short level; /* the last level shown */
    short steps; /* steps shown, when not by time */
    unsigned long lastTime;
    short byTime; /* levels follow the time taken; else 4 levels a step */
    short first;
    unsigned short count;
    PALETTEENTRY target[256];
    PALETTEENTRY current[256];
    PALETTEENTRY start[256];
};

/* A region: a list of rectangles, in a handle ('Rngr'). */
struct Region
{
    long tag; /* 'Rngr' while valid */
    ShortRect bounds;
    unsigned short capacity; /* rectangles allocated (16 at a time) */
    unsigned short count;
    ShortRect rects[1];
};

/* A saved area of a port (e2MapSave), 0xe bytes. */
struct MapSave
{
    basePort *port; /* holding the saved pixels */
    short locks;
    ShortRect rect;
};

/*
 * The Mohawk engine's types, as the game's calls show them (the engine was
 * compiled separately, without -p, so its methods are __cdecl).
 *
 * Rect: the engine takes rectangles by const reference, and the game passes
 * its ShortRects, each converted into a temporary (memcpy'd, by an inline
 * constructor).
 */
class Rect
{
public:
    short left;
    short top;
    short right;
    short bottom;
    Rect(const ShortRect &rect) { memcpy(this, &rect, sizeof(Rect)); }
    __cdecl Rect(short left, short top, short right, short bottom); /* 0x48c8d4 */
};

class WinPoint;

/* A point as the engine passes it by value. */
class Pt : public Point
{
public:
    __cdecl Pt(short x, short y); /* 0x48da17 */
    __cdecl Pt(const Point &point); /* 0x48c6f3 */
    __cdecl Pt(const WinPoint &point); /* 0x48c736 */
};

/* A Windows POINT made from a Pt. */
class WinPoint : public tagPOINT
{
public:
    __cdecl WinPoint(const Pt &point); /* 0x48c70c */
};

/* A Windows RECT made from a Rect. */
class WinRect : public tagRECT
{
public:
    __cdecl WinRect(const Rect &rect); /* 0x48df26 */
};

/* The four bytes of a Color. */
struct ColorBytes
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char kind; /* 0xff none, 0x80 a palette index (in `index`), else RGB (0x10: ...) */
};

/* Color: an RGB colour or a palette index. Copied whole and then by index
   (hence the union). */
class RGBColor;
class Color
{
public:
    union {
        long value;
        unsigned short index;
        ColorBytes bytes;
    };
    __cdecl Color(); /* 0x488874: none */
    __cdecl Color(unsigned short index); /* 0x48889d: a palette index (0xffff: none) */
    Color(const Color &color)
    {
        value = color.value;
        index = color.index;
    }
    Color &__cdecl setRgb(const Color &color); /* 0x4888d9 */
    unsigned short __cdecl paletteIndex() const; /* 0x4888f2 */
    RGBColor __cdecl rgb() const; /* 0x4889a4 */
    long __cdecl kind() const; /* 0x488a39 */
};

/* A Color's bytes as a value of their own. */
class RGBColor
{
public:
    ColorBytes bytes;
    __cdecl RGBColor(unsigned char red, unsigned char green, unsigned char blue,
                     unsigned char kind); /* 0x48c4ac */
    __cdecl RGBColor(const Color &color); /* 0x488a64 */
};

/* An item (0x24 bytes). */
struct InputItem
{
    ShortRect bounds;
    Point hotspot; /* where the mouse goes, unless the group says its centre */
    short key; /* the key that selects it */
    char unknownE[4];
    unsigned short flags;
    char unknown14[4];
    Cursor cursor; /* where it is */
};

/* Callbacks, called through fn_412b4d with the current item; named by offset. */
struct InputHandlers
{
    void (*handler0)(InputItem *item);
    void (*handler4)(InputItem *item);
    void (*handler8)(InputItem *item);
    void (*handlerC)(InputItem *item);
    void (*handler10)(InputItem *item);
    void (*handler14)(InputItem *item);
    void (*handler18)(InputItem *item);
    void (*handler1C)(InputItem *item);
    void (*handler20)(InputItem *item);
    void (*handler24)(InputItem *item);
    short (*hitTest)(Point *where, InputItem *item); /* +0x28 */
    void (*enter)(InputItem *item); /* +0x2c */
    void (*leave)(InputItem *item); /* +0x30 */
};

/* A group of items with its handlers (16 bytes). */
struct Group
{
    InputHandlers *handlers;
    InputItem *items;
    short count;
    short flags; /* the top three bits are its kind */
    short *sounds; /* per item, the sounds for switching it off and on */
};

/* A list of groups (12 bytes); g_4a01ac is an array of them. */
struct GroupList
{
    Group *groups;
    short count;
    short flags;
    void (*changed)(short value);
};

/* The state (0x30 bytes) fn_413a4e loads into the globals and fn_413afd
   saves from them; the part from +0x18 only if asked. */
struct InputState
{
    GroupList *list;
    Group *group;
    InputItem *item;
    Point cursorA;
    Point cursorB;
    Point cursorC;
    short search; /* what fn_41391d looks for (0-7) */
    Point *point;
    InputItem *unknown1E;
    short unknown22;
    short unknown24;
    short unknown26;
    unsigned short unknown28;
    short mode;
    short unknown2C;
    short unknown2E;
};

/* A scene (a puzzle or screen) of the game; only its frame function is known. */
struct Scene
{
    long unknown0;
    long unknown4;
    Callback frame; /* called every pass of the main loop (gameFrame) */
};

/* The display mode WinMain asks for (640x480, 256 colours). */
struct DisplayMode
{
    unsigned short width;
    unsigned short height;
    unsigned long colors; /* at least; 0xffff / -1: any */
    short palettized; /* needs an 8-bit palettized mode */
    char unknownA[2];
};

/* What getMemoryInfo reports (from GlobalMemoryStatus). */
struct MemoryInfo
{
    unsigned long availableVirtual;
    unsigned long totalVirtual;
    unsigned long availablePhysical;
    unsigned long totalPhysical;
    unsigned long availablePageFile;
    unsigned long totalPageFile;
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

/* A 22-byte entry with a value at +4. */
struct Entry22
{
    long unknown0;
    long value;
    char unknown8[14];
};

/* A loaded sound (wave or MIDI), in the list at g_4a00a0. */
struct Entry
{
    short type; /* 0 a wave, 1 MIDI (see soundTypes) */
    short unknown2; /* which way loadSound loads it */
    short key;
    long handle; /* the engine's */
    long unknownA; /* its resource, for loadSound */
    Entry *next;
};

/* One of a sound type's 4 channels. */
struct SoundChannel
{
    unsigned short id; /* 0xffff: free */
    short playing;
    unsigned short started;
};

/* What the engine tells a sound's owner (fn_411d2c). */
struct SoundNotice
{
    unsigned short what; /* 0 a value (in data), 1 finished */
    short unknown2;
    unsigned short unknown4; /* the value's length; an error */
    short unknown6;
    char *data;
};

/* Doubly-linked list node: fields at +0 and +4. */
struct Link
{
    Link *prev;
    Link *next;
};

/* Something reference-counted, with its count at +8. */
struct Deferred;

/* A lock (the OS layer's): while it's held, calls posted to it (deferCall)
   wait in a queue, and the last leaveLock runs them. Locks can be listed at
   `locks` (all of them held at once by fn_46dc45). */
struct DeferLock
{
    DeferLock *prev;
    DeferLock *next;
    unsigned long depth; /* how many times it's held */
    Deferred *queue;
};

/* A call to make under a DeferLock. */
struct Deferred
{
    Deferred *next;
    long queued;
    unsigned long count; /* posted and not yet run */
    void (*proc)(void *data);
    void *data;
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
    __cdecl fileSpec(); /* 0x4850ec */
    __cdecl fileSpec(const char *path);
    __cdecl fileSpec(const fileSpec &directory, const char *name); /* 0x48518c */
    __cdecl ~fileSpec(); /* 0x48533e */
    fileSpec &__cdecl operator=(const fileSpec &from); /* 0x4853a2 */
    short __cdecl compare(const fileSpec &with) const; /* 0x4853f3: 0 if the same, else 0x2844 or an error */
    void __cdecl getPath(char *path) const; /* 0x4854c5 */
    short __cdecl volume(long *volume) const; /* 0x485596 */

private:
    long unknown0;
};

/* Globals, by address */

extern short soundLevel; /* @data 0x4a0090 */
extern long g_4a0098;
extern unsigned long g_4a009c; /* sounds larger than this are loaded differently */
extern Entry *g_4a00a0;
extern short channelCounts[2]; /* @data 0x4a00a4 */
extern char currentChannel[2]; /* @data 0x4a00a8 */
extern SoundChannel soundChannels[2][4]; /* @data 0x4a00aa */
extern long soundTypes[2]; /* @data 0x4a00dc */
extern basePort **screenPortRef; /* @data 0x4a0070: animations show on *screenPortRef */
extern unsigned char animOpcodes[13]; /* @data 0x4a0074 */
extern unsigned char animOperandSizes[13]; /* @data 0x4a0081 */
extern short buttonColors[6]; /* @data 0x4a019c: colours buttons are drawn in */
extern GroupList *g_4a01ac;
extern short g_4a01b0;
extern char emptyString[]; /* @data 0x4a01b8 */
extern char msgUnableToCreate[]; /* @data 0x4a0206 */
extern char textSound[]; /* @data 0x4a0217 */
extern char textMidi[]; /* @data 0x4a021d */
extern char textWaveform[]; /* @data 0x4a0222 */
extern char msgUnknownChunk[]; /* @data 0x4a022b */
extern char msgUnableToPrepare[]; /* @data 0x4a023f */
extern char msgSeekError[]; /* @data 0x4a0251 */
extern char msgUnableToStart[]; /* @data 0x4a025d */
extern char msgPrematureExit[]; /* @data 0x4a026d */
extern char formatJoin[]; /* @data 0x4a027d */
extern char formatErrorNumber[]; /* @data 0x4a0282 */
extern char formatSoundId[]; /* @data 0x4a028d */
extern char msgDeviceFailed[]; /* @data 0x4a0297 */
extern char msgNoScreenPort[]; /* @data 0x4a0313: graphics and placeGamePort share it */
extern short breakpointKey; /* @data 0x4a0708 */
extern char msgUnableToLockPort[]; /* @data 0x4a0710 */
extern char g_4a07a8[2]; /* a one-character string (the second byte is an empty one) */
extern Callback g_4a07ac; /* called before a fatal error is reported */
/* reports an error (showError, as the game sets it up) */
extern void (*errorReporter)(const char *prefix, const char *format, va_list args); /* @data 0x4a07b0 */
extern const char *g_4a07b4; /* the message for a fatal error */
extern unsigned long randomSeed; /* @data 0x4a07b8 */
extern short seedPending; /* @data 0x4a07bc: seed from the time first */
extern unsigned long starvationLimit; /* @data 0x4a07c8: longest gap between main loop passes */
extern char msgStarvation[]; /* @data 0x4a07cc */
extern Callback g_4a07c4;
extern Callback g_4a07e8;
extern Callback g_4a07ec; /* draws the window's contents, if set */
extern Scene *scenes[]; /* @data 0x4a26e8 */
extern short g_4a3e5c; /* set when the game data is found in INSTALLFROMDIR */
extern char installFromDirKey[]; /* @data 0x4a3f06 */
extern char dataDirName[]; /* @data 0x4a3f15 */
extern char installToDirKey[]; /* @data 0x4a3f1b */
extern char userFileName[]; /* @data 0x4a4900 */
extern char rosterFileName[]; /* @data 0x4a4920 */
extern short aboveWindows311; /* @data 0x4a494a */
extern short g_4a4974;
extern short g_4a4976[12];
extern void (*g_4a4a00)(short active); /* told when the window is (de)activated */
extern short g_4a4ad6;
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
extern unsigned short appActive; /* @data 0x4a4ae4 */
extern ShortRect g_4a4ae6;
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
extern DeferLock *locks; /* @data 0x4a8dcc */
extern short loadWholeCast; /* @data 0x4aa410 */
extern short keepFrameRate; /* @data 0x4aa412: frames stay on the beat when late */
extern ShortRect animArea; /* @data 0x4aa414 */
extern char *scriptText; /* @data 0x4aa41c */
extern long castInfo; /* @data 0x4aa420 */
extern short animDrawing; /* @data 0x4aa424: stepping (not skipping) */
extern short g_4aa428;
extern short g_4aa42a;
extern short g_4aa42c;
extern char *g_4aa430;
extern char *g_4aa434;
extern char *g_4aa438;
extern short soundErrorsIgnored; /* @data 0x4aa43c */
extern ButtonGroup buttonGroups[10]; /* @data 0x4aa440 */
extern char *buttonText; /* @data 0x4aa47c */
extern char *buttonError; /* @data 0x4aa480 */
extern InputItem *highlightedItem; /* @data 0x4aa484 */
extern short buttonsOffscreen; /* @data 0x4aa488: drawn buttons aren't copied to the screen */
extern unsigned short g_4aa48a;
extern unsigned char g_4aa48b;
extern short g_4aa48c; /* how many lists g_4a01ac has */
extern GroupList *g_4aa490;
extern Group *g_4aa494;
extern InputItem *g_4aa498;
extern Cursor g_4aa49c;
extern InputItem *enteredItem; /* @data 0x4aa4a8 */
extern short g_4aa4ac;
extern Point *g_4aa4b0;
extern InputItem *g_4aa4b4;
extern short g_4aa4b8;
extern short g_4aa4ba;
extern short g_4aa4bc;
extern unsigned short g_4aa4be;
extern short g_4aa4c0;
extern short g_4aa4c2;
extern void (*g_4aa4c4)(Point *where);
/* Which error reportJoinedError reports */
extern char allocationFailed; /* @data 0x4aa4c8: "Not enough near memory for" */
extern char outOfMemory; /* @data 0x4aa4c9: "Not enough memory for" */
extern char portFailed; /* @data 0x4aa4ca: "Unable to allocate port for" */
extern char loadFailed; /* @data 0x4aa4cb: "Unable to load" */
extern char joinedText[0x100]; /* @data 0x4aa4cc */
extern short reportingJoinedError; /* @data 0x4aa5cc */
extern short breakpointKeyEnabled; /* @data 0x4aa5d4 */
extern short dispatchingEvents; /* @data 0x4aa5d6 */
extern short breakpointRequested; /* @data 0x4aa5d8 */
extern Event eventQueue[32]; /* @data 0x4aa5da */
extern short eventHead; /* @data 0x4aa79a */
extern short eventTail; /* @data 0x4aa79c */
extern Fade *defaultFade; /* @data 0x4aa7a0 */
extern basePort *screenPort; /* @data 0x4aa7a4: the window's port */
extern ShortRect gameRect; /* @data 0x4aa7a8: the game's area */
extern ShortRect screenRect; /* @data 0x4aa7b0 */
extern ShortRect g_4aa7b8;
extern basePort *workPort; /* @data 0x4aa7c8: where the game draws, off screen */
extern short g_4aa7cc;
extern short g_4aa7ce;
extern DisplayMode displayMode; /* @data 0x4aa7d0 */
extern DisplayMode g_4aa7dc;
extern PALETTEENTRY g_4aa7e8[256];
extern PALETTEENTRY g_4aabe8[256];
extern Palette *palette; /* @data 0x4aafe8 */
extern short bitsPerPixel; /* @data 0x4aafec */
extern PALETTEENTRY colors[256]; /* @data 0x4aafee: the palette's colours */
extern va_list formatArgs; /* @data 0x4ab40c: formatString's arguments */
extern short lockedCount; /* @data 0x4ab410 */
extern long lockedResources[10]; /* @data 0x4ab414: resources %L locked */
extern char *lockedData[10]; /* @data 0x4ab43c */
/* extra conversions for formatString: whether a character starts one, and its text */
extern short (*isFormatCharacter)(char c); /* @data 0x4ab464 */
extern char *(*formatCharacter)(char c); /* @data 0x4ab468 */
extern char numberText[]; /* @data 0x4ab46c */
extern short debugMode; /* @data 0x4ab474 */
extern short debugging; /* @data 0x4ab476: errors stop in the debugger */
extern short reportingError; /* @data 0x4ab478 */
extern PALETTEENTRY *g_4ab3f0;
extern char *g_4ab3f4;
extern char *g_4ab3f8;
extern char *g_4ab3fc;
extern char *g_4ab400;
extern short g_4ab404; /* displayMode.unknown8 */
extern short loadingAnimation; /* @data 0x4ab480: the main loop's callback is held off */
extern short clockInTicks; /* @data 0x4ab482: the clock counts 60ths of a second, else ms */
extern unsigned long clockStoppedAt; /* @data 0x4ab484 */
extern unsigned long clockOffset; /* @data 0x4ab488 */
extern unsigned long timers[4]; /* @data 0x4ab48c: when each expires */
extern short g_4ab49c;
extern short g_4ab49e;
extern unsigned long lastCheck; /* @data 0x4ab4a0 */
extern unsigned long thisCheck; /* @data 0x4ab4a4 */
extern Entry22 *g_4ab64c;
extern short g_4af350;
extern short g_4af35a;
extern short g_4afb90;
extern short g_4aff9a[];
extern short currentScene; /* @data 0x4b0d4e */
extern short g_4b0d50;
extern short g_4b0d52;
extern short g_4b0d54;
extern short g_4b0d56;
extern char installDir[256]; /* @data 0x4b1828 */
extern Font *fonts[3]; /* @data 0x4b28c8 */
extern char moduleFileName[256]; /* @data 0x4b28d4 */
extern char g_4b29d4[];
extern short g_4b2ad4;
extern long g_4b2ad8;
extern long g_4b2adc;
extern unsigned short instanceAtom; /* @data 0x4b2ae0 */
extern short quickTimeReady; /* @data 0x4b2ae8 */
extern short g_4b2aea;
extern short g_4b2aec;
extern short g_4b2aee;
extern HINSTANCE appInstance; /* @data 0x4b2af0 */
extern HINSTANCE appPreviousInstance; /* @data 0x4b2af4 */
extern char *appCommandLine; /* @data 0x4b2af8 */
extern long appShowCommand; /* @data 0x4b2afc */
extern short g_4b2b00;
extern short g_4b2b02;
extern short g_4b2b04;
extern char programPath[0x100]; /* @data 0x4b2b06 */
extern char savedDirectory[]; /* @data 0x4b2c06 */
extern WNDCLASS windowClass; /* @data 0x4b2d06 */
extern short classRegistered; /* @data 0x4b2d2e */
extern short g_4b2d30;
extern short g_4b2d32;
extern short g_4b2d34;
extern short inputIgnored; /* @data 0x4b2d36: keys and clicks are dropped */
extern short windowed; /* @data 0x4b2d38 */
extern short g_4b2d3a;
extern short g_4b2d3c;
extern short g_4b2d3e;
extern short g_4b2d40;
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
extern unsigned long g_4b80d4;
extern unsigned long g_4b80d8;
extern unsigned long g_4b80dc;
extern short g_4b80e0; /* ends the main loop when set */
extern short g_4b83e4[];
extern short g_4b99d4;
extern char dataPath[256]; /* @data 0x4b99d6 */
extern short dataPathLength; /* @data 0x4b9ad6 */
extern char dataDrive; /* @data 0x4b9ad8 */
extern short regionErrorCode; /* @data 0x4b9b64 */
extern short g_4b9cf0;
extern short osError; /* @data 0x4b9cf4 */
extern short g_4b9cf6;
extern short engineActive; /* @data 0x4b9cf8 */
extern ActivateHook activateHook; /* @data 0x4b9cfc */
extern short g_4b7cf8;
extern HINSTANCE engineInstance; /* @data 0x4b9d00 */
extern unsigned long appThread; /* @data 0x4b9d04 */
extern HWND appWindow; /* @data 0x4b9d08 */
extern unsigned short g_4b9d22;
extern short g_4b9d4c;
extern long g_4b9d70;
extern long g_4b9d74;

/* Game functions not decompiled yet */

/* Reports an error, printf-style. */
void __cdecl fatalError(const char *format, ...);
void fn_4144d0(DisplayMode *mode, long);
/* Formats into `buffer` (of `size` bytes), printf-style. */
void __cdecl fn_4150c7(long size, char *buffer, const char *format, ...);
void fn_415910();
unsigned long fn_41571f(); /* a tick count */
unsigned long fn_415772(); /* a tick count */
void checkStarvation();
short waitForEventOrTimer(short timer, short type, short discard);
short fn_4156a3(short timer, short type, short discard);
short waitForEventFor(unsigned short timer, long ticks, short type, short discard);
short fn_4156e3(unsigned short timer, long ticks, short type, short discard);
void runMainLoop(short passes);
unsigned long clockTime();
unsigned long clockMs();
unsigned long clockTicks();
void setTimer(unsigned short timer, long ticks);
short timerExpired(unsigned short timer);
void setStarvationLimit(unsigned long limit);
void fn_415910();
void fn_415916();
short isLastRepeated(char *items, unsigned short count, unsigned short size);
short allocateSlot(unsigned long *used, short count, unsigned long reserved);
short randomBelow(short limit);
void fn_46be3d();
unsigned long timerTime(); /* the engine's clock, in ms */
void fn_43ac20();
short playSound(short key, long type, short channel, short eventType, short discard);
short fn_45590b();
void fn_455ab0(short type);
void fn_46293a(short key);
void fn_4624bd(Point *where, short button);
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
void fn_455f66();
void fn_455023(short);
void fn_46251c(long);
LRESULT CALLBACK mainWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
void fn_456914();
void fn_4625b8();
void fn_46310c();
void fn_456c67(long);
short fn_46beac(long);
/* Loads resource `id` of type `type` (e.g. 'CURS') into *handle. */
void fn_46c4fe(long *handle, long type, short id, const char *what, short);
/* Creates the font `name` at `size` into *font. */
void fn_46cb10(Font **font, const char *name, long size, long);
/* Initialises the Mohawk OS layer, with a work buffer. */
short fn_46ddaf(HINSTANCE instance, void *buffer, long size);
/* QuickTime (see quicktime.py) */
long __cdecl QTInitialize(long *version);
long qtim_0b();
long __cdecl cmgr_0b(long, HWND window, UINT message, WPARAM wParam, LPARAM lParam);

/* Engine functions whose calling conventions aren't known yet: these
   declarations produce the calls the game makes. */

char *fn_46cafb(long resource); /* a resource's data */
void fn_41585f();
void fn_46c602(long *);
/* Finds resource `id` of type `type`; 0 if there's none. */
long fn_46c402(long type, short id, short);
void fn_46c5b7(long *resource); /* releases a resource */
/* Joins two strings into a new block at *joined. */
void joinText(char **joined, const char *first, const char *second);
void reportJoinedError(char *message);
short fn_480b80(InputItem *item, Point *where); /* the default hit test */
/* The engine's graphics follow Mac QuickDraw: a current port, and conversions
   between a port's coordinates and the screen's. */
basePort *getPort(); /* 0x48b510 */
basePort *setPort(basePort *port); /* 0x48d960: the previous one */
short globalToLocal(Point *point); /* 0x48c4cc */
short localToGlobal(Point *point); /* 0x48c688 */
short fn_480642(); /* initialises the configuration file */
short fn_483732(long); /* initialises the file manager */
void __cdecl initDisplayMode(DisplayMode *mode, unsigned short width, unsigned short height, unsigned long colors,
                             short palettized);
unsigned short realizePalette(Palette *palette, short foreground);
short canUseDisplayMode(DisplayMode *mode, short change); /* whether a display mode is available (filling in the one it would use) */
Font *setFont(Font *font); /* the previous one */
short setTakeStatic(short take);
short setCursorLevel(short level);
short showCursor();
short hideCursor();
short fn_4887f4();
short fn_48b1b4();
void fn_455273(short);
void fn_44695c();
void fn_46258a();
/* loading */
unsigned short loadMidi(short key);
void unloadMidi(short key);
void fn_414f5c(short key);
short playMidiOn(short key, short channel);
void stopMidi(unsigned short id);
void fn_414fa3(unsigned short id);
short isMidiPlaying(unsigned short id);
short playMidi(short key, short channel, short eventType, short discard);
short waitForMidi(unsigned short id, short eventType, short discard);
short fn_415014(unsigned short id, short eventType, short discard);
short fn_415034(unsigned short id, short stop);
short fn_41504f(char value);
short waitForMidiValue(char value, short eventType, short discard);
short fn_415083(char value, short eventType, short discard);
void stopAllMidi();
void setFormatCharacters(short (*isSpecial)(char c), char *(*text)(char c));
char *__cdecl formatText(long size, char *text, const char *format, ...);
char *formatTextV(long size, char *text, const char *format, va_list args);
char *formatString(long size, char *text, const char *format);
void formatArgument(long size, char **text, const char **format);
void __cdecl warning(const char *format, ...);
void __cdecl unableToLoad(const char *format, ...);
void __cdecl notEnoughMemory(const char *format, ...);
void __cdecl notEnoughNearMemory(const char *format, ...);
void __cdecl unableToAllocatePort(const char *format, ...);
void reportFatalError(const char *prefix, const char *format, va_list args);
char *fn_46cabc(long resource); /* locks a resource */
void fn_46cad1(long resource); /* unlocks it */
char *nthString(char *table, unsigned char n);
char *skipStrings(char *text, short count);
short randomUpTo(unsigned short limit);
void __cdecl formatJoined(const char *text);
short collectParts(short count, const char **parts, const char *text);
/* graphics */
void initGraphics(DisplayMode *mode, short depth);
void closeGraphics();
void drawImage(ResourceList *images, short index, short x, short y, short mode, short anchor);
void drawImageInColor(ResourceList *images, short index, short x, short y, short mode, short color,
                      short anchor);
void getColors(PALETTEENTRY *to, short first, short count);
void setColors(PALETTEENTRY *from, short first, short count);
void fn_4148da(short first, short count);
void createPort(basePort **port, ShortRect *bounds, short keep, const char *name);
void destroyPort(basePort **port, short release);
void fn_414a2e(basePort *port, ShortRect *bounds);
void saveRect(MapSave **save, ShortRect *rect, short locked, const char *name);
void restoreRect(MapSave **save, short free);
void freeSave(MapSave **save);
void clipRect(short *region, ShortRect *rect, short keep);
void fn_414c25(short *region, short free);
void getClipRegion(short *region, short create);
void createRegion(short *region);
void freeRegion(short *region);
void copyBits(basePort *to, basePort *from, ShortRect *rect);
void showRect(ShortRect *rect);
void lockPortOrFail(basePort *port);
void lockSave(MapSave *save);
void unlockSave(MapSave *save);
void alignRect(ShortRect *rect, short x, short y, short how);
void fn_414e7d();
void redrawRect(ShortRect *rect);
void fn_414f01(InputItem *item);
void fn_414f17(InputItem *item);
void fn_456a64();
/* Mohawk engine */
short setMinimalReserve(short minimal);
short drawImageData(unsigned short *image, short x, short y, short mode);
short drawPixelData(short width, short height, short unknown, unsigned short flags, void *pixels,
                    short x, short y, short mode);
Palette *getPortPalette(); /* the current palette */
short setPaletteColors(Palette *palette, unsigned short first, unsigned short count, ColorBytes *colors);
basePort *newPort(short width, short height, short depth, Palette *palette);
short unlockPort(basePort *port);
short deletePort(basePort *port);
short setOrigin(short left, short top);
short clipPortToRect(const Rect &rect);
short setClip(short region);
short getClip(short region);
short copyPortBits(basePort *to, basePort *from, const Rect &fromRect, const Rect &toRect, short mode);
short lockPort(basePort *port); /* locks a port; non-zero on failure */
short invertRect(const Rect &rect);
short lineTo(short x, short y);
basePort *newWindowPort(const Rect &bounds, HWND window, Palette *palette);
Palette *setPortPalette(Palette *palette); /* the previous one */
short setClipRect(const Rect &rect);
short fillPortRect(const Rect &rect, Color color, short);
Color getForeColor();
Color setForeColor(Color color); /* the previous one */
unsigned short setPenMode(short mode); /* the previous one */
void mouseButtonDown(short button, long keys, long where);
short handleNextMessage();
void flushInput(short which);
void handleMessagesIgnoringInput();
void activateApp(long active);
void placeGamePort();
void destroyMainWindow();
void showError(const char *prefix, const char *format, va_list args);
void releaseControlKeys();
void fn_456b2e(short active);
void fn_46da64(short active);
void runClock(short running);
void fn_48b2d8(DisplayMode *mode);
void fn_48d480(DisplayMode *mode);
short initResources();
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

short closeResourceFile(long map, short compact, short force);

/*
 * The Mohawk engine
 */

/* A settings file read into memory (in a handle), in a list of open ones. */
class IniFile
{
public:
    short prev;
    short next;
    fileSpec path;
    short users;
    unsigned short size;
    unsigned short pos; /* where parsing has got to */
    short text; /* the contents, NUL-terminated */
};

/* The settings files' state. */
class IniState
{
public:
    short error; /* of the last call */
    short ready;
    fileSpec path; /* MOHAWK.INI (or .W32) in the program's directory */
    short files; /* the open settings files */
    short unknownA;
};

extern IniState iniState; /* @data 0x4b9b58 */

/* Files */
long fileSize(fileSpec *file); /* -1 on error */
short fileError();
void programDirectory(fileSpec *directory);
long openFile(fileSpec *file, short mode); /* 0 on error */
short readFile(long file, void *buffer, long *size);
short closeFile(long file, short);
short fileMissing(const fileSpec &file); /* 0 if it exists, else an error (0x2845 not found) */
void currentDirectory(fileSpec *directory);
short setCurrentDirectory(fileSpec *directory);
void fn_484994(fileSpec *directory); /* the directory at 0x4b9b9c, where .FOT files go */
short deleteFile(const fileSpec &file);
/* Calls `callback` for each file in the current directory. */
short forEachFile(FileCallback callback, void *data);

/*
 * Ports (QuickDraw's GrafPorts): C++ objects, handed around as their
 * addresses ("handles"; 0 and -1 are never valid), tagged 'Port'. The
 * virtual methods are Pascal like the rest of the engine, the destructor
 * __cdecl. Slots are named by index until their meaning is known.
 */
class basePort;

/* A palette the engine manages, shared by ports. */
class Palette
{
public:
    long magic; /* 'Palt' */
    Palette *next; /* +4: palettes form a ring (graphics.palettes) */
    Palette *prev; /* +8 */
    basePort *ports; /* +0xc: the ports using it */
    short realized; /* +0x10: cleared when the system palette changes */
    short changed; /* +0x12: colours set since it was realized */
    short foreground; /* +0x14: realized as the foreground palette */
    unsigned short first; /* +0x16: the first colour the game may set */
    HPALETTE hpal; /* +0x18 */
    PALETTEENTRY entries[256]; /* +0x1c */
};

class basePort
{
public:
    virtual __cdecl ~basePort(); /* 0 */
    virtual short v1(basePort *to, const Rect *fromRect, const Rect *toRect, short mode, long);
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void depthChanged(); /* 6: the display's depth changed (while locked) */
    virtual void v7();
    virtual void prepare(); /* 8: before GDI calls on dc */
    virtual void paletteChanged(); /* 9: while locked */
    virtual void v10();
    virtual void v11();
    virtual short useFont(Font *font); /* 12 */
    virtual void v13();
    virtual short setColor(Color color, short mode); /* 14: the pen */
    virtual void v15();
    virtual short drawPixels(const Rect &bounds, short width, short height, short unknown,
                             unsigned short flags, void *pixels, short mode, long); /* 16 */
    virtual void v17();
    virtual HBRUSH brush(Color color); /* 18: a new brush */
    virtual void v19();
    virtual void v20();
    virtual unsigned short nearestIndex(RGBColor color); /* 21 */
    virtual void v22();
    virtual RGBColor paletteColor(unsigned short index); /* 23 */
    virtual short init(); /* 24: after construction; non-zero on failure */
    virtual short lock(); /* 25 */
    virtual short fillRect(const Rect &rect, Color color, short); /* 26 */
    virtual void v27();
    virtual void v28();
    virtual short fillRgn(short region, HBRUSH brush, short); /* 29 */
    virtual void v30();
    virtual void release(); /* 31: before deleting */
    virtual void v32();
    virtual Palette *setPalette(Palette *palette); /* 33: the previous one */
    virtual void unlock(); /* 34 */
    virtual void v35();
    virtual void v36();

    static void *operator new(size_t size); /* 0x487f56: zeroed */
    short setFrame(Rect *rect, Pt origin, Pt size); /* 0x486407 */
    static void operator delete(void *block);

    long unknown4; /* 'Port' */
    basePort *next; /* +8: in graphics.ports */
    char unknownC[8];
    long kind; /* +0x14: 5 a window */
    Rect unknown18;
    char unknown20[0xc];
    Pt unknown2c;
    char unknown30[0xc];
    Palette *palette; /* +0x3c */
    char unknown40[4];
    basePort *nextOnPalette; /* +0x44: in palette->ports */
    char unknown48[4];
    Color backColor; /* +0x4c */
    Font *font; /* +0x50 */
    char unknown54[4];
    Color foreColor; /* +0x58 */
    char unknown5c[2];
    short mode; /* +0x5e */
    short clip; /* +0x60: a region */
    short clipChanged; /* +0x62 */
    unsigned short locks; /* +0x64 */
    char unknown66[6];
    HDC dc; /* +0x6c */
    char unknown70[0x38];
    short unknownA8; /* +0xa8: subtracted from text widths */
    char unknownAA[0x16];
};

class displayPort : public basePort
{
};

/* A port in memory, in the display's format. */
class memoryPort : public displayPort
{
public:
    __cdecl memoryPort(short width, short height); /* 0x48c774 */
    char unknownC0[4];
};

/* A device-independent bitmap of any depth. */
class DIBPort : public basePort
{
public:
    __cdecl DIBPort(short width, short height, short depth); /* 0x48a720 */
    char unknownC0[8];
};

/* An 8-bit DIB. RTTI names DIBPort as its base, but it is smaller (0xc6
   bytes) than DIBPort (0xc8); modelled on basePort until its fields are
   known. */
class DIB8Port : public basePort
{
public:
    __cdecl DIB8Port(short width, short height); /* 0x489be4 */
    char unknownC0[6];
};

/* A window's port: two more slots. */
class windowPort : public displayPort
{
public:
    __cdecl windowPort(const Rect &bounds, HWND window); /* 0x48db64 */
    virtual short v37();
    virtual short v38();
    char unknownC0[8];
};

/* A Mac 'CURS' resource: 16x16 image and mask, hot spot (big-endian). */
struct MacCursor
{
    unsigned short data[16];
    unsigned short mask[16];
    short hotV; /* +0x40 */
    short hotH; /* +0x42 */
};

/* A font (0x38 bytes and its name), in a ring (graphics.fonts). */
class Font
{
public:
    long magic; /* 'Font' */
    Font *next;
    Font *prev;
    unsigned short users; /* +0xc: can't be disposed of while used */
    char unknownE[4];
    short unknown12;
    short unknown14;
    char name[0x22]; /* +0x16: its face ("SYSTEM" by default), at most 31 characters */
};

/* A font file added for the game (graphics.fontFiles). */
struct FontFile
{
    FontFile *next;
    short trueType; /* made into a .FOT to add it */
    short created; /* the .FOT was created (and is deleted on removal) */
    char path[1]; /* +8: what was added, NUL-terminated */
};

/* A decompressor DLL listed in the settings (graphics.decompressors). */
struct Decompressor
{
    Decompressor *next;
    char name[8]; /* its key in [Graphics.Decompressors] */
    HMODULE module; /* +0xc */
    FARPROC proc; /* +0x10: GFXXDECPROC */
};

/* The drawing engine's state, cleared by openGraphicsEngine. */
struct GraphicsState
{
    short error; /* of the last call */
    short ready; /* +2 */
    short active; /* +4: the application is active */
    char unknown6[2];
    ActivateHook previousHook; /* +8 */
    WNDPROC windowProc; /* +0xc: the main window's, before openGraphicsEngine */
    unsigned short depth; /* +0x10: the display's bits per pixel (at most 24) */
    char unknown12[2];
    HCURSOR cursor; /* +0x14 */
    short standardCursor; /* +0x18: the cursor is a system one (not to be destroyed) */
    unsigned char cursorData[0x44]; /* +0x1a: the MacCursor it was made from */
    char unknown5e[2];
    short cursorFix; /* +0x60: [Graphics] fEnableCursorFix: hide the cursor by making it blank */
    short cursorLevel; /* +0x62: with cursorFix, as ShowCursor counts */
    HCURSOR savedCursor; /* +0x64: with cursorFix, the real cursor while it is hidden */
    unsigned short realizing; /* +0x68: in realizePalette (no WM_PALETTECHANGED broadcasts) */
    short minimalReserve; /* +0x6a: keep only black and white */
    unsigned short paletteReserved; /* +0x6c: system colours kept (half at each end) */
    char unknown6e[2];
    short takeStatic; /* +0x70: take over the static colours while active */
    char unknown72[2];
    UINT systemPaletteUse; /* +0x74 */
    PALETTEENTRY systemColors[20]; /* +0x78: the static colours (10 at each end) */
    FontFile *fontFiles; /* +0xc8 */
    Decompressor *decompressors; /* +0xcc */
    basePort *currentPort; /* +0xd0 */
    basePort *ports; /* +0xd4 */
    Font *fonts; /* +0xd8: a ring */
    Font *defaultFont; /* +0xdc */
    Palette *palettes; /* +0xe0: a ring */
    Palette *defaultPalette; /* +0xe4 */
};

extern GraphicsState graphics; /* @data 0x4b9ba0 */
extern int sysColorIndices[21]; /* @data 0x4a8b38: the colours the static palette entries show */
extern COLORREF monoSysColors[21]; /* @data 0x4a8b8c: them in black and white (SYSPAL_NOSTATIC) */
extern COLORREF savedSysColors[21]; /* @data 0x4b9c88 */

/* Display modes and the system palette. The engine uses Windows 95's
   DEVMODE (0x94 bytes); Borland C++ 4.5's headers have the older one (0x7c)
   and lack the display-settings constants. */
#ifndef DM_BITSPERPEL
#define DM_BITSPERPEL 0x00040000L
#define DM_PELSWIDTH 0x00080000L
#define DM_PELSHEIGHT 0x00100000L
struct DeviceMode : DEVMODE
{
    DWORD dmICMMethod;
    DWORD dmICMIntent;
    DWORD dmMediaType;
    DWORD dmDitherType;
    DWORD dmReserved1;
    DWORD dmReserved2;
};
#else
typedef DEVMODE DeviceMode;
#endif
long changeDisplaySettings(DeviceMode *mode, unsigned long flags);
HPALETTE createPalette(PALETTEENTRY *entries);
BOOL enumDisplaySettings(const char *device, unsigned long index, DeviceMode *mode);
BOOL polygon(HDC dc, const Point *points, unsigned short count);
UINT setSystemPaletteUse(HDC dc, UINT use);
void graphicsActivate(short active);
short findDisplayMode(const DisplayMode *want, DeviceMode *best);
void currentDisplayMode(DeviceMode *mode);
short openGraphicsEngine(const DisplayMode *mode, short change); /* an error code */
short addFont(const char *name, void *directory);
LRESULT CALLBACK graphicsWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
short graphicsBufferSize();
void closeGraphicsEngine();
void getDisplayMode(DisplayMode *mode);
short setDisplayMode(const DisplayMode *mode);
Palette *newPalette(unsigned short count, ColorBytes *colors); /* colours `kind` bit 0: PC_RESERVED */
short deletePalette(Palette *palette);
void disposePalette(Palette *palette);
Palette *paletteHandle(Palette *palette); /* 0x48d5df */
Font *newFont(const char *name, short, short);
short disposeFont(Font *font);
Font *fontObject(Font *font); /* 0x48d555 */
Font *fontHandle(Font *font);
Palette *checkPalette(Palette *palette, short kind); /* 0x48d779 */
basePort *portHandle(basePort *port); /* 0x48d9bd */
/* An image resource: this header (big-endian, as stored), then its pixels
   or, compressed, an LzImage or ExternalImage. */
struct ImageHeader
{
    short width;
    short height;
    short rowBytes; /* negative for top-down rows */
    unsigned short flags; /* bits 0-3 depth (0-4: 1, 4, 8, 16, 24 bits), 8-11 compression
                             (1 LZ, 15 a decompressor DLL), 12 16-bit pixels byte-swapped */
};

/* After an ImageHeader: Mohawk's LZ compression. */
struct LzImage
{
    unsigned long size; /* decompressed */
    long unknown4;
    unsigned short window; /* the ring buffer's size: 256 to 4096 */
    unsigned char data[1];
};

/* After an ImageHeader: compressed by a decompressor DLL. */
struct ExternalImage
{
    char name[8]; /* its key in [Graphics.Decompressors] */
    unsigned long size; /* decompressed */
    unsigned long paramSize;
    unsigned char params[1];
};

/* A decompressor DLL's GFXXDECPROC. */
typedef short (*DecompressProc)(void *out, unsigned long size, short width, short height,
                                short depth, long rowBytes, void *params,
                                unsigned long paramSize, unsigned short);

short decompressImage(short handle);
void swapWords(void *data, unsigned long count);
void lzDecompress(unsigned char *dest, const unsigned char *source, unsigned long size,
                  unsigned char *ring, short bits);
short setCursorShape(const MacCursor *cursor); /* 0-3: arrow, cross, I-beam, wait */
void drawPackedPixels(long offset, unsigned char *bits, long rowBytes, ShortRect bounds, short x, short y,
                      ShortRect *clip, unsigned short width, unsigned short height,
                      const unsigned char *data, short transparent);
unsigned char packedPixel(const unsigned char *data, unsigned short x, unsigned short y);
basePort *checkPort(basePort *port, short kind);
basePort *portObject(short kind);
short setPortError(short error);
short getPortError();
short eraseRgn(short region);
short frameRect(const ShortRect &rect);
Font *getFont();
unsigned short nearestPaletteIndex(Palette *palette, RGBColor color);
unsigned short textWidth(const char *text, unsigned short length); /* length 0xffff: NUL-terminated */

/*
 * Memory, as on the Mac: relocatable blocks by handle (a short: an index
 * into a table of entries), and fixed ones by pointer. Each block starts
 * with a Chunk header, and is reached through a "master pointer" (a Block):
 * a moveable Windows global handle, whose first word points at the memory,
 * or a fixed block holding a pointer to just after itself. Handles' blocks
 * can be purged when memory runs short, unless locked.
 */
struct Chunk
{
    unsigned short magic; /* 'BM' */
    unsigned short handle; /* its handle, for a handle's block */
    unsigned long size : 31; /* of the data after this header */
    unsigned short moveable : 1; /* a handle's block */
};
typedef Chunk **Block;

struct HandleEntry
{
    unsigned short locks : 7;
    unsigned short age : 4; /* 15 when used, counted down by purges */
    unsigned short state : 2;
    unsigned short used : 1;
    unsigned short keep : 1; /* never purge */
    unsigned short purgeable : 1;
    union {
        Block block; /* 0 if empty or purged */
        unsigned short nextFree; /* unused entries form a list */
    };
    short unknown6;
};

struct HandleTable
{
    unsigned short freeList;
    unsigned short count;
    HandleEntry entries[1];
};

/* Asked before purging a handle's block (0: don't). */
typedef short (*PurgeProc)(short handle, short purpose);
/* Asked for memory when an allocation fails (non-zero: try again). */
typedef short (*GrowProc)(unsigned long size, short error);

struct MemoryState
{
    short error; /* of the last call */
    short ready;
    unsigned short unknown4; /* fn_48f260 counts it up to 3 */
    short purgeEnabled;
    PurgeProc purgeProc; /* +8 */
    GrowProc growProc; /* +0xc */
    HandleTable *table; /* +0x10 */
};

extern MemoryState heap; /* @data 0x4b9cdc */

short newHandle(unsigned long size); /* 0x48e5ec */
void *newPtr(unsigned long size);
unsigned long availableMemory(unsigned long size);
short disposePtr(void *pointer);
unsigned long availableVirtualMemory();
unsigned long handleSize(short handle);
unsigned short handleLocks(short handle);
unsigned short handleState(short handle);
unsigned long ptrSize(void *pointer);
void getMemoryInfo(MemoryInfo *info);
void *fn_48ea00(short handle);
short recoverMemory(short error, unsigned long size);
Block allocBlock(unsigned long size, unsigned short moveable);
short freeBlock(Block block);
Block resizeBlock(Block block, unsigned long size);
short resizeFixedBlock(Chunk *chunk, unsigned long size);
Block blockOf(Chunk *chunk);
short initMemory(unsigned long size, unsigned short handles);
short growHandleTable(unsigned short count);
short canPurge(HandleEntry *entry, short purpose);
short memoryBufferSize();
void closeMemory();
short growZone(unsigned long size, short error);
unsigned short entryIndex(HandleEntry *entry);
void fn_48ef15(Block block);
void fn_48ef1c(Block block);
void moveMemory(void *to, const void *from, unsigned long size);
short lockPtr(void *pointer);
short unlockPtr(void *pointer);
unsigned long purgeMemory(unsigned long needed, short purpose);
short setPurgeEnabled(short enabled);
void *resizePtr(void *pointer, unsigned long size);
short isPointer(void *pointer);
short fn_48f260();
void fillMemory(void *to, unsigned char value, unsigned long size);
GrowProc setGrowProc(GrowProc proc);
unsigned short setHandleLocks(short handle, unsigned short locks);
short setHandleState(short handle, unsigned short state);
PurgeProc setPurgeProc(PurgeProc proc);
unsigned short setPurgeable(short handle, short purgeable);
short swapHandleData(short a, short b);
short setMemError(short error);
HandleEntry *handleEntry(unsigned short handle);
short validHandle(unsigned short handle, short);
void *handleData(short handle); /* 0x48f5bc */
void *lockHandle(short handle); /* 0x48e96c */
short unlockHandle(short handle); /* 0x48f550 */
short setHandleSize(short handle, unsigned long size); /* an error code */
short disposeHandle(short handle); /* 0x48e71c */
short memError(); /* 0x48e80c */

/*
 * Resources: Mohawk archives (ScummVM's name; `MHWK` files), read through
 * resource maps. Everything in a file is big-endian. A resource's ID is a
 * long: its file-table index (from 1) in the high word, its map's handle in
 * the low word. Errors of the last call are in resources.error.
 */

/* An archive's header, as stored (byteSwapHeader converts it). */
struct MohawkHeader
{
    unsigned long tag; /* 'MHWK' */
    unsigned long size; /* of the rest of the file */
    unsigned long type; /* 'RSRC' */
    unsigned short version; /* 0x100 */
    unsigned short compacted; /* 0 if the file has no unused space */
    unsigned long fileSize;
    unsigned long directoryOffset;
    unsigned short fileTableOffset; /* from the directory: its size */
    unsigned short fileTableSize;
};

/* The directory: a type table, then each type's resource and name tables
   (offsets from the directory's start), then the names. */
struct TypeEntry
{
    unsigned long type;
    unsigned short resources; /* ResourceTable */
    unsigned short names; /* NameTable */
};

struct Directory
{
    unsigned short names; /* where the names start */
    unsigned short count;
    TypeEntry types[1];
};

struct ResourceRef
{
    unsigned short id;
    unsigned short index; /* into the file table, from 1 */
};

struct ResourceTable
{
    unsigned short count;
    ResourceRef entries[1]; /* by id */
};

struct NameRef
{
    unsigned short name; /* from the names' start */
    unsigned short index;
};

struct NameTable
{
    unsigned short count;
    NameRef entries[1]; /* by name */
};

/* FileTableEntry flags */
#define RESOURCE_MODIFIED 0x01
#define RESOURCE_LOADED 0x02
#define RESOURCE_PURGED 0x04 /* its handle's block has been purged */
#define RESOURCE_08 0x08 /* can't be written */
#define RESOURCE_DELETED 0x10
#define RESOURCE_PRELOAD 0x20
#define RESOURCE_LOCKED 0x40
#define RESOURCE_PURGEABLE 0x80

struct FileTableEntry
{
    unsigned long offset;
    unsigned short sizeLow;
    unsigned char sizeHigh;
    unsigned char flags;
    short handle; /* stored as 0; the data's handle once loaded */
};

struct FileTable
{
    unsigned short countHigh;
    unsigned short count;
    FileTableEntry entries[1];
};

struct PreloadRequest;

/* An open archive, in a handle (maps form a list, and those with preloads
   pending a ring). */
struct ResourceMap
{
    unsigned long tag; /* 'RMap' */
    short next; /* the map list */
    short prev;
    short nextPreload; /* the ring of maps with preloads */
    short prevPreload;
    unsigned short users;
    short unknownE;
    long file;
    short async; /* reads can go on in the background */
    short readOnly;
    short directory; /* a Directory, in a handle */
    short counters; /* per entry: loaded and preload counts (2 bytes) */
    unsigned short preloads;
    short unknown1E;
    PreloadRequest *preload; /* the next to run */
    PreloadRequest *lastPreload;
    unsigned long fileSize;
    unsigned long directoryOffset;
    unsigned long directorySize; /* with the file table */
    unsigned short compacted;
    short dirty;
    unsigned short modified; /* resources modified */
    unsigned short unknown3A;
    FileTable fileTable;
};

/* A preload request's provider, told of `event` for resource `id`. */
typedef void *(*PreloadProc)(long event, long id, void *data);

/* PreloadProc events */
#define PRELOAD_GET_BUFFER 0
#define PRELOAD_RELEASE_BUFFER 1
#define PRELOAD_CANCELLED 2
#define PRELOAD_DONE 3
#define PRELOAD_FAILED 4

/* A resource being read ahead of its use, in a ring per map ordered by
   position in the file. */
struct PreloadRequest
{
    unsigned long tag; /* 'RQRq' */
    PreloadRequest *next;
    PreloadRequest *prev;
    long id;
    PreloadProc proc;
    long data; /* the provider's; the default one keeps a handle here */
    char *buffer;
    unsigned long offset; /* in the resource */
    unsigned long length;
    unsigned long done;
    unsigned long position; /* of the resource in the file */
    short finished;
    unsigned short busy;
    PreloadProc callback; /* the default provider's caller's */
    long callbackData;
};

struct ResourceState
{
    short error; /* of the last call */
    short ready;
    short shareReadOnly; /* [Resource] fShareReadOnly */
    short unknown6;
    PurgeProc previousPurgeProc;
    unsigned short handleState; /* resources' handles get it */
    short buffer; /* for copying within files */
    short currentMap;
    short maps; /* the list */
    unsigned short preloads;
    unsigned short syncPreloads;
    unsigned short asyncPreloads;
    short preloadMap; /* the ring */
    long preloadThread;
    long systemMap; /* SYSTEM.W32 or SYSTEM.MHK */
};

extern ResourceState resources; /* @data 0x4b9d8c */

/* Resource IDs */
long makeResourceId(short map, unsigned short index); /* 0x492a25 */
unsigned long resourceIndex(long id);
long resourceMapHandle(long id); /* the low word */
ResourceMap *resourceMap(long handle);
short findEntry(long id, ResourceMap **map, FileTableEntry **entry);
unsigned short __cdecl lowWord(unsigned long value);
unsigned short __cdecl highWord(unsigned long value);
unsigned long __cdecl makeLong(unsigned short low, unsigned short high);
short setResourceError(short error);
unsigned long __cdecl byteSwapLong(unsigned long value); /* big-endian to native and back */
unsigned short __cdecl byteSwapShort(unsigned short value);
short writeResource(long id);
long findResourceByHandle(short handle);
short releaseResource(long id, short release);
short resourceError();
short getResourceInfo(long id, long *file, unsigned long *offset, unsigned long *size);
unsigned long resourceSize(long id);
short loadResource(long id, short use);
unsigned short resourceHandle(long id); /* 0 if not loaded, 0xffff on error */
/* A resource's use counts: loads, then preloads. */
unsigned char *resourceCounts(ResourceMap *map, unsigned short index);
short disposeResourceHandle(short handle);
void attachHandle(FileTableEntry *entry, short handle);
short readResourceBytes(long id, void *buffer, unsigned long *size, unsigned long offset);
short finishPreloads(long id);
short purgeResource(short handle, short purpose);
long openResourceFile(const fileSpec &file, short readOnly);
void byteSwapHeader(MohawkHeader *header);
void byteSwapDirectory(Directory *directory, short fromFile);
void byteSwapFileTable(FileTable *table, short fromFile);
/* Runs a map's preloads for up to `time` ms (0xffffffff: all of them). */
unsigned short runMapPreloads(long map, unsigned long time);
PreloadRequest *startPreload(long id, PreloadProc callback, long data);
short disposePreload(PreloadRequest *request);
short cancelPreloads(long id);
unsigned short servicePreloads(unsigned long time);
unsigned short stepPreload(PreloadRequest *request, unsigned long time); /* 0xffff on error */
PreloadRequest *findPreload(ResourceMap *map, long id, short idle);
void insertPreload(ResourceMap *map, PreloadRequest *request);
void *defaultPreloadProc(long event, long id, short *handle);
PreloadRequest *newPreloadRequest(long id, PreloadProc proc, long data, unsigned long length,
                                  unsigned long offset);
void seekPreloads(ResourceMap *map, unsigned long position);
void preloadThread(long);
PreloadRequest *checkRequest(PreloadRequest *request);
void *callProvider(PreloadRequest *request, long event);
long comparePreloads(PreloadRequest *a, PreloadRequest *b);
short removeFromDirectory(short directory, unsigned short index);
short addToDirectory(short directory, unsigned short index, unsigned long type, unsigned short id,
                     const char *name);
short resourceBufferSize();
void closeResources();
unsigned short setResourcePurgeable(long id, short purgeable);
int __cdecl compareEntries(const void *a, const void *b); /* file-table entries, for qsort */
long findResource(unsigned long type, unsigned short id, long map);
short writeResourceMap(long handle);
short writeMapHeader(ResourceMap *map); /* 0x492483 */
short writeResourceData(long id, const void *buffer, unsigned long length, unsigned long offset);
short copyFileBytes(ResourceMap *map, unsigned long to, unsigned long from, unsigned long length);

/* The files resources come from (the async file API) */
struct VolumeInfo
{
    char unknown0[0x26];
    short async; /* reads can go on in the background */
    char unknown28[4];
    short readOnly;
    char unknown2E[10];
};

short lockFile(long file, long timeout); /* 0x4860cc: 0, or 300 if it timed out */
void unlockFile(long file); /* 0x484d98 */
unsigned long seekFile(long file, unsigned long offset, short whence); /* 0x484dc4: -1 on error */
short writeFile(long file, const void *buffer, long *size); /* 0x48610c */
short setFileSize(long file, long size); /* 0x484f3c */
unsigned long fileLength(long file); /* 0x4845fc */
short fileSpecOf(long file, fileSpec *spec); /* 0x484934 */
short volumeInfo(long volume, VolumeInfo *info); /* 0x4849ac */
long fn_4850d4(long value); /* 0x4850d4: the previous value */

/*
 * Timers: multimedia timer events ('TEvt'), whose procedures run under the
 * timers' lock. Delays longer than the timer device allows are counted down
 * in steps; periodic ones are rescheduled to catch up when late.
 */
typedef void (*TimerProc)(long timer, long data);

struct TimerEvent
{
    unsigned long tag; /* 'TEvt' */
    TimerEvent *next; /* the active list, or the free list */
    TimerEvent *prev;
    short active;
    short unknownE;
    unsigned long remaining; /* until it fires */
    unsigned long period; /* 0: fire once */
    TimerProc proc;
    long data;
    long unknown20;
    Deferred call; /* runs fireTimer under the lock */
    unsigned long id; /* the multimedia timer */
    unsigned long interval; /* of the multimedia timer */
    unsigned long due;
    short oneShot; /* the multimedia timer is one-shot (adjusted) */
    short unknown46;
};

struct TimerState
{
    short error; /* of the last call */
    short ready;
    DeferLock lock;
    TIMECAPS caps;
    TimerEvent *free;
    TimerEvent *active;
    TimerEvent *current; /* firing */
};

extern TimerState timerState; /* @data 0x4b9db0 */

long newTimer(unsigned long delay, unsigned long period, TimerProc proc, long data);
short killTimer(long timer);
void freeTimer(TimerEvent *event);
TimerEvent *timerEvent(long timer);
void lockTimers();
void unlockTimers();
void CALLBACK timerCallback(UINT id, UINT message, DWORD data, DWORD, DWORD);
void fireTimer(void *event);
short timerError();
short initTimers();
short timerBufferSize();
void closeTimers();
unsigned short startTimer(TimerEvent *event);
short setTimerError(short error);
long timerId(TimerEvent *event);

/*
 * Sounds: MIDI and wave objects (AudioObject, tagged 'AObj'), handed around
 * as handles (their addresses) and kept in a list. Errors of the last call
 * are in sound.error.
 */

/* Told when a sound starts (4) or stops (5), and of its progress. */
typedef void (*SoundNotify)(long sound, SoundNotice *notice, long cookie);

class AudioObject
{
public:
    virtual void __cdecl release() = 0; /* before it's freed */
    virtual short __cdecl activate(short active);
    virtual short __cdecl openDevice() = 0;
    virtual short __cdecl setDeviceRate(long rate) = 0;
    virtual short __cdecl setDeviceVolume(long volume) = 0;
    virtual short __cdecl startDevice(short playing) = 0;
    virtual void __cdecl haltDevice() = 0;
    virtual void __cdecl closeDevice() = 0;
    virtual long __cdecl deviceHandle() = 0; /* its map or wave device */
    virtual long __cdecl position() = 0;
    virtual void __cdecl pause() = 0;
    virtual short __cdecl open(unsigned short device);
    virtual void __cdecl endLoop() = 0;
    virtual void __cdecl resetLoop() = 0;
    virtual void __cdecl resume() = 0;
    virtual short __cdecl seek(long position) = 0;
    virtual short __cdecl setText(const char *text, unsigned short length) = 0;
    virtual short __cdecl setRate(long rate);
    virtual short __cdecl setVolume(long volume);
    virtual short __cdecl play(SoundNotify notify, long cookie);
    virtual void __cdecl stop();
    virtual void __cdecl close();

    unsigned long tag; /* 'AObj' */
    long kind; /* 0 MIDI, 1 wave */
    AudioObject *next;
    AudioObject *prev;
    long rate; /* speed (MIDI: tempo scale) */
    long volume; /* MIDI: the velocity curve */
    long duration; /* ms (MIDI: ticks) */
    short unknown20;
    short endingLoop;
    short active; /* sounds are on (the application is active) */
    short playing;
    short started; /* play() has started it */
    short isOpen;
    unsigned short device;
    short unknown2E;
    SoundNotify notify;
    long cookie;
    DeferLock lock;
};

struct SoundState
{
    short error; /* of the last call */
    short ready;
    short active;
    short driverOpen;
    AudioObject *objects;
    unsigned short midiDevice;
    short cacheMidiDevice; /* keep the default MIDI device open */
    struct MidiMap *midiCache;
    unsigned short waveDevice;
    short cacheWaveDevice;
    long waveCache;
    short translateWaveRate; /* [Audio] fTranslateWaveRateOnError */
    short unknown1E;
};

extern SoundState sound; /* @data 0x4b9b08 */

short setSoundsActive(short active); /* 0x4764bc: non-zero on failure */
short disposeSound(long sound);
short forEachSound(long kind, unsigned short device, short (*proc)(AudioObject *object, long data),
                   long data);
void chooseMidiDevice();
void chooseWaveDevice();
short findIniEntry(fileSpec *file, const char *section, char *entry, unsigned short size,
                   const char *name, unsigned short version, unsigned short, unsigned short);
unsigned short soundDevice(long sound);
long soundDeviceHandle(long sound);
long soundDuration(long sound);
short soundError(); /* 0x476bb4 */
long soundRate(long sound);
long soundPosition(long sound);
unsigned short soundFlags(long sound);
long soundKind(long sound);
long soundVolume(long sound);
short initSound(); /* 0x476d0a */
short pauseSound(long sound);
short openSound(long sound, unsigned short device); /* 0xffff: the default device */
short soundBufferSize();
void closeSounds();
short endSoundLoop(long sound);
short resumeSound(long sound);
short seekSound(long sound, long position);
short setSoundText(long sound, const char *text, unsigned short length);
short setSoundRate(long sound, long rate);
short setSoundVolume(long sound, long volume);
/* Starts a sound; its owner hears about it through `notify`. Non-zero on failure. */
short playSound(long sound, SoundNotify notify, long cookie);
short stopSound(long sound);
short closeSound(long sound);
unsigned short setMidiDevice(unsigned short device); /* the previous one */
unsigned short setWaveDevice(unsigned short device);
unsigned short openWaveOutDevice(long *out, unsigned short device, PCMWAVEFORMAT *format, long, long,
                        long flags);
AudioObject *audioObject(long sound); /* 0 if it isn't one */
unsigned short __cdecl makeWord(unsigned char low, unsigned char high);
long newSound(short data); /* from a Mohawk MIDI or WAVE in a handle */
long newStreamedSound(long resource, long);
/* Called but not decompiled yet */
short openWaveOut(long *out, unsigned short device, PCMWAVEFORMAT *format, long, long,
                  long flags); /* 0x47c712 */
short closeWaveOut(long wave); /* 0x47c3d4 */
short getWaveCaps(unsigned short device, void *caps, long size); /* 0x47c432 */
short fn_47a074(short open); /* MMSYSERR_NOTSUPPORTED */
unsigned short initMidi(); /* 0x47a07f */
void closeMidi(); /* 0x47a0c0 */
short fn_47c62c();
void fn_47c995();
long __cdecl parseNumber(const char *text); /* 0x47a066 */
short setSoundError(short error); /* 0x47de96 */
void __cdecl notifySound(AudioObject *object, SoundNotice *notice); /* 0x47e0d3 */
AudioObject *__cdecl newMidiSound(short data); /* 0x478f0b */
void *__cdecl operator new(size_t size, void *where); /* 0x47dea7: zeroed */
AudioObject *__cdecl newWaveSound(short data); /* 0x47a28d */
AudioObject *__cdecl newStreamedWave(long resource, long file, long); /* 0x47cd5c */

/*
 * The MIDI mapper: midiOut-style calls on "maps", which share a device
 * (opened once) and each remap channels, scale velocities and track what
 * they've cached, as [MidiMap.TargetDevices] and [MidiMap.TargetDeviceInfo]
 * describe the device. Results are MMSYSERR codes.
 */
struct MidiMap;

struct MidiDevice
{
    MidiDevice *next;
    unsigned short id;
    short unknown6;
    MIDIOUTCAPS caps;
    short target; /* its entry in [MidiMap.TargetDeviceInfo] */
    unsigned short drumChannel;
    unsigned short channels[16]; /* where each channel goes */
    short muted[16];
    long resetSysex; /* SYSX resources sent on opening and closing */
    long closeSysex;
    unsigned short users;
    short unknown8A;
    HMIDIOUT out;
    unsigned short unknown90;
    short unknown92;
    MidiMap *maps; /* a ring */
};

struct MidiMap
{
    unsigned long tag; /* 'MMap' */
    MidiDevice *device;
    MidiMap *next;
    MidiMap *prev;
    short minimal; /* opened without the mapping state (only short messages) */
    short unknown12;
    long table; /* the velocity curve's */
    unsigned char velocities[128];
    short channelMuted[16]; /* by the sounds' own sysex */
    unsigned char notes[16][128]; /* notes on, by channel and key */
    WORD drumCache[128];
    WORD patchCache[128];
};

/* A stretch of a wave sound played as one buffer: split at cue points and
   at the loop's ends. */
class WaveSound;
struct WaveBlock
{
    Deferred call; /* runs waveBlockDone */
    WaveSound *sound;
    unsigned long start; /* in samples */
    unsigned long length;
    unsigned char *cue; /* the cue point it starts at */
    short prepared;
    short unknown26;
    WAVEHDR header;
};

/* A wave sound: a Mohawk WAVE file (a Data chunk of PCM samples, perhaps
   looping, and a Cue# list of named positions) played through the
   engine's wave output (wavebuf). */
class WaveSound : public AudioObject
{
public:
    virtual void __cdecl release();
    virtual short __cdecl openDevice();
    virtual short __cdecl setDeviceRate(long rate);
    virtual short __cdecl setDeviceVolume(long volume);
    virtual short __cdecl startDevice(short paused);
    virtual void __cdecl haltDevice();
    virtual void __cdecl closeDevice();
    virtual long __cdecl deviceHandle();
    virtual long __cdecl position();
    virtual void __cdecl pause();
    virtual void __cdecl endLoop();
    virtual void __cdecl resetLoop();
    virtual void __cdecl resume();
    virtual short __cdecl seek(long position);
    virtual short __cdecl setText(const char *text, unsigned short length);
    virtual short __cdecl play(SoundNotify notify, long cookie);

    void __cdecl buildBlocks();
    void __cdecl unprepare();
    unsigned long __cdecl positionAt(unsigned long sample);

    long wave;
    short data; /* the file's handle */
    short unknown4E;
    unsigned long *file;
    unsigned char *cues; /* Cue# */
    unsigned long start; /* in samples */
    long samplesPerMs; /* fixed-point */
    long msPerSample;
    unsigned long base; /* the device's position 0, in samples */
    short resetting;
    short loopDone;
    unsigned long loopAdjust;
    WaveBlock *loopBlock;
    WaveBlock *afterLoop;
    WAVEHDR loopHeader; /* from a position inside the loop */
    short loopHeaderPrepared;
    unsigned short blockAlign;
    unsigned long sampleCount;
    unsigned short sampleRate;
    unsigned char bitsPerSample;
    unsigned char channels;
    unsigned short encoding;
    unsigned short loops; /* 0xffff: forever */
    unsigned long loopStart;
    unsigned long loopEnd;
    unsigned char *samples;
    unsigned short blockCount;
    short unknownB6;
    WaveBlock blocks[1];
};

void waveBlockDone(void *block); /* 0x47ae62 */
void CALLBACK waveCallback(long wave, UINT message, DWORD instance, DWORD, DWORD); /* 0x47ae14 */
short wavebufPause(long wave); /* 0x47c94b */
short wavebufPrepareHeader(long wave, WAVEHDR *header, unsigned short size); /* 0x47c96b */
short wavebufReset(long wave); /* 0x47c9c8 */
short wavebufRestart(long wave); /* 0x47c9e8 */
short wavebufSetPlaybackRate(long wave, long rate); /* 0x47ca52 */
short wavebufSetVolume(long wave, long volume); /* 0x47ca77 */
short wavebufUnprepareHeader(long wave, WAVEHDR *header, unsigned short size); /* 0x47ca9c */
short wavebufWrite(long wave, WAVEHDR *header, unsigned short size); /* 0x47cac6 */
short wavebufGetPosition(long wave, MMTIME *time, unsigned short size); /* 0x47c5dd */
short wavebufBreakLoop(long wave); /* 0x47c3b4 */

/* The engine uses Windows 95's MIDIHDR (0x40 bytes, with the streaming
   fields); Borland C++ 4.5's headers have the older one (0x1c). */
#ifndef MHDR_ISSTRM
struct MidiHeader : MIDIHDR
{
    DWORD dwOffset;
    DWORD dwReserved[8];
};
#else
typedef MIDIHDR MidiHeader;
#endif

/* A Mohawk MIDI file's track, as the sequencer plays it. */
struct MidiTrack
{
    unsigned char *start;
    unsigned char *cursor;
    long delta; /* ticks to its next event */
    short done;
    unsigned char status; /* running status */
    char unknownF;
};

/* A MIDI sound: a Mohawk MIDI file (an MThd header, MTrk tracks, and Key#
   and Prg# lists of the drum keys and patches to cache) played through a
   MIDI map by a timer. Markers "setup end", "loop start" and "loop end#n"
   control looping; cue points are passed to the owner. */
class MidiSound : public AudioObject
{
public:
    virtual void __cdecl release();
    virtual short __cdecl openDevice();
    virtual short __cdecl setDeviceRate(long rate);
    virtual short __cdecl setDeviceVolume(long volume);
    virtual short __cdecl startDevice(short playing);
    virtual void __cdecl haltDevice();
    virtual void __cdecl closeDevice();
    virtual long __cdecl deviceHandle();
    virtual long __cdecl position();
    virtual void __cdecl pause();
    virtual void __cdecl endLoop();
    virtual void __cdecl resetLoop();
    virtual void __cdecl resume();
    virtual short __cdecl seek(long position);
    virtual short __cdecl setText(const char *text, unsigned short length);
    virtual short __cdecl play(SoundNotify notify, long cookie);

    short __cdecl cachePatches(short cache);
    void __cdecl advance(unsigned short ticks, short play, short notify);
    void __cdecl setTempo(unsigned long tempo);
    unsigned short __cdecl nextStep();
    short __cdecl seekTo(unsigned long position, short play);
    long __cdecl dispatch(MidiTrack *track, short play, short notify);

    MidiMap *map;
    MIDIOUTCAPS caps;
    short data; /* the file's handle */
    short unknown82;
    unsigned long *file;
    unsigned long fileSize;
    unsigned char *keys; /* Key# */
    unsigned char *patches; /* Prg# */
    MidiHeader header; /* the whole file, prepared */
    MidiHeader sysex; /* for sending sysex */
    long timer;
    unsigned long startTime;
    unsigned short step; /* ticks to the timer's next call */
    short unknown11E;
    long interval; /* the timer's, in fixed-point ms */
    long tempoScale; /* fixed-point, 1 / rate */
    unsigned long tempo; /* microseconds per quarter note */
    long msPerTick; /* fixed-point */
    long ticksPerMs;
    unsigned short maxMs; /* the longest step, in ms */
    unsigned short maxTicks;
    unsigned long nextEvent; /* ticks to the next event */
    unsigned long ticks; /* the position */
    unsigned long time; /* ms */
    unsigned short fraction; /* of a ms */
    unsigned short finishedTracks;
    short setupDone;
    short unknown14A;
    unsigned long setupEnd; /* ticks at "setup end" */
    Deferred call; /* the timer's, to run midiStep */
    short looping; /* "loop end" wants a jump back */
    short lastLoop;
    unsigned short loopCount; /* left to play (0xffff: forever) */
    short unknown16A;
    unsigned long loopStart;
    unsigned long loopEnd;
    const char *findText; /* a cue point setText looks for */
    unsigned short findLength;
    short found;
    unsigned short format;
    unsigned short division;
    unsigned short trackCount;
    short unknown182;
    MidiTrack tracks[1];
};

struct MidiMapState
{
    short ready;
    short hardReset; /* [MidiMap] fEnableHardReset */
    MidiDevice *devices;
};

extern MidiMapState midiMapState; /* @data 0x4b9b28 */

short midiMapCacheDrumPatches(long map, unsigned short patch, WORD *keys, unsigned short flags);
short midiMapCachePatches(long map, unsigned short bank, WORD *patches, unsigned short flags);
void fillVelocities(MidiMap *map, long table);
short midiMapClose(long map);
short getChannelMap(unsigned short device, unsigned short *channels);
short getMutedChannels(unsigned short device, short *muted);
short getMidiDevCaps(unsigned short device, MIDIOUTCAPS *caps, unsigned short size);
short findMidiDevice(unsigned short id, MidiDevice **device);
short getDrumChannel(unsigned short device, unsigned short *channel);
short midiMapDevice(long map, unsigned short *device);
short midiMapTarget(long map, short *target);
short midiMapTable(long map, long *table);
short initMidiMap();
short setChannelMuted(unsigned short device, unsigned short channel, short muted);
short midiMapOpen(MidiMap **map, unsigned short device, long callback, long instance, long flags);
short midiMapPrepareHeader(long map, MidiHeader *header, unsigned short size);
void closeMidiMaps();
short setChannelMap(unsigned short device, unsigned short channel, unsigned short to);
short midiMapReset(long map);
short loadTargetDevice(MidiDevice *device, short target);
short setChannelMaps(unsigned short device, unsigned short *channels);
short setMutedChannels(unsigned short device, short *muted);
short setMidiMapTable(long map, long table);
short midiMapUnprepareHeader(long map, MidiHeader *header, unsigned short size);
MidiMap *midiMap(long map); /* 0x478d38: 0 if it isn't one */
void resetChannel(MidiMap *map, int channel); /* 0x478b09 */
short midiMapLongMsg(long map, MidiHeader *header, unsigned short size);
short mapShortMsg(MidiMap *map, unsigned long message);
short midiMapShortMsg(long map, unsigned long message);
unsigned long __cdecl readVarLen(unsigned char **p);
void midiStep(void *sound);
void midiTimer(long timer, long sound);
/* Fixed-point (16.16) arithmetic */
long fixedDiv(long a, long b); /* 0x46d754 */
long fixedMul(long a, long b); /* 0x46d7aa */
long makeFixed(short whole, unsigned short fraction); /* 0x48988a */
short fixedToInt(long value); /* 0x48989e */
unsigned short fixedFraction(long value); /* 0x4898ab */
short fixedRound(long value); /* 0x4898b6 */

/* Threads (the OS layer) */
long createThread(void (*proc)(long), long, long stackSize, short); /* 0x46e302 */
void deleteThread(long thread); /* 0x46e463 */
void resumeThread(long thread); /* 0x46e857 */
void yieldThread(long); /* 0x46eb9f */
void fn_46eadd(long thread, short state);
short fn_46e605(long thread);
void fn_46e410();
void fn_46e43a();

/* Rectangles (QuickDraw's) */
short emptyRect(ShortRect *rect);
void offsetRect(ShortRect *rect, short dx, short dy);
void insetRect(ShortRect *rect, short dx, short dy);
short sectRect(ShortRect *rect, ShortRect *with);
ShortRect *unionRect(ShortRect *into, ShortRect *add);
short ptInRect(ShortRect *rect, Point *point);
ShortRect *__cdecl setRect(ShortRect *rect, short left, short top, short right, short bottom);

/* Regions (errors in regionError) */
short newRgn();
void disposeRgn(short region);
short setEmptyRgn(short region);
unsigned short emptyRgn(short region);
short setRectRgn(short region, ShortRect *rect);
short copyRgn(short to, short from);
void compactRgn(short region);
void regionToHrgn(HRGN target, short region, short dx, short dy);
short sectRgnWithRect(short region, ShortRect *rect);
short sectRgnRects(short region, long count, ShortRect *rects);
void diffRgnRect(short region, ShortRect *rect);
void diffRgnRects(short region, long count, ShortRect *rects);
void unionRgnRect(short region, ShortRect *rect);
short unionRgnRects(short region, long count, ShortRect *rects);
short unionRgn(short to, short from);
short tidyRgn(Region *region);
Region *getRegion(short region);
void shrinkRgn(short handle, Region **region);
void removeRgnRect(Region *region, long index);
short regionError();
short insertRgnRect(short handle, Region **region, long index, ShortRect *rect);
short setRegionError(short error);

/* Settings files */
short iniError();
short openIni(fileSpec *path);
unsigned short lineStart(IniFile *ini, char *text);
unsigned short nextLine(IniFile *ini, char *text);
void closeIni(short handle);
unsigned short nextKey(IniFile *ini, char *text);
unsigned short nextSection(IniFile *ini, char *text);
short findKey(IniFile *ini, char *text, const char *section, const char *key);
short findSection(IniFile *ini, char *text, const char *section);
short initIni();
short iniBufferSize();
void closeAllIni();
unsigned short skipSpaces(IniFile *ini, char *text);
short isSpace(char c);
short setIniError(short error);
short getIniString(fileSpec *file, const char *section, const char *key, char *buffer,
                   unsigned short size);
short getIniBool(fileSpec *file, const char *section, const char *key, short *value);
short getIniLong(fileSpec *file, const char *section, const char *key, long *value);

/* Decompiled functions, by address */

unsigned short loadSoundByKey(short key, long type);
unsigned short fn_411382(short key, long type);
void unloadSound(short key, long type);
void fn_41158c(short key, long type);
short playSoundOn(short key, long type, short channel);
short fn_411bfe(short key, long type, short channel);
void unloadSounds();
Entry *fn_4115f5(short key, long tag);
void fn_411910(Entry *entry, short channel);
Entry *addSound(short key, long type);
void removeSound(Entry **entry);
void setSoundType(Entry **entry, short key, long type);
void reportSoundError(short id, long type, Entry *entry, const char *message);
short findChannel(short type);
short fn_4120c8(char value, long type);
Entry *findOrAddSound(short key, long type);
short isSoundPlaying(unsigned short id, long type);
short startSound(Entry *entry, short channel);
void stopSounds(unsigned short id, long type);
void fn_411e4c(unsigned short id, long type);
short fn_4120a2(unsigned short id, long type, short stop);
short waitForSound(unsigned short id, long type, short eventType, short discard);
short fn_412084(unsigned short id, long type, short eventType, short discard);
short waitForSoundValue(char value, long type, short eventType, short discard);
short fn_412159(char value, long type, short eventType, short discard);
short loadSound(Entry *entry);
Entry *getSound(short key, long type);
short prepareSound(Entry *entry, short channel);
void fn_4119f3(Entry *entry, short channel);
void fn_411d2c(long, SoundNotice *notice, long cookie);
void fn_412176(long type);
short fn_4121a5(short level);
void fn_4117a8(Entry *entry);
short fn_412b4d(void (*callback)(InputItem *item));
short fn_412844(InputItem *item);
short fn_412884();
void fn_412cc0();
void fn_412cd0();
void fn_412cdf();
void fn_412d37();
void fn_412d47();
void fn_412d57();
short fn_41336f();
short fn_4133a4();
short hitTestFocus(Point *where);
void fn_412b6b();
void fn_412b8f();
void fn_412cef();
void fn_412d13();
void fn_412bb3(short alternative);
void fn_412c3d();
short fn_4128c6(GroupList *list);
short fn_41295f(Group *group);
void fn_413a4e(InputState *state, short all);
void fn_413afd(InputState *state, short all);
void fn_413bad(Point *where);
short fn_41382a(Group *group, short start);
short fn_4138a2(Group *group, short start);
short fn_41391d(InputItem *item);
short fn_413693(GroupList *list, short first, short start);
short fn_413755(GroupList *list, short first, short start);
short fn_41348b(short list, short group, short start);
short fn_41357a(short list, short group, short start);
short focusItemAtPoint(Point *where);
short focusItemByKey(short key);
short focusItemAt(short x, short y);
short focusItem(InputItem *item);
void visitAllItems();
void numberAllItems();
void setGroupLists(GroupList *lists, short count, unsigned short flags);
void leaveEnteredItem();
void enterFocusedItem();
void moveMouseTo(short x, short y);
void moveMouseToFocus();
short moveFocus(short direction);
InputItem *hoverItemAtPoint(Point *where);
void highlightFocus();
void toggleFocusedItem();
void switchOffOthers();
short fn_412722(short on, short value);
void stepFocus(short direction);
short fn_412587(InputItem *item, unsigned short button);
short trackPress(unsigned short button);
InputItem *highlightItemAt(short x, short y);
short handleMouse(Point *where, unsigned short button);
InputItem *handleKey(unsigned short *key);
void pressFocusedItem();
void getItemPosition(InputItem *item, Cursor *where);
InputItem *itemAt(short x, short y);
void activateItemAt(short x, short y);
void fn_413bcf(void (*hook)(Point *where));
void freeText(void **block);
short queuedEvents();
void postEvent(Event *event);
short getEvent(Event *event);
short hasEvent(short type);
void removeEvents(short type);
void postKeyEvent(short key);
void postMouseEvent(Point *where, short button);
short isEventWaiting(short type, short discard);
void discardEvents(short type);
short handleNextEvent();
void getMousePosition(Point *where);
void waitForEvent(short type, short discard);
void __cdecl nextEventIndex(short *index);
void freeFade(Fade **fade);
/* anim */
void freeAnim(Anim **anim);
short stepAnim(Anim *anim);
short skipAnim(Anim *anim, short frames);
Anim *showAnim(Anim *anim);
void drawAnim(Anim *anim);
void drawSprite(Anim *anim, Sprite *sprite);
void markSprite(Anim *anim, short index, short which);
void resetSprite(Sprite *sprite);
void addRect(Anim *anim, ShortRect *rect, short which);
short playAnimation(AnimSpec *spec);
void setupAnim(AnimSpec *spec);
void loadCast(Anim *anim, short first, const char *name);
short playAnim(AnimSpec *spec);
void freeAnimSpec(AnimSpec *spec);
void restartAnim(Anim *anim, short run);
void runFrame(Anim *anim);
void loadScript(long *script, short id, const char *name);
void freeScript(long *script);
short fn_411212();
void setupAnimOffscreen(AnimSpec *spec);
void spritesBounds(Anim *anim, ShortRect *into);
short *fn_46cae6(long resource);
void fn_46c6db(long *info, short id, short *count, const char *name);
void fn_46c77c(long *resource);
void fn_46c148(long *resource, short first, short member, const char *name);
void fn_46c808(long *resource, short id, const char *name, short);
void fn_46c86c(long *resource);
void fn_46c88c(long *resource, short id, const char *name);
void fn_46c970(long *resource);
void fn_48b1e8(const Rect &rect); /* erases a rectangle */
/* buttons */
void drawButtonOn(InputItem *item);
void fn_4121df(InputItem *item);
void drawButtonInColor0(InputItem *item);
void drawButtonOff(InputItem *item);
void fn_41221b(InputItem *item);
void drawButtonInColor1(InputItem *item);
void fn_412244(InputItem *item);
void fn_412255(InputItem *item);
void drawButtonInColor4(InputItem *item);
void drawButtonPressed(InputItem *item, short mode);
void drawButton(InputItem *item, short on, short mode);
void drawButtonInColor(InputItem *item, short color);
void addButtonGroup(ResourceList **images, short id, short kind, Group *group, const char *name);
void freeButtonGroup(ResourceList **images);
void fn_412482(ResourceList **images);
void fn_46c011(ResourceList **list, short id, const char *what, short); /* loads a resource list */
void fn_46c2db(ResourceList **list);
void fadeTo(PALETTEENTRY *to);
void fadePalette(PALETTEENTRY *to, unsigned short first, unsigned short count, short duration,
                 short byTime, Fade **fade);
void startFade(Fade **fade, PALETTEENTRY *to, unsigned short first, unsigned short count,
               short duration, short byTime);
void runFade(Fade **fade);
short stepFade(Fade *fade);
long fn_46d754(long numerator, long denominator); /* 16.16 fixed-point division */
void fn_4153b0(Callback callback);
void setErrorReporter(void (*reporter)(const char *prefix, const char *format, va_list args));
void fn_4153ce(const char *message);
void fn_415514();
void fn_415604(Callback callback);
short fn_4157f3();
void fn_415811();
void fn_41581b(short flag);
unsigned short toLowerAscii(unsigned short c);
unsigned short toUpperAscii(unsigned short c);
void fn_415a11(Callback callback);
void fn_415a20(Callback callback);
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
void gameFrame();
void mainLoopEvents();
void checkDisplayMode(DisplayMode *mode);
void handleWaitingMessage();
void waitWhilePaused();
short pumpMessage(MSG *message, unsigned short first, unsigned short last, unsigned short flags);
void handleSystemKey(MSG *message);
void handleMessage(MSG *message);
short createMainWindow(short width, short height);
short addModifierKeys(short modifiers);
void getCursorPosition(Point *where);
void setCursorPosition(short x, short y);
short isButtonStillDown(unsigned short button);
short allocateBlock(void **block, unsigned long size);
void getClockTime(char *hour, char *minute, char *second);
void enterProgramDirectory();
void restoreDirectory();
void brightenPalette(PALETTEENTRY *entries, short first, short count);
short isInputWaiting(short which);
void logMessage(long message, long wParam, long lParam, short after, long result);
void dumpMessages();
short noteOutOfMemory(unsigned long size, short error);
int isMousePresent();
void freeAndClear(void **block);
char *intToDecimal(int value, char *buffer);
char *unsignedToDecimal(unsigned long value, char *buffer);
void fn_455e26(long);
void fn_455e2d(long);
short fn_455e85(Point *where, short button);
void fn_456a2f(Callback callback);
short fn_4568d8();
short mainLoopUpdate();
void fn_456a3e(long first, long second);
void fn_456a55(void (*callback)(short active));
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
void __cdecl clearLock(DeferLock *lock); /* 0x46d7f8 */
long __cdecl enterLock(DeferLock *lock); /* 0x46d827 */
void __cdecl leaveLock(DeferLock *lock); /* 0x46d838 */
void __cdecl initLock(DeferLock *lock, short listed); /* 0x46d8af */
void __cdecl removeLock(DeferLock *lock); /* 0x46d8e8 */
void __cdecl deferCall(DeferLock *lock, Deferred *call); /* 0x46d91c */
short fn_46d9c8();
void *localAlloc(unsigned long size); /* 0x46d95c */
void localFree(void *block); /* 0x46d998 */
short fn_46e00b(unsigned long thread); /* whether a thread belongs to the game */
void fn_46da35(short value);
int isAlignedPointer(void *pointer);
long atomicDecrement(long *value);
void *atomicExchange(void **target, void *value);
long atomicIncrement(long *value);
short debugBreak(short value);
void fn_46dc45();
HINSTANCE engineInstanceHandle();
unsigned long appThreadId();
HWND appWindowHandle();
unsigned long currentTimeMs();
short osLockMemory(void *address, unsigned long size);
short osUnlockMemory(void *address, unsigned long size);
short isAppActive();
short fn_46dff7();
ActivateHook setActivateHook(ActivateHook hook);
HINSTANCE fn_46e0ec(long);
short setOsError(short error);
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
