/*
 * Declarations shared by the decompiled modules: the game's globals, the types
 * worked out so far, and every decompiled function, so any module can use them.
 * Names follow CLAUDE.md: by address until their purpose is known. Types are
 * inferred from how the code uses them and grow as more of it is decompiled.
 */

#ifndef ZOOMBINIS_H
#define ZOOMBINIS_H

#include <windows.h>
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
typedef void (*Callback)();

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
    long port; /* drawn into */
    long screen; /* shown on */
    long background; /* a saved copy of what's under it, or 0 */
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
    long port; /* holding the saved pixels */
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
    short unknown4;
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
    __cdecl fileSpec(); /* 0x4850ec */
    __cdecl fileSpec(const char *path);
    __cdecl fileSpec(const fileSpec &directory, const char *name); /* 0x48518c */
    __cdecl ~fileSpec(); /* 0x48533e */
    fileSpec &__cdecl operator=(const fileSpec &from); /* 0x4853a2 */
    short __cdecl compare(const fileSpec &with) const; /* 0x4853f3: 0 if the same, else 0x2844 or an error */

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
extern long *screenPortRef; /* @data 0x4a0070: animations show on *screenPortRef */
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
extern Counted *g_4a8dcc;
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
extern long screenPort; /* @data 0x4aa7a4: the window's port */
extern ShortRect gameRect; /* @data 0x4aa7a8: the game's area */
extern ShortRect screenRect; /* @data 0x4aa7b0 */
extern ShortRect g_4aa7b8;
extern long workPort; /* @data 0x4aa7c8: where the game draws, off screen */
extern short g_4aa7cc;
extern short g_4aa7ce;
extern DisplayMode displayMode; /* @data 0x4aa7d0 */
extern DisplayMode g_4aa7dc;
extern PALETTEENTRY g_4aa7e8[256];
extern PALETTEENTRY g_4aabe8[256];
extern long palette; /* @data 0x4aafe8 */
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
extern long fonts[3]; /* @data 0x4b28c8 */
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
extern short g_4b9cf4;
extern short g_4b9cf6;
extern short g_4b9cf8;
extern long g_4b9cfc;
extern short g_4b7cf8;
extern long g_4b9d00;
extern long g_4b9d04;
extern long g_4b9d08;
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
unsigned long fn_492fbc(); /* the engine's clock, in ms */
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
void fn_46cb10(long *font, const char *name, long size, long);
/* Initialises the Mohawk OS layer, with a work buffer. */
short fn_46ddaf(HINSTANCE instance, void *buffer, long size);
/* QuickTime (see quicktime.py) */
long __cdecl QTInitialize(long *version);
long qtim_0b();
long __cdecl cmgr_0b(long, HWND window, UINT message, WPARAM wParam, LPARAM lParam);

/* Engine functions whose calling conventions aren't known yet: these
   declarations produce the calls the game makes. */

void fn_476622(long handle);
void fn_4771a4(long handle);
short fn_476bb4(); /* the last sound error */
void fn_476f50(long handle);
short fn_476e72(long handle, long); /* prepares a sound */
short fn_476ff6(long handle, long position); /* seeks a sound */
char *fn_46cafb(long resource); /* a resource's data */
long fn_477794(short resource);
long fn_477848(long resource, long);
void fn_41585f();
/* Starts a sound; its owner hears about it through `notify`. Non-zero on failure. */
short fn_47712a(long handle, void (*notify)(long, SoundNotice *, long cookie), long cookie);
void fn_46c602(long *);
/* Finds resource `id` of type `type`; 0 if there's none. */
long fn_46c402(long type, short id, short);
void fn_46c5b7(long *resource); /* releases a resource */
unsigned long fn_48ff28(long resource); /* a resource's size */
short fn_490140(long resource);
/* Joins two strings into a new block at *joined. */
void joinText(char **joined, const char *first, const char *second);
void reportJoinedError(char *message);
void fn_4771e4(long handle);
short fn_480b80(InputItem *item, Point *where); /* the default hit test */
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
void fn_48da48(short);
short fn_48d22c(int);
void fn_48daa8();
void fn_48c538();
void fn_4887f4();
void fn_48b1b4();
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
void createPort(long *port, ShortRect *bounds, short keep, const char *name);
void destroyPort(long *port, short release);
void fn_414a2e(long port, ShortRect *bounds);
void saveRect(MapSave **save, ShortRect *rect, short locked, const char *name);
void restoreRect(MapSave **save, short free);
void freeSave(MapSave **save);
void clipRect(short *region, ShortRect *rect, short keep);
void fn_414c25(short *region, short free);
void getClipRegion(short *region, short create);
void createRegion(short *region);
void freeRegion(short *region);
void copyBits(long to, long from, ShortRect *rect);
void showRect(ShortRect *rect);
void lockPort(long port);
void lockSave(MapSave *save);
void unlockSave(MapSave *save);
void alignRect(ShortRect *rect, short x, short y, short how);
void fn_414e7d();
void redrawRect(ShortRect *rect);
void fn_414f01(InputItem *item);
void fn_414f17(InputItem *item);
void fn_456a64();
/* Mohawk engine */
short fn_48ba5a(DisplayMode *mode, short); /* non-zero on failure */
void fn_48d798(short);
long fn_488d08(short count, PALETTEENTRY *entries); /* creates a palette */
void fn_48906c(long palette);
short fn_48c300();
void fn_48c314();
void fn_48adf0(unsigned short *image, short x, short y, short mode); /* draws an image */
long fn_48b4a8(); /* the current palette */
void fn_48d5ec(long palette, short first, short count, PALETTEENTRY *entries);
long fn_488ba8(short width, short height, short depth, long); /* creates a port */
void fn_48db08(long port);
void fn_4890f8(long port);
void fn_48d9c8(short left, short top);
void fn_488828(const Rect &rect);
void fn_48d1e0(short region);
void fn_48b2ac(short region);
void fn_488a88(long to, long from, const Rect &fromRect, const Rect &toRect, short mode);
short fn_48c750(long port); /* locks a port; non-zero on failure */
void fn_48c5fc(const Rect &rect);
long fn_488f34(const Rect &bounds, HWND window, long); /* creates a window port */
void fn_48d574(long);
void fn_48d194(const Rect &rect);
void fn_48c9ac(const Rect &rect, Color color, short); /* fills a rectangle */
Color fn_48b4d8(); /* the current colour */
Color fn_48d884(Color color); /* sets the colour, returning the old one */
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
short fn_4764bc(short open); /* opens (1) or closes the sound driver; non-zero on failure */
void runClock(short running);
void fn_48b2d8(DisplayMode *mode);
void fn_48d480(DisplayMode *mode);
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

void fn_48f660(long handle, long, long);

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
void closeFile(long file, short);
short fileMissing(fileSpec &file);
unsigned long handleSize(short handle);
void fn_48f464(short handle, short);

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
    char unknown0[0xc];
    basePort *ports; /* +0xc: the ports using it */
    short unknown10;
    short unknown12;
    short unknown14;
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
    virtual void v6();
    virtual void v7();
    virtual void prepare(); /* 8: before GDI calls on dc */
    virtual void v9();
    virtual void v10();
    virtual void v11();
    virtual void v12();
    virtual void v13();
    virtual short setColor(short which, Color color); /* 14 */
    virtual void v15();
    virtual void v16();
    virtual void v17();
    virtual void v18();
    virtual void v19();
    virtual void v20();
    virtual unsigned short nearestIndex(RGBColor color); /* 21 */
    virtual void v22();
    virtual RGBColor paletteColor(unsigned short index); /* 23 */
    virtual void v24();
    virtual short lock(); /* 25 */
    virtual short fillRect(short, Color color, const Rect *rect); /* 26 */
    virtual void v27();
    virtual void v28();
    virtual void v29();
    virtual void v30();
    virtual void release(); /* 31: before deleting */
    virtual void v32();
    virtual void v33();
    virtual void unlock(); /* 34 */
    virtual void v35();
    virtual void v36();

    long unknown4; /* 'Port' */
    char unknown8[0xc];
    long kind; /* +0x14: 5 a window */
    char unknown18[0x24];
    Palette *palette; /* +0x3c */
    char unknown40[0x18];
    Color foreColor; /* +0x58 */
    char unknown5c[2];
    short mode; /* +0x5e */
    short clip; /* +0x60: a region */
    short clipChanged; /* +0x62 */
    short locks; /* +0x64 */
    char unknown66[6];
    HDC dc; /* +0x6c */
    char unknown70[0x50];
};

/* A window's port: two more slots. */
class windowPort : public basePort
{
public:
    virtual void v37();
    virtual short v38();
};

extern short portError; /* @data 0x4b9ba0 */
extern unsigned short paletteReserved; /* @data 0x4b9c0c: system colours kept (half at each end) */
extern basePort *currentPort; /* @data 0x4b9c70 */
basePort *checkPort(basePort *port, short kind);
basePort *portObject(short kind);
short setPortError(short error);
short getPortError();
long portHandle(basePort *port);

/* Memory: relocatable blocks by handle (a short), as on the Mac. */
short newHandle(long size);
void *handleData(short handle);
void *lockHandle(short handle);
void unlockHandle(short handle);
short setHandleSize(short handle, long size); /* an error code */
short disposeHandle(short handle);
short memError();

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
void setEmptyRgn(short region);
unsigned short emptyRgn(short region);
void setRectRgn(short region, ShortRect *rect);
void copyRgn(short to, short from);
void compactRgn(short region);
void regionToHrgn(HRGN target, short region, short dx, short dy);
void sectRgnWithRect(short region, ShortRect *rect);
short sectRgnRects(short region, long count, ShortRect *rects);
short fn_48f4bc(short to, short from);
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
long fn_455013(long, long);
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
