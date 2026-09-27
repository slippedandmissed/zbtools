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

/* A list of shapes (images), loaded together (e2memory). */
struct ResourceList
{
    short id; /* of the 'tCNT'/'SHPL' resource listing them */
    long list; /* that resource */
    long palette; /* its 'tPAL' resource, if any */
    unsigned short count;
    long resources[1]; /* the shapes' resources */
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

/* A polygon, in a handle (as a port fills them). */
struct PolygonData
{
    long tag;
    ShortRect bounds;
    unsigned short count; /* +0xc */
    Point points[1]; /* +0xe */
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
class Rect : public ShortRect
{
public:
#ifdef RECT_OUT_OF_LINE
    /* The engine's port modules call this out of line; the game expands it. */
    __cdecl Rect(const ShortRect &rect); /* 0x48a667 */
#else
    Rect(const ShortRect &rect) { memcpy(this, &rect, sizeof(Rect)); }
#endif
    __cdecl Rect(short left, short top, short right, short bottom); /* 0x48c8d4 */
    /* The ports' modules call these (out of line). */
    __cdecl Rect(); /* 0x48ab89 */
    Rect &__cdecl operator=(const ShortRect &rect); /* 0x48ab91 */
    void __cdecl operator=(const tagRECT &rect); /* 0x48ac29 */
};


/* A slot in the camp (basecamp), 22 bytes. */
struct CampSlot
{
    long zoombini; /* 0: empty */
    ShortRect rect; /* where it stands */
    char unknownC[10];
};

/* The camp's slots, in a block. */
struct Camp
{
    short row; /* the first row shown */
    short count;
    CampSlot slots[625];
};

/* A Zoombini on the journey (19 bytes, from 0xa934 in the game's state). */
struct Traveller
{
    long zoombini;
    char unknown4[5];
    char unknown9[10];
};

/* A view (the view module): part of the screen, with callbacks to update
   and draw it. Partly known. */
struct View
{
    char unknown0[0x24];
    unsigned long nextUpdate; /* +0x24 */
    unsigned long interval; /* +0x28 */
    char changed; /* +0x2c */
    char reset; /* +0x2d */
    char unknown2e[0xa0];
    ShortRect unknownCe; /* +0xce */
};

/* A button in the camp (0x24 bytes). */
struct CampButton
{
    ShortRect rect;
    char unknown8[0x1c];
};

/* Images in one block: each one's offset from the start (from index 1). */
struct ImageBank
{
    long unknown0;
    long offsets[1];
};

/* A Zoombini on screen (the snoids module), 0x104 bytes (by the frame drawCamp gives it). Partly known. */
struct Snoid
{
    char unknown0[0x98];
    short unknown98;
    long unknown9a;
    ShortRect bounds; /* +0x9e: where it was drawn */
    short x; /* +0xa6 */
    short y;
    char unknownAa[8];
    short unknownB2;
    char unknownB4[8];
    long zoombini; /* +0xbc */
    short unknownC0;
    char unknownC2[0x42];
};

/* The Zoombinis setting out (0x266 bytes, at 0xa92e in the game's state;
   saved at 0xa462). */
struct Party
{
    short unknown0;
    short unknown2;
    short unknown4;
    Traveller travellers[32];
};

/* A wipe in progress (basecamp's runWipe). */
struct Wipe
{
    short steps; /* lines to copy */
    short done; /* lines copied */
    unsigned long time; /* of the last step */
    long position; /* lines due (16.16) */
    long step; /* lines per tick (16.16) */
    basePort *from;
    basePort *to;
    ShortRect fromRect;
    ShortRect toRect;
    unsigned short direction; /* 0 from the top, 1 bottom, 2 left, 3 right */
};

/* Venetian blinds in progress (basecamp's runBlinds). */
struct Blinds
{
    short stripe; /* each stripe's height: the lines to copy in each */
    short done;
    unsigned long time;
    long position;
    long step;
    basePort *from; /* none: erase */
    basePort *to;
    ShortRect fromRect;
    ShortRect toRect;
    short stripes;
};

class WinPoint;

/* A point as the engine passes it by value. */
class Pt : public Point
{
public:
    __cdecl Pt(short x, short y); /* 0x48da17 */
    __cdecl Pt(const Point &point); /* 0x48c6f3 */
    __cdecl Pt(const WinPoint &point); /* 0x48c736 */
    __cdecl Pt(); /* 0x488786 */
    void __cdecl operator=(const Pt &point); /* 0x4887ab */
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
    __cdecl Color(const RGBColor &color); /* 0x4888d9 */
    unsigned short __cdecl paletteIndex() const; /* 0x4888f2 */
    RGBColor __cdecl rgb() const; /* 0x4889a4 */
    long __cdecl kind() const; /* 0x488a39 */
};

/* A Color's bytes as a value of their own. */
class RGBColor
{
public:
    union {
        long value;
        ColorBytes bytes;
    };
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


/* A timer the OS layer runs from its window's messages (0x1c bytes), tagged
   'ksTI' (bytes in memory order). Handed around as a handle (its address). */
struct OsTimer
{
    long tag;
    OsTimer *prev;
    OsTimer *next;
    void (*proc)(long timer, long data);
    long data;
    long interval; /* ms; 0 stopped, -1 every time */
    unsigned long due;
};

/* The OS layer's state (0x2c bytes), cleared by osStartup. */
struct OsState
{
    short error; /* of the last call */
    short running; /* started */
    short active; /* the application is active */
    short unknown6;
    ActivateHook activateHook;
    HINSTANCE instance;
    unsigned long thread; /* the application's */
    HWND window; /* 'MOHAWK OS Manager' */
    HHOOK hook; /* on the thread's messages, to run timers */
    short runningTimers;
    short unknown1E;
    unsigned long nextTimer; /* when the next timer is due (0: none) */
    OsTimer *timers;
    UINT timerId; /* WM_TIMER's, while there are timers */
};

/* The system, as osStartup found it (0x2c bytes). */
struct SystemState
{
    short processor; /* 3 a 386 ... 7 */
    unsigned short windowsVersion; /* BCD: 0x395 for 3.95 (Windows 95) */
    SYSTEM_INFO info;
    short windowsNT;
    short unknown2A;
};

/*
 * The OS layer's threads (os_threads) are its own, cooperative ones, as on
 * the Mac: each has a stack of its own (carved from a buffer osStartup is
 * given) and a saved register context, and they take turns on the
 * application thread, switched by the scheduler at calls into the layer and
 * every time slice (a timer). Threads, mutexes and events are sync objects
 * (RTTI classes thread, mutex, event and their base sync): handed around as
 * handles (their addresses), tagged 'sync', and waited for the same way.
 */
class thread;

/* A thread's saved registers (0x2c bytes). */
struct Context
{
    long *stack; /* its stack's block (after the block's size) */
    unsigned long flags;
    unsigned long eax;
    unsigned long ebx;
    unsigned long ecx;
    unsigned long edx;
    unsigned long edi;
    unsigned long esi;
    unsigned long ebp;
    unsigned long esp;
    unsigned long eip; /* where it resumes */
};

class sync
{
public:
    __cdecl sync(); /* 0x46efa4 */
    virtual __cdecl ~sync(); /* 0 */
    virtual thread *__cdecl owner(); /* 1: a mutex's; 0 */
    virtual void __cdecl setSignaled(unsigned short signaled); /* 2: an event set, a thread ended */
    /* 3: waits (-1: for ever); 0 when it's signaled (or acquired), else
       an error (0x12e timed out). */
    virtual short __cdecl wait(thread *waiter, unsigned long timeout);
    virtual void __cdecl acquire(thread *waiter); /* 4: a waiter's wait is over */
    virtual void __cdecl enqueue(thread *waiter); /* 5 */
    virtual short __cdecl isBlocked(thread *waiter); /* 6: whether it must wait */
    virtual void __cdecl dequeue(thread *waiter, short result); /* 7: ends its wait with `result` */

    static void *__cdecl operator new(size_t size); /* 0x46f043: zeroed */
    static void __cdecl operator delete(void *block); /* 0x46f07f */

    long tag; /* +4: 'sync' */
    long kind; /* +8: 'thrd', 'mutx' or 'evnt' */
    unsigned short signaled; /* +0xc */
    short unknownE;
    sync *prev; /* +0x10: in threads.objects */
    sync *next; /* +0x14 */
    thread *waiters; /* +0x18: a ring */
};

class mutex : public sync
{
public:
    __cdecl mutex(); /* 0x46ed4a */
    virtual __cdecl ~mutex(); /* 0x46ed6a */
    virtual thread *__cdecl owner(); /* 1 */
    virtual void __cdecl setSignaled(unsigned short signaled); /* 2: released */
    virtual short __cdecl wait(thread *waiter, unsigned long timeout); /* 3 */
    virtual void __cdecl acquire(thread *waiter); /* 4 */
    virtual short __cdecl isBlocked(thread *waiter); /* 6 */
    virtual void __cdecl attach(thread *owner); /* 8 */
    virtual void __cdecl detach(thread *owner); /* 9 */

    unsigned short count; /* +0x1c: acquisitions by its owner */
    short unknown1E;
    thread *holder; /* +0x20 */
    mutex *next; /* +0x24: in holder's ring of mutexes */
    mutex *prev; /* +0x28 */
};

class event : public sync
{
public:
    __cdecl event(); /* 0x46ece8 */
    virtual void __cdecl setSignaled(unsigned short signaled); /* 2 */

    Deferred resetCall; /* +0x1c: posted by resetEvent */
    Deferred setCall; /* +0x30: posted by setEvent */
};

class thread : public sync
{
public:
    __cdecl thread(); /* 0x46f1f5 */
    virtual __cdecl ~thread(); /* 0x46f21b */
    virtual void __cdecl setSignaled(unsigned short ended); /* 2: 0 to start it, 1 to end it */
    virtual short __cdecl wait(thread *waiter, unsigned long timeout); /* 3: until it ends */

    unsigned long sliceStart; /* +0x1c */
    unsigned long sliceEnd; /* +0x20: 0 when it's to give way */
    unsigned long wakeTime; /* +0x24: sleeping until (yieldThread) */
    unsigned short suspendCount; /* +0x28 */
    unsigned short priority; /* +0x2a: above 1 is urgent */
    Context context; /* +0x2c */
    mutex *mutexes; /* +0x58: held, a ring */
    thread *prev; /* +0x5c: in the ring of threads started */
    thread *next; /* +0x60 */
    sync *waitingOn; /* +0x64 */
    short waitResult; /* +0x68 */
    short unknown6A;
    thread *waitPrev; /* +0x6c: in waitingOn's ring of waiters */
    thread *waitNext; /* +0x70 */
    unsigned long waitUntil; /* +0x74: 0 for ever */
};

/* The threads' state (0x3c bytes). */
struct ThreadState
{
    short error; /* of the last call */
    short initialized;
    DeferLock lock; /* +4: taken while scheduling state changes */
    long timeSlice; /* +0x14: ms (20) */
    unsigned short runnable; /* +0x18: threads started and not suspended */
    unsigned short urgent; /* +0x1a: of those, with priorities above 1 */
    unsigned short schedulingOff; /* +0x1c: disableScheduling's count */
    short scheduling; /* +0x1e: in schedule */
    thread *ring; /* +0x20: the threads started */
    thread *main; /* +0x24: the application thread's own */
    thread *current; /* +0x28 */
    sync *objects; /* +0x2c: every sync object */
    long timer; /* +0x30: the time-slice timer, while two threads run */
    long *stacks; /* +0x34: blocks: a size (bit 0: used), then the stack */
    long *stacksEnd; /* +0x38 */
};

/* The engine's file name class (4 bytes, no virtual functions). The engine
   wasn't compiled with -p, so its methods use the C convention. */
class fileSpec
{
public:
    __cdecl fileSpec(); /* 0x4850ec */
    __cdecl fileSpec(const char *path); /* 0x4850f8 */
    __cdecl fileSpec(const fileSpec &from); /* 0x485373 */
    __cdecl fileSpec(long volume, const char *path); /* 0x485291 */
    __cdecl fileSpec(const fileSpec &directory, const char *name); /* 0x48518c */
    __cdecl ~fileSpec(); /* 0x48533e */
    fileSpec &__cdecl operator=(const fileSpec &from); /* 0x4853a2 */
    short __cdecl compare(const fileSpec &with) const; /* 0x4853f3: 0 if the same, else 0x2844 or an error */
    short __cdecl getPath(char *path) const; /* 0x4854c5 */
    /* The volume and full path, checking the volume is there. */
    short __cdecl locate(long *volume, char *path) const; /* 0x4855c0 */
    short __cdecl volume(long *volume) const; /* 0x485596 */
    short __cdecl fileName(char *name) const; /* 0x485485: the last part */
    /* The directory part, with forward slashes. */
    short __cdecl directory(char *path) const; /* 0x48550c */
    /* Checks the name and makes it a full path on its volume. */
    short __cdecl canonicalize(); /* 0x485604 */

    struct FileName *name; /* shared, counted */
};

/* A fileSpec's name. */
struct FileName
{
    short references;
    short unknown2;
    long volume;
    char path[0x100]; /* from the root (or share), shortened to fit */
};

/* Whether a name fits the volume's file system: long names on NTFS and
   HPFS; else 8.3, with more characters allowed on long-name FAT. -1 if
   the volume isn't known. */
unsigned short __cdecl validName(const char *name, unsigned short length, long volume); /* 0x485a63 */

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
inline Party *party()
{
    return (Party *)(g_4a4ba0 + 0xa92e);
}
inline Party *savedParty()
{
    return (Party *)(g_4a4ba0 + 0xa462);
}
/* The Zoombinis on the journey. */
inline Traveller *travellers()
{
    return (Traveller *)(g_4a4ba0 + 0xa934);
}
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
extern short preloaded; /* @data 0x4ab4a8: a handle of the resources preloaded */
extern short preloadedCount; /* @data 0x4ab4aa */
extern Wipe wipe; /* @data 0x4ab4ac */
extern long cheatCode; /* @data 0x4ab4d8: the last keys typed, 7 bits each */
extern Blinds blinds; /* @data 0x4ab4dc */
extern long shapeListKind; /* @data 0x4a07f0: 'SHPL' */
extern long soundListKind; /* @data 0x4a07f4: 'SNDL' */
extern long noPreloadKind; /* @data 0x4a07f8 */
extern long cheatHash; /* @data 0x4a07fc */
extern short campRow; /* @data 0x4ab508: the first row of the camp shown */
extern short campRows; /* @data 0x4ab50a */
extern short campShown; /* @data 0x4ab50c: slots shown (campRows * 5) */
extern short campCount; /* @data 0x4ab50e: Zoombinis in the camp */
extern short campLast; /* @data 0x4ab510: the last slot used */
extern short g_4ab512;
extern Camp *camp; /* @data 0x4ab514 */
extern short g_4ab518;
extern short g_4ab51a;
extern short g_4ab524;
extern short g_4ab526;
extern short g_4ab52a;
extern short g_4ab52c;
extern short g_4ab52e;
extern CampButton campButtons[7]; /* @data 0x4a0824 */
extern ImageBank *campButtonImages; /* @data 0x4a0970 */
extern long campButtonsResource; /* @data 0x4a0968: holding campButtonImages */
extern long campFrameResource; /* @data 0x4a096c: holding g_4a0974 */
extern long campMap; /* @data 0x4ab520: BaseCamp.MHK */
extern short campActive; /* @data 0x4ab528 */
extern short campBusy; /* @data 0x4a0a9c: in campIdle */
extern short g_4b80ee;
extern short g_4a48e6;
extern short g_4b755a;
extern short g_4b755c;
extern short g_4b9684;
extern ImageBank *g_4a0974; /* the camp's frame */
extern ShortRect campButtonsBounds; /* @data 0x4a0a9e */
extern ShortRect campArrival; /* @data 0x4a0aa6: where Zoombinis back from the journey stand */
extern short g_4a080c;
extern short g_4a080e; /* the camp is scrolled half a row */
extern ShortRect g_4a0920; /* the way the camp is asked to scroll (1-4) */
extern short campX[10]; /* @data 0x4a09b0: each row's x, in two layouts (4a080e) */
extern short campY[10][5]; /* @data 0x4a09c6: each slot's y, in two layouts */
extern short primes[5]; /* @data 0x4a0800 */
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
extern short g_4b99d4; /* 1: e2memory's frees free at once, else mark purgeable */
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
extern short regionErrorCode; /* @data 0x4b9b64 */
extern short localMemErrorCode; /* @data 0x4b9cf0 */
extern OsState os; /* @data 0x4b9cf4 */
extern short g_4b7cf8;
extern SystemState systemState; /* @data 0x4b9d20 */
extern ThreadState threads; /* @data 0x4b9d4c */
extern thread *dyingThread; /* @data 0x4b9d88 */

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
void __cdecl debugPrintf(const char *format, ...); /* 0x46db93: to the debugger */
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
void fn_46cb10(Font **font, const char *name, unsigned short size, unsigned short style);
/* Initialises the Mohawk OS layer, with a work buffer. */
short osStartup(HINSTANCE instance, void *stacks, long size); /* 0x46ddaf */
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
long fn_46c402(long type, short id, short note);
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
short initFiles(long); /* 0x483732: initialises the file layer */
void __cdecl initDisplayMode(DisplayMode *mode, unsigned short width, unsigned short height, unsigned long colors,
                             short palettized);
unsigned short realizePalette(Palette *palette, short foreground);
short canUseDisplayMode(DisplayMode *mode, short change); /* whether a display mode is available (filling in the one it would use) */
Font *setFont(Font *font); /* the previous one */
short setTakeStatic(short take);
short setCursorLevel(short level);
short showCursor();
short hideCursor();
short beginPortUpdate();
short endPortUpdate();
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
short copyPortBits(basePort *to, basePort *from, const Rect &toRect, const Rect &fromRect, short mode);
short lockPort(basePort *port); /* locks a port; non-zero on failure */
short invertRect(const Rect &rect);
short lineTo(short x, short y);
short moveTo(short x, short y); /* 0x48c974 */
short drawText(const Rect &rect, unsigned short flags, const char *text,
               unsigned short length); /* 0x48aed8 */
basePort *newWindowPort(const Rect &bounds, HWND window, Palette *palette);
Palette *setPortPalette(Palette *palette); /* the previous one */
short setClipRect(const Rect &rect);
short fillPortRect(const Rect &rect, Color color, short);
Color getForeColor();
Color setForeColor(Color color); /* the previous one */
unsigned short setPenWidth(short width); /* the previous one */
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

void runClock(short running);
void fn_48b2d8(DisplayMode *mode);
void fn_48d480(DisplayMode *mode);
short initResources();
short fn_493096(); /* initialises the timer */

/* Reads `key` from `section` of the INI file `file` into buffer; non-zero if
   it couldn't. */
short fn_480790(const fileSpec &file, const char *section, const char *key, char *buffer,
                long size);
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
long fileSize(const fileSpec &file); /* 0x484690: a directory's is its files' total; -1 on error */
short fileError(); /* 0x484670 */
void programDirectory(fileSpec *directory); /* 0x484678 */
long openFile(fileSpec *file, short mode); /* 0x484b50: a handle, 0 on error (modes: FileRecord::open) */
short readFile(long file, void *buffer, long *size); /* 0x484c08 */
short closeFile(long file, short force); /* 0x48266c: even if closing fails, with force */
short fileMissing(const fileSpec &file); /* 0x483420: 0 if it exists, else an error (0x2845 not found) */
void currentDirectory(fileSpec *directory); /* 0x4845e4 */
short setCurrentDirectory(const fileSpec *directory); /* 0x484e9c */
void tempDirectory(fileSpec *directory); /* 0x484994: Windows' temporary directory (where .FOT files go) */
short deleteFile(const fileSpec &file);
/* Calls `callback` for each file (or subdirectory) in the current
   directory, until it returns nonzero. */
short forEachDirectory(FileCallback callback, void *data); /* 0x4831bc */
short forEachFile(FileCallback callback, void *data); /* 0x483310 */

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

/* A port's pen (+0x54). */
class Pen
{
public:
    Pt position;
    Color color;
    unsigned short mode; /* setMode's */
    short width;
    __cdecl Pen(); /* 0x48878e */
    __cdecl Pen(const Pt &position, const Color &color, unsigned short mode, short width); /* 0x4887c4 */
};

class PixMap;

/* A device-independent bitmap (a DIB section) that ports draw through
   (module_489148). */
class DIB
{
public:
    /* Draws `from` (of the bitmap) into `to` of a port, flipped as asked. */
    virtual short draw(basePort *port, Rect to, Rect from, long usage, DWORD rop,
                       unsigned short flipX, unsigned short flipY); /* 0 */
    virtual HDC getDC(); /* 1 */
    virtual void releaseDC(HDC dc); /* 2 */
    virtual void getColors(unsigned short first, unsigned short count, RGBQUAD *colors); /* 3 */
    virtual short create(short clear); /* 4: non-zero on failure */
    virtual void destroy(); /* 5 */
    /* The usage to draw with (DIB_RGB_COLORS unless the port takes indices). */
    virtual long usage(long paletteKind, DWORD rop, short flipX, short flipY); /* 6 */
    virtual void setColors(unsigned short first, unsigned short count,
                           const RGBQUAD *colors); /* 7 */
    virtual void setEntries(unsigned short first, short count,
                            const PALETTEENTRY *entries); /* 8 */

    __cdecl DIB(short width, short height, unsigned short depth); /* 0x489148 */
    static void *__cdecl operator new(size_t size); /* 0x48ac52 */
    static void __cdecl operator delete(void *block); /* 0x48aba7 */

    ShortRect bounds; /* +4: (0, 0, width, height) */
    unsigned short depth; /* +0xc */
    short unknownE;
    long portUsage; /* +0x10: DIB_PAL_COLORS */
    BITMAPINFO *indexInfo; /* +0x14: with palette indices for colours */
    BITMAPINFO *info; /* +0x18: with RGB colours */
    void *bits; /* +0x1c */
    long rowBytes; /* +0x20 */
    long lastRow; /* +0x24: the offset of the top row (DIBs are bottom-up) */
    HBITMAP bitmap; /* +0x28 */
    HDC dc; /* +0x2c */
};

extern RGBQUAD monoColors[2]; /* @data 0x4a8ab0 */
extern RGBQUAD vgaColors[16]; /* @data 0x4a8ab8 */
extern DWORD patternRops[8]; /* @data 0x4a8af8: PatBlt's for the drawing modes */
extern DWORD copyRops[8]; /* @data 0x4a8b18: BitBlt's for the transfer modes */

class basePort
{
public:
    virtual __cdecl ~basePort(); /* 0 */
    /* Copies `from` of this port to `to` of another, in a transfer mode;
       flags: 1 and 2 pick the stretch mode, 0x10 and 0x20 flip. */
    virtual short copyBits(basePort *port, const Rect *to, const Rect *from, unsigned short mode,
                           unsigned short flags); /* 1 */
    virtual void applyMapping(); /* 2: bounds to frame, as viewport and window */
    virtual short clipTo(HRGN rgn); /* 3: clips to the clip region and rgn */
    virtual short setupDC(); /* 4: once dc is made */
    virtual short toHrgn(HRGN target, short region); /* 5: in device coordinates */
    virtual void depthChanged(); /* 6: the display's depth changed (while locked) */
    virtual void cleanupDC(); /* 7: before dc goes */
    virtual void prepare(); /* 8: before GDI calls on dc */
    virtual void realizePalette(); /* 9: while locked */
    virtual short setBackColor(Color color); /* 10 */
    virtual short setClip(short region); /* 11 */
    virtual short useFont(Font *font); /* 12 */
    virtual void setUnknown66(short value); /* 13 */
    virtual short setColor(Color color, unsigned short width); /* 14: the pen */
    virtual void setMode(short mode); /* 15: the pen's */
    virtual short drawPixels(const Rect &bounds, unsigned short width, unsigned short height,
                             short rowBytes, unsigned short format, void *pixels,
                             unsigned short mode, unsigned short flags); /* 16 */
    virtual void getBits(PixMap *map); /* 17: unsupported but by DIB ports */
    virtual HBRUSH brush(Color color); /* 18: a new brush */
    virtual HBRUSH patternBrush(const unsigned short *pattern); /* 19: a new brush */
    virtual int stretchDIBits(int toX, int toY, int toWidth, int toHeight, int fromX, int fromY,
                              int fromWidth, int fromHeight, const void *bits,
                              BITMAPINFO *info, UINT usage, DWORD rop); /* 20 */
    virtual unsigned short nearestIndex(RGBColor color); /* 21 */
    virtual Color getPixel(short x, short y); /* 22 */
    virtual RGBColor paletteColor(unsigned short index); /* 23 */
    virtual short init(); /* 24: after construction; non-zero on failure */
    virtual short lock() = 0; /* 25 */
    virtual short fillRect(const Rect &rect, Color color, short mode); /* 26 */
    virtual short fillOval(const Rect *rect, HBRUSH brush, short mode); /* 27 */
    virtual short fillPoly(short polygon, HBRUSH brush, short mode); /* 28 */
    virtual short patBlt(const Rect *rect, HBRUSH brush, unsigned short mode); /* 29 */
    virtual short fillRgn(short region, HBRUSH brush, short mode); /* 30 */
    virtual void release(); /* 31: before deleting */
    /* Scrolls a rectangle; what's uncovered goes into `region`. */
    virtual short scroll(const Rect *rect, short dx, short dy, short region); /* 32 */
    virtual Palette *setPalette(Palette *palette); /* 33: the previous one */
    virtual void unlock(); /* 34 */
    /* Draws an 8-bit (or less) DIB with colour 0 transparent. */
    virtual void drawMasked(const Rect *rect, const void *bits, BITMAPINFO *info, short flipX,
                            short flipY); /* 35 */
    virtual COLORREF colorRef(Color color); /* 36 */

    static void *operator new(size_t size); /* 0x487f56: zeroed */
    static void operator delete(void *block); /* 0x4870d1 */
    __cdecl basePort(const Rect &bounds); /* 0x486318 */
    short setFrame(const Rect *bounds, Pt origin, Pt size); /* 0x486407 */

    long tag; /* +4: 'Port' */
    basePort *next; /* +8: in graphics.ports */
    basePort *prev; /* +0xc */
    long paletteKind; /* +0x10: 1 selects its Palette's own HPALETTE (not a copy); 1 and 2 take RGB DIBs */
    long kind; /* +0x14: 5 a window */
    Rect bounds; /* +0x18: in device pixels */
    Rect frame; /* +0x20: the coordinates drawn in */
    Pt offset; /* +0x28: frame's origin less bounds' */
    Pt size; /* +0x2c: the frame's (0: the bounds') */
    short scaled; /* +0x30: frame and bounds differ in size */
    short unknown32;
    long scaleX; /* +0x34: frame over bounds, fixed-point */
    long scaleY; /* +0x38 */
    Palette *palette; /* +0x3c */
    Palette *realized; /* +0x40: the palette last realized */
    basePort *nextOnPalette; /* +0x44: in palette->ports */
    basePort *prevOnPalette; /* +0x48 */
    Color backColor; /* +0x4c */
    Font *font; /* +0x50 */
    Pen pen; /* +0x54 */
    short clip; /* +0x60: a region */
    short clipApplied; /* +0x62: clip is selected into dc */
    unsigned short locks; /* +0x64 */
    short unknown66; /* +0x66 */
    short unknown68;
    short unknown6A;
    HDC dc; /* +0x6c */
    short rasterCaps; /* +0x70 */
    unsigned short depth; /* +0x72: bits per pixel (at most 24) */
    long mapMode; /* +0x74 */
    HRGN clipRgn; /* +0x78 */
    HPALETTE hpal; /* +0x7c */
    HPEN hpen; /* +0x80 */
    HFONT hfont; /* +0x84 */
    TEXTMETRIC metrics; /* +0x88 */
};

/* A port on the display, through a display DC (displayport). */
class displayPort : public basePort
{
public:
    __cdecl displayPort(const Rect &bounds); /* 0x48ac90 */
    virtual void depthChanged(); /* 6 */
    virtual void prepare(); /* 8 */
    virtual void realizePalette(); /* 9 */
    virtual int stretchDIBits(int toX, int toY, int toWidth, int toHeight, int fromX, int fromY,
                              int fromWidth, int fromHeight, const void *bits,
                              BITMAPINFO *info, UINT usage, DWORD rop); /* 20 */
    virtual short lock(); /* 25 */
};

/* A port in memory, in the display's format: a compatible bitmap
   (memoryport). */
class memoryPort : public displayPort
{
public:
    __cdecl memoryPort(short width, short height); /* 0x48c774 */
    virtual short init(); /* 24 */
    virtual short lock(); /* 25 */
    virtual void release(); /* 31 */

    HBITMAP bitmap; /* +0xc0 */
};

/* A device-independent bitmap of any depth. */
/* What a port's getBits gives (0x22 bytes). */
class PixMap
{
public:
    void *bits;
    long rowBytes; /* negative: bottom-up */
    unsigned short depth;
    Rect dibBounds; /* +0xa */
    Rect bounds; /* +0x12 */
    Rect frame; /* +0x1a */
};

/* A port drawing into a DIB section (dibport). Declared with 4-byte
   alignment: it's 0xc8 bytes, but DIB8Port, derived from it, is 0xc6. */
#pragma pack(push, 4)
class DIBPort : public basePort
{
public:
    __cdecl DIBPort(short width, short height, unsigned short depth); /* 0x48a720 */
    virtual short copyBits(basePort *port, const Rect *to, const Rect *from, unsigned short mode,
                           unsigned short flags); /* 1 */
    virtual void prepare(); /* 8 */
    virtual void realizePalette(); /* 9 */
    virtual void getBits(PixMap *map); /* 17 */
    virtual short init(); /* 24 */
    virtual short lock(); /* 25 */
    virtual void release(); /* 31 */
    virtual void unlock(); /* 34 */

    DIB *dib; /* +0xc0 */
    short gdiPending; /* +0xc4: GDI has drawn since the last GdiFlush */
};
#pragma pack(pop)

/* An 8-bit DIB port, which draws into its bits directly where it can
   (module_48990c). */
class DIB8Port : public DIBPort
{
public:
    __cdecl DIB8Port(short width, short height); /* 0x489be4 */
    virtual short copyBits(basePort *port, const Rect *to, const Rect *from, unsigned short mode,
                           unsigned short flags); /* 1 */
    virtual void realizePalette(); /* 9 */
    virtual short drawPixels(const Rect &bounds, unsigned short width, unsigned short height,
                             short rowBytes, unsigned short format, void *pixels,
                             unsigned short mode, unsigned short flags); /* 16 */
    virtual int stretchDIBits(int toX, int toY, int toWidth, int toHeight, int fromX, int fromY,
                              int fromWidth, int fromHeight, const void *bits,
                              BITMAPINFO *info, UINT usage, DWORD rop); /* 20 */
    virtual Color getPixel(short x, short y); /* 22 */
    virtual short fillRect(const Rect &rect, Color color, short mode); /* 26 */
    virtual COLORREF colorRef(Color color); /* 36 */
    /* Draws 8-bit pixels (bottom-up rows `rowBytes` apart) with their
       origin at x, y, within `clip` and the clip region. */
    virtual void drawBits(short x, short y, unsigned short width, unsigned short height,
                          long rowBytes, const void *bits, const Rect *clip); /* 37 */

    void flush(); /* 0x48a681: GdiFlush, if GDI has drawn */
};

/* A window's port: two more slots. */
class windowPort : public displayPort
{
public:
    __cdecl windowPort(const Rect &bounds, HWND window); /* 0x48db64 */
    virtual short setClip(short region); /* 11 */
    virtual short lock(); /* 25 */
    virtual void release(); /* 31 */
    virtual short scroll(const Rect *rect, short dx, short dy, short region); /* 32 */
    virtual void unlock(); /* 34 */
    /* Takes the window's update region as a clip (after WM_PAINT). */
    virtual short beginUpdate(); /* 37 */
    virtual short endUpdate(); /* 38 */

    HWND window; /* +0xc0 */
    HRGN update; /* +0xc4: between beginUpdate and endUpdate */
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
    short rotated; /* +0xe */
    unsigned short angle; /* +0x10: of a turn (16-bit fraction) */
    unsigned short size; /* +0x12: in pixels */
    unsigned short style; /* +0x14: 1 bold, 2 italic, 4 underlined */
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
Font *newFont(const char *name, unsigned short size, unsigned short style);
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
void drawPackedPixels(unsigned char *bits, long offset, long rowBytes, Rect bounds, short x, short y,
                      const Rect &clip, unsigned short width, unsigned short height,
                      const unsigned char *data, short transparent);
unsigned char packedPixel(const unsigned char *data, unsigned short x, unsigned short y);
/* DIB8Port's blitters (dib8port) */
void copyPixels(unsigned char *bits, long offset, long rowBytes, Rect bounds, short x, short y,
                const Rect &clip, unsigned short width, unsigned short height, long fromRowBytes,
                const unsigned char *from, short transparent); /* 0x48990c */
void fillPixels(unsigned char *bits, long offset, long rowBytes, Rect bounds, const Rect &area,
                unsigned char value); /* 0x489a2d */
void expandBits(unsigned char *bits, long offset, long rowBytes, Rect bounds, short x, short y,
                const Rect &clip, unsigned short width, unsigned short height, long fromRowBytes,
                const unsigned char *from, unsigned char color); /* 0x489aad */
basePort *checkPort(basePort *port, short kind);
basePort *portObject(short kind);
short setPortError(short error);
short getPortError();
short eraseRect(const Rect &rect);
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

class Volume;
class FileRecord;

/* The file layer: files are opened by path on volumes (disks found in
   drives, or network shares), and every Win32 call that could block on a
   slow or missing disk runs on a worker thread (the async API), so the
   game can ask the user to insert the disk and try again. */

/* A worker thread for async calls; idle ones are kept in `asyncWorkers`. */
class AsyncWorker
{
public:
    __cdecl AsyncWorker(); /* 0x48213c */
    __cdecl ~AsyncWorker(); /* 0x482164 */
    short run(void (*proc)(void *data), void *data); /* 0x4821ec: waits for it */

    AsyncWorker *next;
    AsyncWorker *prev;
    HANDLE thread;
    DWORD threadId;
    HANDLE wake;
    long done; /* an OS-layer event */
    long caller; /* the waiting thread */
    void (*proc)(void *data);
    void *data;
};

/* An async call (RTTI class asyncAPI): a Win32 call to make on a worker
   thread, with its arguments and results. */
class asyncAPI
{
public:
    __cdecl asyncAPI(); /* 0x482308 */
    __cdecl ~asyncAPI(); /* 0x48231b */
    virtual void run() = 0;
    short call(); /* 0x482371 */
    /* Calls, asking to insert the disk and retrying for `path`'s errors. */
    short callFor(const char *path); /* 0x4823db */

    AsyncWorker *worker;
    long unknown8;
    DWORD error; /* the call's GetLastError, 0 if it worked */
};

class asyncCloseHandle : public asyncAPI
{
public:
    __cdecl asyncCloseHandle(HANDLE handle);
    virtual void run();
    HANDLE handle;
};

class asyncCreateDirectory : public asyncAPI
{
public:
    __cdecl asyncCreateDirectory(const char *path, SECURITY_ATTRIBUTES *security);
    virtual void run();
    const char *path;
    SECURITY_ATTRIBUTES *security;
};

class asyncCreateFile : public asyncAPI
{
public:
    __cdecl asyncCreateFile(const char *path, DWORD access, DWORD share,
                            SECURITY_ATTRIBUTES *security, DWORD creation, DWORD flags,
                            HANDLE templateFile);
    virtual void run();
    const char *path;
    DWORD access;
    DWORD share;
    SECURITY_ATTRIBUTES *security;
    DWORD creation;
    DWORD flags;
    HANDLE templateFile;
    HANDLE file; /* the result */
};

class asyncDeleteFile : public asyncAPI
{
public:
    __cdecl asyncDeleteFile(const char *path);
    virtual void run();
    const char *path;
};

class asyncFindFirstFile : public asyncAPI
{
public:
    __cdecl asyncFindFirstFile(const char *pattern, WIN32_FIND_DATA *found);
    virtual void run();
    const char *pattern;
    WIN32_FIND_DATA *found;
    HANDLE find; /* the result */
};

class asyncGetFileAttributes : public asyncAPI
{
public:
    __cdecl asyncGetFileAttributes(const char *path);
    virtual void run();
    const char *path;
    DWORD attributes; /* the result */
};

class asyncGetVolumeInformation : public asyncAPI
{
public:
    __cdecl asyncGetVolumeInformation(const char *root, char *name, DWORD nameSize,
                                      DWORD *serial, DWORD *maxComponent, DWORD *flags,
                                      char *fileSystem, DWORD fileSystemSize);
    virtual void run();
    const char *root;
    char *name;
    DWORD nameSize;
    DWORD *serial;
    DWORD *maxComponent;
    DWORD *flags;
    char *fileSystem;
    DWORD fileSystemSize;
};

class asyncReadFile : public asyncAPI
{
public:
    __cdecl asyncReadFile(HANDLE file, void *buffer, DWORD size, DWORD *read,
                          OVERLAPPED *overlapped);
    virtual void run();
    HANDLE file;
    void *buffer;
    DWORD size;
    DWORD *read;
    OVERLAPPED *overlapped;
};

class asyncRemoveDirectory : public asyncAPI
{
public:
    __cdecl asyncRemoveDirectory(const char *path);
    virtual void run();
    const char *path;
};

class asyncSetEndOfFile : public asyncAPI
{
public:
    __cdecl asyncSetEndOfFile(HANDLE file);
    virtual void run();
    HANDLE file;
};

class asyncSetFileAttributes : public asyncAPI
{
public:
    __cdecl asyncSetFileAttributes(const char *path, DWORD attributes);
    virtual void run();
    const char *path;
    DWORD attributes;
};

class asyncWriteFile : public asyncAPI
{
public:
    __cdecl asyncWriteFile(HANDLE file, const void *buffer, DWORD size, DWORD *written,
                           OVERLAPPED *overlapped);
    virtual void run();
    HANDLE file;
    const void *buffer;
    DWORD size;
    DWORD *written;
    OVERLAPPED *overlapped;
};

void asyncRun(void *call); /* 0x482362: a worker's procedure for an asyncAPI */
DWORD WINAPI asyncThread(void *worker); /* 0x4822c4 */

extern AsyncWorker *asyncWorkers; /* @data 0x4a865c */

/* What GetVolumeInformation says about a volume. */
struct DiskInfo
{
    char name[0x24];
    DWORD serial;
    unsigned short maxPath;
    unsigned short maxName;
    short casePreserved;
    short caseSensitive;
    char fileSystem[0x40];
};

/* VWIN32's DeviceIoControl registers (for its DOS IOCTL call, 1). */
struct DiocRegisters
{
    DWORD ebx;
    DWORD edx;
    DWORD ecx;
    DWORD eax;
    DWORD edi;
    DWORD esi;
    DWORD flags; /* bit 0: carry, failed */
};

/* A request to the user (FileState.askUser): 3 insert a volume. */
struct FileRequest
{
    long kind;
    short canAsk;
    short unknown6;
    long drive;
    long volume;
    char *message;
};

/* A drive letter's state. */
class Drive
{
public:
    __cdecl ~Drive(); /* 0x482b50 */
    void activate(short active); /* 0x482b80 */
    void eject(); /* 0x482bbc */
    short init(long index); /* 0x482cb3: 1 if there's no drive, or on error */
    short use(long volume); /* 0x482f2c: makes sure the volume is in the drive */
    void setLocked(short on); /* 0x483028: locks the media in (Windows 95) */
    void lock(short on); /* 0x483102: holds its mutex */
    short readInfo(DiskInfo *info); /* 0x483127 */

    long number; /* 1-based; 0 if there's no drive */
    unsigned short drive; /* 1-based */
    unsigned short letter;
    short type : 8; /* 0 removable, 1 CD-ROM, 2 fixed, 3 floppy, 4 network, 5 RAM disk */
    unsigned short valid : 1;
    unsigned short cdrom : 1;
    unsigned short remote : 1;
    unsigned short removable : 1;
    short locked; /* the media is locked in */
    long volume; /* the volume in it, 0 if not known */
    long mutex;
};

/* The drives, 'A' on. */
struct DriveTable
{
    unsigned short count;
    short unknown2;
    Drive drives[1];
};

/* A volume the file layer knows ('Volm'): a disk seen in a drive, or a
   network share. Handles to it are its address. */
class Volume
{
public:
    __cdecl Volume(long drive, DiskInfo *info); /* 0x485e08: a disk */
    __cdecl Volume(const char *share, DiskInfo *info); /* 0x485e62: a share */
    __cdecl ~Volume(); /* 0x485ebb */
    static void *__cdecl operator new(size_t size); /* 0x485ddd: zeroed */
    static void __cdecl operator delete(void *block); /* 0x4860bc */
    void touch(unsigned long time); /* 0x485f17 */
    void rootPath(char *path); /* 0x486007 */
    short mount(); /* 0x486050: makes sure it's in its drive */

    unsigned long tag;
    Volume *prev;
    Volume *next;
    long id;
    DiskInfo info;
    char unknown80[4];
    unsigned long lastUsed;
    Drive *drive; /* 0 for a share */
    char *share;
};

/* An open file ('File'). Handles to it are its address. */
class FileRecord
{
public:
    __cdecl FileRecord(); /* 0x483e4c */
    __cdecl ~FileRecord(); /* 0x483e75 */
    static void *__cdecl operator new(size_t size); /* 0x484bdc: zeroed */
    static void __cdecl operator delete(void *block); /* 0x484440 */
    short open(const fileSpec &spec, unsigned short mode); /* 0x483ebb */
    short lock(short on); /* 0x48409b */
    short close(); /* 0x4840ed */

    unsigned long tag;
    FileRecord *next;
    FileRecord *prev;
    Volume *volume;
    long mutex;
    unsigned short mode;
    short kind;
    HANDLE handle;
    char path[0x100]; /* shortened to fit once open */
};

Volume *volumeOf(long id); /* 0x485dc5: 0 if it isn't a volume */
/* The known volume with that label, in the drive (or any removable drive
   for a removable one) or on the share; 0 if none. */
long findVolume(DiskInfo *info, long drive, const char *share); /* 0x485f2a */

/* The file layer's state. */
struct FileState
{
    short error; /* of the last call */
    short ready;
    short active; /* the application is active */
    short unknown6;
    ActivateHook previousHook;
    Volume *volumes;
    FileRecord *files;
    AsyncWorker *spareWorker;
    DriveTable *drives;
    short (*askUser)(void *request); /* to insert a disk, retry, ... */
    unsigned short canAsk;
    short unknown22;
    fileSpec currentDirectory;
    fileSpec programDirectory;
    fileSpec appDirectory; /* programDirectory's answer */
    fileSpec tempDirectory;
};

extern FileState files; /* @data 0x4b9b6c */

short setFileError(short error); /* 0x4861e7 */
long driveNumber(char letter); /* 0x485d70: 0 if there's no such drive */
Drive *driveAt(long number, short check); /* 0x48607c */
/* Asks the user what to do about a Win32 error on `path` (insert the disk,
   retry); 0 to give up. */
short askAboutError(const char *path, DWORD error); /* 0x4835d8 */
short readDiskInfo(const char *root, DiskInfo *info); /* 0x4834aa */
short askFileUser(FileRequest *request); /* 0x484365: 0 to give up */
void filesActivated(short active); /* 0x483464: the activate hook */
short __fastcall fileLayerVersion(); /* 0x483a22: 0x500, or 0 if not initialised */
void __cdecl closeFiles(); /* 0x483a36 */
FileRecord *findOpenFile(long volume, const char *path); /* 0x483c8a */
short fileErrorOf(DWORD error); /* 0x483cfb: a Win32 error as the file layer's */
long mountDrive(long number); /* 0x484a4c: finds the volume in a drive, 0 if none */
short getAttributes(const char *path, DWORD *attributes); /* 0x483557 */
short setAttributes(const char *path, DWORD attributes); /* 0x483ad5 */
/* Whether a directory is one of the engine's or holds an open file, or a
   file is open. */
short fileInUse(long volume, const char *path, DWORD attributes); /* 0x483b2e */
/* Creates a file (mode 1) or directory (mode 2), hidden (4), read-only (8). */
short createPath(const fileSpec &spec, unsigned short mode); /* 0x4826b4 */
FileRecord *fileOf(long handle, short kind); /* 0x486240: 0 if it isn't an open file */

/* The files resources come from (the async file API) */
struct VolumeInfo
{
    char name[0x22];
    unsigned short maxPath;
    unsigned short maxName;
    short async; /* reads can go on in the background (always) */
    short casePreserved;
    short caseSensitive;
    short readOnly; /* a CD-ROM */
    short unknown2E;
    long drive; /* its number, 0 for a share */
    char *share;
};

short lockFile(long file, long timeout); /* 0x4860cc: 0, or 0x283d if it timed out */
void unlockFile(long file); /* 0x484d98 */
unsigned long seekFile(long file, unsigned long offset, long whence); /* 0x484dc4: -1 on error */
short writeFile(long file, const void *buffer, long *size); /* 0x48610c */
short setFileSize(long file, unsigned long size); /* 0x484f3c */
unsigned long fileLength(long file); /* 0x4845fc */
short fileSpecOf(long file, fileSpec *spec); /* 0x484934 */
short volumeInfo(long volume, VolumeInfo *info); /* 0x4849ac */
long setAskUser(long handler); /* 0x4850d4: sets FileState.askUser, returning the previous one */

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
 * Sounds: MIDI and wave objects (audioObj, tagged 'AObj'), handed around
 * as handles (their addresses) and kept in a list. Errors of the last call
 * are in sound.error.
 */

/* Told when a sound starts (4) or stops (5), and of its progress. */
typedef void (*SoundNotify)(long sound, SoundNotice *notice, long cookie);

class audioObj
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
    audioObj *next;
    audioObj *prev;
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
    audioObj *objects;
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
short forEachSound(long kind, unsigned short device, short (*proc)(audioObj *object, long data),
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
audioObj *audioObject(long sound); /* 0 if it isn't one */
unsigned short __cdecl makeWord(unsigned char low, unsigned char high);
long newSound(short data); /* from a Mohawk MIDI or WAVE in a handle */
long newStreamedSound(long resource, long);
/* Called but not decompiled yet */
short openWaveOut(long *out, unsigned short device, PCMWAVEFORMAT *format, long, long,
                  long flags); /* 0x47c712 */
short getWaveCaps(unsigned short device, void *caps, long size); /* 0x47c432 */
short fn_47a074(short open); /* MMSYSERR_NOTSUPPORTED */
unsigned short initMidi(); /* 0x47a07f */
void closeMidi(); /* 0x47a0c0 */
short fn_47c62c();
void fn_47c995();
long __cdecl parseNumber(const char *text); /* 0x47a066 */
short setSoundError(short error); /* 0x47de96 */
void __cdecl notifySound(audioObj *object, SoundNotice *notice); /* 0x47e0d3 */
audioObj *__cdecl newMidiSound(short data); /* 0x478f0b */
void *__cdecl operator new(size_t size, void *where); /* 0x47dea7: zeroed */
audioObj *__cdecl newWaveSound(short data); /* 0x47a28d */
audioObj *__cdecl newStreamedWave(long resource, long file, long preloadMs); /* 0x47cd5c */

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
class waveObj;
struct WaveBlock
{
    Deferred call; /* runs waveBlockDone */
    waveObj *sound;
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
class waveObj : public audioObj
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

/* A buffer a streamed wave sound reads into and queues (in a ring). */
class wavestreamObj;
struct StreamBuffer
{
    Deferred call; /* runs streamBufferDone */
    StreamBuffer *next;
    StreamBuffer *prev;
    wavestreamObj *sound;
    unsigned long start; /* in samples */
    unsigned long length;
    unsigned char *cue; /* the cue point it ends at */
    unsigned long capacity; /* in samples */
    unsigned long size; /* in bytes */
    short prepared;
    short queued;
    short done;
    short unknown3A;
    long keep; /* the loop's first buffer, kept */
    WAVEHDR header;
    unsigned char data[1];
};

/* A wave sound played from its resource's file (or a file of its own) as it
   goes: a thread reads ahead into buffers, up to three seconds' worth, after
   an optional preloaded start. */
class wavestreamObj : public audioObj
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

    StreamBuffer *__cdecl newBuffer(unsigned long samples);
    void __cdecl freeBuffer(StreamBuffer *buffer);
    short __cdecl prepareBuffer(StreamBuffer *buffer);
    short __cdecl readBuffer(StreamBuffer *buffer);
    short __cdecl queueBuffer(StreamBuffer *buffer);
    void __cdecl unprepareBuffer(StreamBuffer *buffer);
    short __cdecl stream();
    unsigned long __cdecl positionAt(unsigned long sample);

    long wave;
    long file;
    long resource; /* 0: the file is its own */
    unsigned char *cues; /* Cue#, byte-swapped */
    short streaming;
    short unknown5A;
    long thread;
    long event; /* wakes the thread */
    unsigned short queued;
    short unknown66;
    unsigned long queuedSamples;
    StreamBuffer *ring;
    unsigned long bufferSamples;
    unsigned long maxQueued; /* in samples */
    unsigned long readPosition;
    short atEnd;
    unsigned short blockAlign;
    unsigned long dataOffset; /* of the samples, in the file */
    unsigned long start; /* in samples */
    long samplesPerMs; /* fixed-point */
    long msPerSample;
    unsigned long base; /* the device's position 0, in samples */
    short resetting;
    short loopDone;
    unsigned short loopsPlayed;
    unsigned short loopsRead;
    unsigned long loopAdjust;
    StreamBuffer *loopBuffer;
    StreamBuffer *loopEndBuffer;
    unsigned short sampleRate;
    short unknownAA;
    unsigned long sampleCount;
    unsigned char bitsPerSample;
    unsigned char channels;
    unsigned short encoding;
    unsigned short loops;
    short unknownB6;
    unsigned long loopStart;
    unsigned long loopEnd;
    unsigned char *preload;
    unsigned long preloadSamples;
};

void streamThread(long wave); /* 0x47db85 */
short __cdecl readStream(long resource, long file, void *buffer, unsigned long *size,
                         unsigned long offset); /* 0x47dc02 */
void CALLBACK streamCallback(long wave, unsigned short message, DWORD instance, DWORD header,
                             DWORD); /* 0x47df34 */
void streamBufferDone(void *buffer); /* 0x47df7a */
void waveBlockDone(void *block); /* 0x47ae62 */
void CALLBACK waveCallback(long wave, unsigned short message, DWORD instance, DWORD header, DWORD); /* 0x47ae14 */
/* DirectSound (loaded at run time from DSOUND.DLL, when [WaveMix]
   fEnableDirectSound is set): the parts WaveMix uses. BCC32 4.5 predates
   dsound.h. */
struct DSCAPS
{
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwMinSecondarySampleRate;
    DWORD dwMaxSecondarySampleRate;
    DWORD unknown10[20];
};

#define DSCAPS_SECONDARYMONO 0x100
#define DSCAPS_SECONDARYSTEREO 0x200
#define DSCAPS_SECONDARY8BIT 0x400
#define DSCAPS_SECONDARY16BIT 0x800

struct DSBUFFERDESC
{
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwBufferBytes;
    DWORD dwReserved;
    WAVEFORMATEX *lpwfxFormat;
};

#define DSERR_ALLOCATED 0x8878000aL
#define DSERR_BADFORMAT 0x88780064L
#define DSERR_OUTOFMEMORY 0x8007000eL

class IDirectSoundBuffer
{
public:
    virtual long __stdcall QueryInterface(const GUID &iid, void **object) = 0;
    virtual unsigned long __stdcall AddRef() = 0;
    virtual unsigned long __stdcall Release() = 0;
    virtual long __stdcall GetCaps(void *caps) = 0;
    virtual long __stdcall GetCurrentPosition(DWORD *play, DWORD *write) = 0;
    virtual long __stdcall GetFormat(WAVEFORMATEX *format, DWORD size, DWORD *written) = 0;
    virtual long __stdcall GetVolume(long *volume) = 0;
    virtual long __stdcall GetPan(long *pan) = 0;
    virtual long __stdcall GetFrequency(DWORD *frequency) = 0;
    virtual long __stdcall GetStatus(DWORD *status) = 0;
    virtual long __stdcall Initialize(void *sound, const DSBUFFERDESC *desc) = 0;
    virtual long __stdcall Lock(DWORD offset, DWORD bytes, void **first, DWORD *firstBytes,
                                void **second, DWORD *secondBytes, DWORD flags) = 0;
    virtual long __stdcall Play(DWORD reserved1, DWORD reserved2, DWORD flags) = 0;
    virtual long __stdcall SetCurrentPosition(DWORD position) = 0;
    virtual long __stdcall SetFormat(const WAVEFORMATEX *format) = 0;
    virtual long __stdcall SetVolume(long volume) = 0;
    virtual long __stdcall SetPan(long pan) = 0;
    virtual long __stdcall SetFrequency(DWORD frequency) = 0;
    virtual long __stdcall Stop() = 0;
    virtual long __stdcall Unlock(void *first, DWORD firstBytes, void *second, DWORD secondBytes) = 0;
    virtual long __stdcall Restore() = 0;
};

#define DSBPLAY_LOOPING 1

class IDirectSound
{
public:
    virtual long __stdcall QueryInterface(const GUID &iid, void **object) = 0;
    virtual unsigned long __stdcall AddRef() = 0;
    virtual unsigned long __stdcall Release() = 0;
    virtual long __stdcall CreateSoundBuffer(const DSBUFFERDESC *desc, IDirectSoundBuffer **buffer,
                                             void *outer) = 0;
    virtual long __stdcall GetCaps(DSCAPS *caps) = 0;
    virtual long __stdcall DuplicateSoundBuffer(IDirectSoundBuffer *original,
                                                IDirectSoundBuffer **duplicate) = 0;
    virtual long __stdcall SetCooperativeLevel(HWND window, DWORD level) = 0;
};

#define DSSCL_NORMAL 1

typedef BOOL(CALLBACK *DSENUMCALLBACK)(GUID *guid, const char *description, const char *module,
                                       void *context);

/* WaveMix's output buffer (RTTI class wavebuf): a ring of samples the mixer
   writes ahead into while the device plays them, through waveOut
   (wavebufWO) or a looping DirectSound buffer (wavebufDS). Positions count
   samples since opening. The owner is told (through a deferred call) when
   more can be written. */
typedef void (*WavebufNotify)(long data, unsigned long played, unsigned long written);

class wavebuf
{
public:
    virtual __cdecl ~wavebuf();
    virtual short __cdecl close() = 0;
    virtual void __cdecl lock() = 0;
    virtual void __cdecl unlock() = 0;
    virtual short __cdecl position(unsigned long *played, unsigned long *written) = 0;
    /* The space from `at` (at least the written position) to write in, in
       one or two pieces (the ring wraps). */
    virtual short __cdecl lockBuffer(unsigned long at, void **first, unsigned long *firstLength,
                                     void **second, unsigned long *secondLength) = 0;
    virtual short __cdecl open(PCMWAVEFORMAT *format, WavebufNotify notify, long data) = 0;
    virtual void __cdecl formats(unsigned long *rate, unsigned long *formats) = 0;
    virtual short __cdecl start() = 0;
    virtual short __cdecl unlockBuffer() = 0;

    char name[32]; /* the device's */
};

/* Through waveOut: a ring of blocks, written ahead by a thread polling the
   device's position. */
class wavebufWO : public wavebuf
{
public:
    __cdecl wavebufWO(unsigned short device);
    virtual __cdecl ~wavebufWO();
    virtual short __cdecl close();
    virtual void __cdecl lock();
    virtual void __cdecl unlock();
    virtual short __cdecl position(unsigned long *played, unsigned long *written);
    virtual short __cdecl lockBuffer(unsigned long at, void **first, unsigned long *firstLength,
                                     void **second, unsigned long *secondLength);
    virtual short __cdecl open(PCMWAVEFORMAT *format, WavebufNotify notify, long data);
    virtual void __cdecl formats(unsigned long *rate, unsigned long *formats);
    virtual short __cdecl start();
    virtual short __cdecl unlockBuffer();

    static void *__cdecl operator new(size_t size);
    static void __cdecl operator delete(void *block);
    short __cdecl readDeviceInfo(const char *key);
    unsigned long __cdecl devicePosition();
    short __cdecl fill(unsigned long minimum);

    unsigned short device;
    short unknown26;
    WAVEOUTCAPS caps;
    DeferLock lockState;
    Deferred call; /* runs wavebufWONotify */
    short supported; /* not "not supported" in [WaveMix.DeviceInfo] */
    short unknown82;
    unsigned long blockCount; /* from [WaveMix.DeviceInfo] */
    unsigned long blockSamples;
    unsigned long ahead; /* samples to keep written ahead */
    unsigned long prime; /* samples to write before starting */
    short isOpen;
    short unknown96;
    PCMWAVEFORMAT format;
    WavebufNotify notify;
    long data;
    HWAVEOUT wave;
    WAVEHDR *headers;
    unsigned char *buffer;
    long thread;
    unsigned long locked;
    unsigned long totalSamples;
    short started;
    short unknownCA;
    unsigned long done; /* the furthest block end the device has finished */
    unsigned long played;
    unsigned long written;
};

/* Through DirectSound: a looping buffer of one second. */
class wavebufDS : public wavebuf
{
public:
    __cdecl wavebufDS(unsigned short device);
    virtual __cdecl ~wavebufDS();
    virtual short __cdecl close();
    virtual void __cdecl lock();
    virtual void __cdecl unlock();
    virtual short __cdecl position(unsigned long *played, unsigned long *written);
    virtual short __cdecl lockBuffer(unsigned long at, void **first, unsigned long *firstLength,
                                     void **second, unsigned long *secondLength);
    virtual short __cdecl open(PCMWAVEFORMAT *format, WavebufNotify notify, long data);
    virtual void __cdecl formats(unsigned long *rate, unsigned long *formats);
    virtual short __cdecl start();
    virtual short __cdecl unlockBuffer();

    static void *__cdecl operator new(size_t size);
    static void __cdecl operator delete(void *block);

    unsigned short device;
    unsigned short enumerated; /* devices seen while enumerating */
    short available;
    short unknown2A;
    GUID guid;
    CRITICAL_SECTION section;
    DeferLock lockState;
    Deferred call; /* runs wavebufDSNotify */
    IDirectSound *directSound;
    DSCAPS caps;
    short isOpen;
    short unknownDE;
    WAVEFORMATEX format;
    short unknownF2;
    WavebufNotify notify;
    long data;
    IDirectSoundBuffer *buffer;
    unsigned long bufferSamples;
    HANDLE thread;
    DWORD playCursor; /* in bytes */
    unsigned long played;
    unsigned long playWraps;
    DWORD writeCursor;
    unsigned long written;
    unsigned long writeWraps;
    short locked;
    short unknown122;
    void *lockFirst;
    DWORD lockFirstBytes;
    void *lockSecond;
    DWORD lockSecondBytes;
    short started;
    short unknown136;
};

short __cdecl newWavebuf(unsigned short device, wavebuf **buffer); /* 0x47af3b */
short __cdecl initWavebuf(); /* 0x47b000 */
void __cdecl closeWavebuf(); /* 0x47b015 */
void wavebufWONotify(void *data); /* 0x47b1d5 */
void CALLBACK wavebufWOCallback(HWAVEOUT wave, UINT message, DWORD instance, DWORD header,
                                DWORD); /* 0x47b360 */
void __fastcall forgetWavebufCache(); /* 0x47b46f */
void __cdecl freeWavebufCache(); /* 0x47b7c2 */
void wavebufWOThread(long data); /* 0x47b856 */
BOOL CALLBACK enumerateDirectSound(GUID *guid, const char *description, const char *module,
                                   void *context); /* 0x47b9d0 */
void wavebufDSNotify(void *data); /* 0x47bb80 */
short __cdecl loadDirectSound(); /* 0x47bd45 */
void __cdecl freeDirectSound(); /* 0x47c1a7 */
DWORD WINAPI wavebufDSThread(void *data); /* 0x47c229 */

extern short useDirectSound; /* @data 0x4a82d0 */
extern short wavebufCache; /* @data 0x4a82d4: a handle kept for waveOut buffers */
extern HINSTANCE directSoundLibrary; /* @data 0x4a8358 */
extern long(WINAPI *directSoundCreate)(GUID *guid, IDirectSound **sound, void *outer); /* @data 0x4b9b38 */
extern long(WINAPI *directSoundEnumerate)(DSENUMCALLBACK callback, void *context); /* @data 0x4b9b3c */

/* WaveMix (the engine's software mixer): its objects stand in for waveOut
   devices, handed out as handles by wavebufOpen and used through the other
   wavebuf API functions, which mirror waveOut's. Mixed objects share a
   wmxDevice (one per wave device, owning its wavebuf); objects opened with
   bit 31 of the flags set are plain wmxObjects; when WaveMix is disabled
   they're wmxWaveOuts, which pass everything to waveOut. */
class wmxDevice;

class wmxObject
{
public:
    __cdecl wmxObject(wmxDevice *device, PCMWAVEFORMAT *format, long callback, long instance,
              unsigned long flags);
    virtual __cdecl ~wmxObject();
    /* Mixes (or with `first`, copies) `count` samples from `at` into `out`;
       0 if it has nothing there. */
    virtual short __cdecl mix(unsigned long at, void *out, unsigned long count, short first);
    virtual void __cdecl played(unsigned long at); /* the device has played to `at` */
    virtual unsigned short __cdecl breakLoop();
    virtual unsigned short __cdecl close();
    virtual unsigned short __cdecl getLevels(unsigned long *levels);
    virtual unsigned short __cdecl getID(unsigned short *id);
    virtual unsigned short __cdecl getPitch(unsigned long *pitch);
    virtual unsigned short __cdecl getPlaybackRate(unsigned long *rate);
    virtual unsigned short __cdecl getPosition(MMTIME *time, unsigned short size);
    virtual unsigned short __cdecl getVolume(unsigned long *volume);
    virtual unsigned short __cdecl pause();
    virtual unsigned short __cdecl prepareHeader(WAVEHDR *header, unsigned short size);
    virtual unsigned short __cdecl reset();
    virtual unsigned short __cdecl restart();
    /* The left and right levels (low and high words), as waveOutSetVolume's. */
    virtual unsigned short __cdecl setLevels(unsigned long levels);
    virtual unsigned short __cdecl setPitch(unsigned long pitch);
    virtual unsigned short __cdecl setPlaybackRate(unsigned long rate);
    virtual unsigned short __cdecl setVolume(unsigned long volume);
    virtual unsigned short __cdecl unprepareHeader(WAVEHDR *header, unsigned short size);
    virtual unsigned short __cdecl write(WAVEHDR *header, unsigned short size);

    static void *__cdecl operator new(size_t size);
    static void __cdecl operator delete(void *block);
    void notify(unsigned short message, long param1, long param2); /* 0x47f9f6 */

    unsigned long tag; /* 'WMix' */
    wmxObject *next; /* in wmx.objects */
    wmxObject *prev;
    wmxDevice *device;
    PCMWAVEFORMAT format;
    long callback;
    long instance;
    unsigned long flags;
    wmxObject *deviceNext; /* in device->objects */
    wmxObject *devicePrev;
};

/* A block queued on a wmxMixer (made by prepareHeader, found through the
   header's `reserved`). Positions and lengths are in the mix's samples. */
struct WmxBlock
{
    WAVEHDR *header;
    WmxBlock *next;
    WmxBlock *prev;
    unsigned long samples; /* in the object's format */
    unsigned long length; /* once through */
    unsigned long start;
    unsigned long end; /* after its loops */
    unsigned long loops; /* more times round the loop it's in */
    unsigned long loopSamples;
    unsigned long loopLength;
    unsigned long loopStart;
    unsigned long loopTotal;
    unsigned long loopsDone;
};

/* A mixing loop (module_47fae8): see there. */
typedef void (*WmxMixProc)(unsigned char *out, const unsigned char *in, unsigned long count,
                           long outStride, long inStride, unsigned long step, long volume,
                           short identity, const unsigned char *table);

/* One output channel of a wmxMixer. */
struct WmxChannel
{
    long volume; /* fixed-point */
    WmxMixProc mix; /* adds into the output */
    WmxMixProc copy; /* for the first object mixed */
    long mixOffset; /* bytes into an output sample */
    long copyOffset;
    long inOffset; /* bytes into an input sample */
    unsigned char table[0x100]; /* 8-bit samples at the volume */
    short identity; /* the table changes nothing */
    short unknown11A;
};

/* A WaveMix object whose blocks are mixed in software into its device's
   output. */
class wmxMixer : public wmxObject
{
public:
    __cdecl wmxMixer(wmxDevice *device, PCMWAVEFORMAT *format, long callback, long instance,
                     unsigned long flags);
    virtual __cdecl ~wmxMixer();
    virtual short __cdecl mix(unsigned long at, void *out, unsigned long count, short first);
    virtual void __cdecl played(unsigned long at);
    virtual unsigned short __cdecl breakLoop();
    virtual unsigned short __cdecl close();
    virtual unsigned short __cdecl getLevels(unsigned long *levels);
    virtual unsigned short __cdecl getID(unsigned short *id);
    virtual unsigned short __cdecl getPitch(unsigned long *pitch);
    virtual unsigned short __cdecl getPlaybackRate(unsigned long *rate);
    virtual unsigned short __cdecl getPosition(MMTIME *time, unsigned short size);
    virtual unsigned short __cdecl getVolume(unsigned long *volume);
    virtual unsigned short __cdecl pause();
    virtual unsigned short __cdecl prepareHeader(WAVEHDR *header, unsigned short size);
    virtual unsigned short __cdecl reset();
    virtual unsigned short __cdecl restart();
    virtual unsigned short __cdecl setLevels(unsigned long levels);
    virtual unsigned short __cdecl setPitch(unsigned long pitch);
    virtual unsigned short __cdecl setPlaybackRate(unsigned long rate);
    virtual unsigned short __cdecl setVolume(unsigned long volume);
    virtual unsigned short __cdecl unprepareHeader(WAVEHDR *header, unsigned short size);
    virtual unsigned short __cdecl write(WAVEHDR *header, unsigned short size);

    short buildTable(long volume, unsigned char *table); /* 0x47e6d9 */
    void retire(WmxBlock *block); /* 0x47e794 */
    void findBlock(unsigned long at, WmxBlock **block, unsigned long *start); /* 0x47e7ed */
    void retime(unsigned long at); /* 0x47ea78 */
    void chooseMixers(); /* 0x47ec2c */

    unsigned long donePosition; /* samples of the blocks played out */
    unsigned long position; /* in samples */
    WmxBlock *queue;
    WmxBlock *queueTail;
    unsigned long levels;
    unsigned long volume; /* fixed-point */
    long step; /* output samples per input sample, fixed-point */
    unsigned long rate; /* fixed-point */
    short paused;
    short unknown5A;
    unsigned long pauseOffset;
    unsigned short channels;
    short unknown62;
    WmxChannel channel[2];
};

/* Straight through to waveOut. */
class wmxWaveOut : public wmxObject
{
public:
    __cdecl wmxWaveOut(PCMWAVEFORMAT *format, long callback, long instance, unsigned long flags);
    virtual __cdecl ~wmxWaveOut();
    virtual unsigned short __cdecl breakLoop();
    virtual unsigned short __cdecl close();
    virtual unsigned short __cdecl getID(unsigned short *id);
    virtual unsigned short __cdecl getPitch(unsigned long *pitch);
    virtual unsigned short __cdecl getPlaybackRate(unsigned long *rate);
    virtual unsigned short __cdecl getPosition(MMTIME *time, unsigned short size);
    virtual unsigned short __cdecl getVolume(unsigned long *volume);
    virtual unsigned short __cdecl pause();
    virtual unsigned short __cdecl prepareHeader(WAVEHDR *header, unsigned short size);
    virtual unsigned short __cdecl reset();
    virtual unsigned short __cdecl restart();
    virtual unsigned short __cdecl setPitch(unsigned long pitch);
    virtual unsigned short __cdecl setPlaybackRate(unsigned long rate);
    virtual unsigned short __cdecl setVolume(unsigned long volume);
    virtual unsigned short __cdecl unprepareHeader(WAVEHDR *header, unsigned short size);
    virtual unsigned short __cdecl write(WAVEHDR *header, unsigned short size);

    unsigned short open(unsigned short device, unsigned long flags); /* 0x47fddc */

    HWAVEOUT wave;
    WAVEOUTCAPS caps;
};

/* A wave device WaveMix mixes for. */
class wmxDevice
{
public:
    __cdecl wmxDevice(); /* 0x47e0ec */
    __cdecl ~wmxDevice(); /* 0x47e0f9 */
    static void *__cdecl operator new(size_t size);
    static void __cdecl operator delete(void *block);
    unsigned short open(unsigned short device); /* 0x47e1a0 */
    void mix(unsigned long at); /* 0x47e45e */
    void mixInto(unsigned long at, void *out, unsigned long count); /* 0x47e4d2 */
    void silence(void *out, unsigned long count); /* 0x47e5b8 */

    short isOpen;
    unsigned short device;
    wmxDevice *next; /* in wmx.devices */
    wmxDevice *prev;
    PCMWAVEFORMAT format; /* the mix's */
    wavebuf *buffer;
    unsigned long written; /* how far the buffer is mixed */
    unsigned short objectCount;
    short unknown26;
    wmxObject *objects;
};

/* The extended caps wavebufGetDevCaps fills in when asked for them. */
struct WmxCaps
{
    WAVEOUTCAPS caps;
    unsigned long rate;
    unsigned long formats;
};

struct WmxState
{
    short initialized;
    short stereo; /* [WaveMix] fStereo */
    unsigned long rate; /* [WaveMix] ulFrameRate */
    unsigned long frameSize; /* [WaveMix] ulFrameSize */
    wmxObject *objects;
    wmxDevice *devices;
    short enabled; /* [WaveMix] fEnable */
    short unknown16;
};

extern WmxState wmx; /* @data 0x4b9b40 */

void wmxDeviceFormats(wavebuf *buffer, unsigned long *rate, unsigned long *formats); /* 0x47e52b */
void wmxDeviceNotify(long data, unsigned long played, unsigned long written); /* 0x47e163 */
void CALLBACK wmxWaveOutCallback(HWAVEOUT wave, UINT message, DWORD instance, DWORD param1,
                                 DWORD param2); /* 0x47fdc2 */
/* An object's callback (CALLBACK_FUNCTION), with a word or a 32-bit message. */
typedef void(CALLBACK *WmxShortCallback)(long handle, unsigned short message, long instance,
                                         long param1, long param2);
typedef void(CALLBACK *WmxCallback)(long handle, UINT message, long instance, long param1,
                                    long param2);
void mixByteIntoByte(unsigned char *out, const unsigned char *in, unsigned long count,
                     long outStride, long inStride, unsigned long step, long volume,
                     short identity, const unsigned char *table); /* 0x47fae8 */
void mixByteIntoWord(unsigned char *out, const unsigned char *in, unsigned long count,
                     long outStride, long inStride, unsigned long step, long volume,
                     short identity, const unsigned char *table); /* 0x47fb3b */
void mixWordIntoWord(unsigned char *out, const unsigned char *in, unsigned long count,
                     long outStride, long inStride, unsigned long step, long volume,
                     short identity, const unsigned char *table); /* 0x47fb8a */
void copyByteToByte(unsigned char *out, const unsigned char *in, unsigned long count,
                    long outStride, long inStride, unsigned long step, long volume,
                    short identity, const unsigned char *table); /* 0x47fbdc */
void copyByteToWord(unsigned char *out, const unsigned char *in, unsigned long count,
                    long outStride, long inStride, unsigned long step, long volume,
                    short identity, const unsigned char *table); /* 0x47fc45 */
void copyWordToWord(unsigned char *out, const unsigned char *in, unsigned long count,
                    long outStride, long inStride, unsigned long step, long volume,
                    short identity, const unsigned char *table); /* 0x47fcad */
void fillBytes(void *out, unsigned char value, unsigned long count); /* 0x47fd11 */
void fillWords(void *out, unsigned short value, unsigned long count); /* 0x47fd3b */
wmxObject *wmxObjectOf(long handle); /* 0x47caf0 */
short __cdecl initWaveMix(); /* 0x47c62c */
void __cdecl closeWaveMix(); /* 0x47c995 */
unsigned short wavebufOpen(long *handle, unsigned short device, PCMWAVEFORMAT *format,
                           long callback, long instance, unsigned long flags); /* 0x47c712 */
unsigned short wavebufGetDevCaps(unsigned short device, WmxCaps *caps, unsigned short size); /* 0x47c432 */

unsigned short wavebufBreakLoop(long handle); /* 0x47c3b4 */
unsigned short wavebufClose(long handle); /* 0x47c3d4 */
unsigned short wavebufGetLevels(long handle, unsigned long *levels); /* 0x47c40d */
unsigned short wavebufGetID(long handle, unsigned short *id); /* 0x47c56e */
unsigned short wavebufGetPitch(long handle, unsigned long *pitch); /* 0x47c593 */
unsigned short wavebufGetPlaybackRate(long handle, unsigned long *rate); /* 0x47c5b8 */
unsigned short wavebufGetPosition(long handle, MMTIME *time, unsigned short size); /* 0x47c5dd */
unsigned short wavebufGetVolume(long handle, unsigned long *volume); /* 0x47c607 */
unsigned short wavebufPause(long handle); /* 0x47c94b */
unsigned short wavebufPrepareHeader(long handle, WAVEHDR *header, unsigned short size); /* 0x47c96b */
unsigned short wavebufReset(long handle); /* 0x47c9c8 */
unsigned short wavebufRestart(long handle); /* 0x47c9e8 */
unsigned short wavebufSetLevels(long handle, unsigned long levels); /* 0x47ca08 */
unsigned short wavebufSetPitch(long handle, unsigned long pitch); /* 0x47ca2d */
unsigned short wavebufSetPlaybackRate(long handle, unsigned long rate); /* 0x47ca52 */
unsigned short wavebufSetVolume(long handle, unsigned long volume); /* 0x47ca77 */
unsigned short wavebufUnprepareHeader(long handle, WAVEHDR *header, unsigned short size); /* 0x47ca9c */
unsigned short wavebufWrite(long handle, WAVEHDR *header, unsigned short size); /* 0x47cac6 */

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
class midiObj : public audioObj
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
long createThread(void (*proc)(long), long argument, unsigned short stackSize,
                  unsigned short priority); /* 0x46e302: suspended */
/* Threads, events and mutexes are OS-layer sync objects, deleted, waited
   for (an event set, a mutex acquired) the same way. */
short deleteSync(long sync); /* 0x46e463 */
void resumeThread(long thread); /* 0x46e857 */
void yieldThread(long); /* 0x46eb9f */
void setThreadPriority(long thread, unsigned short priority); /* 0x46eadd */
unsigned short threadPriority(long thread); /* 0x46e605 */
long currentThread(); /* 0x46e5dc */
long mainThread(); /* 0x46e5f4 */
void stopOtherThreads(); /* 0x46e2a4 */
void reschedule(unsigned long now); /* 0x46e5a4 */
void timesliceProc(long timer, long data); /* 0x46e71a */
void threadExit(); /* 0x46e8e0: where a thread's procedure returns to */
short schedule(unsigned long now); /* 0x46e90e: whether it switched threads */
sync *__cdecl syncOf(long sync, long kind); /* 0x46f442: 0 if it isn't one (of the kind) */
short initContext(Context *context, void (*proc)(long), long argument,
                  unsigned short stackSize); /* 0x46f5c0 */
short freeContext(Context *context); /* 0x46f68f */
void resumeContext(Context *context); /* 0x46f6c9 */
void abandonContext(Context *context); /* 0x46f6f9 */
void switchContext(Context *to, Context *save); /* 0x46f70e */
void disableScheduling(); /* 0x46e410 */
void enableScheduling(); /* 0x46e43a */
short initThreads(char *stacks, char *end); /* 0x46e658 */
void stopThreads(); /* 0x46e749 */
void suspendThread(long thread); /* 0x46ebca */
long newEvent(short); /* 0x46e380 */
void setEvent(long event); /* 0x46ea83 */
void resetEvent(long event); /* 0x46e7fd */
short waitSync(long sync, long timeout); /* 0x46ecb2: 0 when set (or acquired); 0x12e timed out */
long newMutex(short); /* 0x46e3c8 */
void releaseMutex(long mutex); /* 0x46e78c */

/* Rectangles (QuickDraw's) */
short emptyRect(ShortRect *rect);
ShortRect *offsetRect(ShortRect *rect, short dx, short dy); /* the rectangle */
void insetRect(ShortRect *rect, short dx, short dy);
short sectRect(ShortRect *rect, ShortRect *with);
ShortRect *unionRect(ShortRect *into, ShortRect *add);
short ptInRect(ShortRect *rect, const Point &point);
ShortRect *__cdecl setRect(ShortRect *rect, short left, short top, short right, short bottom);

/* Regions (errors in regionError) */
short newRgn();
void disposeRgn(short region);
short setEmptyRgn(short region);
unsigned short emptyRgn(short region);
short setRectRgn(short region, ShortRect *rect);
short copyRgn(short to, short from);
void compactRgn(short region);
short regionToHrgn(HRGN target, short region, short dx, short dy);
short sectRgnWithRect(short region, ShortRect *rect);
short sectRgnRects(short region, long count, ShortRect *rects);
short diffRgnRect(short region, ShortRect *rect);
short diffRgnRects(short region, long count, ShortRect *rects);
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
void fn_46c148(long *resource, unsigned short first, unsigned short member, const char *name);
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
/* basecamp */
unsigned short loadWave(short key);
unsigned short fn_415a46(short key);
void unloadWave(short key);
void fn_415a72(short key);
short playWaveOn(short key, short channel);
short fn_415aa3(short key, short channel);
void stopWaves(unsigned short id);
void fn_415ad4(unsigned short id);
short isWavePlaying(unsigned short id);
short playWave(short key, short channel, short eventType, short discard);
short loadAndPlayWave(short key, short channel, short eventType, short discard);
short waitForWave(unsigned short id, short eventType, short discard);
short fn_415b76(unsigned short id, short eventType, short discard);
short fn_415b96(unsigned short id, short stop);
short fn_415bb1(char value);
short waitForWaveValue(char value, short eventType, short discard);
short fn_415be5(char value, short eventType, short discard);
short runWipe(basePort *to, basePort *from, const ShortRect *toRect, const ShortRect *fromRect,
              short duration, unsigned short direction, short eventType, short discard);
void startWipe(basePort *to, basePort *from, const ShortRect *toRect, const ShortRect *fromRect,
               short duration, unsigned short direction);
short stepWipe();
void drawWipe(short upTo);
short wipeScreen(const ShortRect *rect, short duration, unsigned short direction, short eventType,
                 short discard);
void startScreenWipe(const ShortRect *rect, short duration, unsigned short direction);
void noteCheatKey(unsigned short key);
int isCheat(long hash, long code);
void getShapeSize(ResourceList *list, unsigned short index, short *height, short *width);
short randomBetween(short low, short high);
void reduceFraction(short *numerator, short *denominator, unsigned short largest);
short runBlinds(basePort *to, basePort *from, const ShortRect *toRect, const ShortRect *fromRect,
                short duration, short stripe, short eventType, short discard);
void startBlinds(basePort *to, basePort *from, const ShortRect *toRect, const ShortRect *fromRect,
                 short duration, short stripe);
short stepBlinds();
void drawBlinds(short upTo);
short blindsScreen(const ShortRect *rect, short duration, short stripe, short eventType,
                   short discard);
void startScreenBlinds(const ShortRect *rect, short duration, short stripe);
void preloadResource(long type, short id, long *kind);
void *preloadDone(long event, long id, void *kind);
void freePreloaded(short release);
void copyRegion(basePort *to, basePort *from, short region);
void showRegion(short region);
void getDateTime(short *year, char *month, char *day, char *hour, char *minute);
void drawOutlinedText(unsigned short outline, unsigned short color, ShortRect rect,
                      unsigned short flags, const char *text);
void nudgeRect(ShortRect *rect, short direction);
void fn_416754();
long fn_417906(long);
void leaveCamp();
void campIdle();
void fn_463359();
void fn_46560b();
void fn_4624fc();
void fn_46356c();
void fn_463e9e(short);
void fn_43af6b();
void drawCampButtons(short button, short pressed, short group, short show); /* 0x41790f */
void drawCampButtons1(View *);
long fn_4640d1(short);
void fn_4640a6(long);
void drawCampButtons2(View *);
void fn_417aec(View *, short region);
short findCampSlot(short start, ShortRect rect, short occupied);
void scrollCamp(View *view, long);
void drawCamp(View *);
void noteCampSlot(short slot);
void initSnoid(Snoid *snoid); /* 0x45bf41 */
void fn_45b06a(Snoid *snoid, short);
void fn_45ab97(Snoid *snoid, short);
void fn_4571d8(Snoid *snoid);
short campSlotsUsed();
void insertCampRow();
void compactCamp();
void updateCampScroll(short stop);
void fn_4184b7();
short returnToCamp();
View *findView(short id); /* 0x4640f8 */
void fn_4666b7(short sound, short);
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
short localMemError();
void *localAlloc(unsigned short size); /* 0x46d95c */
void localFree(void *block); /* 0x46d998 */
void *localReAlloc(void *block, unsigned short size); /* 0x46d9cf */
short isMohawkThread(unsigned long thread); /* 0x46e00b: whether a thread runs the Mohawk OS */
BOOL CALLBACK findManagerWindow(HWND window, short *version); /* 0x46e02f */
void setLocalMemError(short value);
int isAlignedPointer(void *pointer);
long atomicDecrement(long *value);
void *atomicExchange(void **target, void *value);
long atomicIncrement(long *value);
short debugBreak(short value);
void enterAllLocks(); /* 0x46dc45 */
void leaveAllLocks(); /* 0x46dd02 */
short osSetActive(short active); /* 0x46da64 */
long newTimer(void (*proc)(long timer, long data), long data, long interval); /* 0x46daca */
void deleteTimer(long timer); /* 0x46dbc2 */
void runTimers(unsigned long now, short all); /* 0x46dc64 */
LRESULT CALLBACK timerHook(int code, WPARAM wParam, LPARAM lParam); /* 0x46dd40 */
void osShutdown(); /* 0x46e081 */
void setTimerInterval(long timer, long interval); /* 0x46e0f8 */
LRESULT CALLBACK managerWindowProc(HWND window, UINT message, WPARAM wParam,
                                   LPARAM lParam); /* 0x46e164 */
void osIdle(); /* 0x46e1b2 */
unsigned short bcdVersion(unsigned short version); /* 0x46e21a */
HINSTANCE engineInstanceHandle();
unsigned long appThreadId();
HWND appWindowHandle();
unsigned long currentTimeMs();
short osLockMemory(void *address, unsigned long size);
short osUnlockMemory(void *address, unsigned long size);
short isAppActive();
short osVersion(); /* 0x46dff7: 0x500 once running */
ActivateHook setActivateHook(ActivateHook hook);
HINSTANCE fn_46e0ec(long);
short setOsError(short error);
long timerHandle(OsTimer *timer); /* 0x46e1f8 */
OsTimer *timerOf(long timer); /* 0x46e202: 0 if it isn't one */
char __cdecl lowByte(char value); /* 0x46e28e */
int __cdecl highByte(unsigned short value);
short threadError(); /* the OS layer's last error */
void resetEventCall(void *event); /* 0x46e842 */
void setEventCall(void *event); /* 0x46eac8 */
long __cdecl syncHandle(sync *object); /* 0x46f43a */
void fn_46f74f(thread *to); /* 0x46f74f */
void recordReturn(Context *context, unsigned short depth); /* 0x46f771 */
short setThreadError(short error); /* 0x46f78e */

#endif
