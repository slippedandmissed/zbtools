/*
 * Declarations shared by the decompiled modules: the game's globals, the types
 * worked out so far, and every decompiled function, so any module can use them.
 * Names follow CLAUDE.md: by address until their purpose is known. Types are
 * inferred from how the code uses them and grow as more of it is decompiled.
 */

#ifndef ZOOMBINIS_H
#define ZOOMBINIS_H

#include <time.h>

/* Types */

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
extern long g_4a07ac;
extern long g_4a07b0;
extern long g_4a07b4;
extern time_t g_4a07b8;
extern long g_4a07c4;
extern long g_4a07e8;
extern long g_4a07ec;
extern short g_4a3e5c; /* set when the game data is found in INSTALLFROMDIR */
extern char installFromDirKey[]; /* @data 0x4a3f06 */
extern char dataDirName[]; /* @data 0x4a3f15 */
extern char installToDirKey[]; /* @data 0x4a3f1b */
extern long g_4a4a00;
extern long g_4a4a14;
extern long g_4a4a18;
extern long g_4a4a1c;
extern char *g_4a4ba0;
extern short g_4a4ce6;
extern char configFileName[]; /* @data 0x4a5149 */
extern short g_4a7b94;
extern long g_4a7f58;
extern Counted *g_4a8dcc;
extern long g_4aa498;
extern long g_4aa4c4;
extern char g_4aa4c9;
extern short g_4aa79a;
extern short g_4aa79c;
extern short g_4ab49c;
extern short g_4ab49e;
extern Entry22 *g_4ab64c;
extern short g_4af350;
extern short g_4af35a;
extern short g_4afb90;
extern short g_4aff9a[];
extern short g_4b0d52;
extern short g_4b0d54;
extern char installDir[256]; /* @data 0x4b1828 */
extern short g_4b2d38;
extern short g_4b7b38;
extern short g_4b7b3a;
extern long g_4b7b68;
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

/* Engine functions whose calling conventions aren't known yet: these
   declarations produce the calls the game makes. */

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

void fn_476622(long handle);
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
void fn_4153b0(long value);
void fn_4153bf(long value);
void fn_4153ce(long value);
void fn_415514();
void fn_415604(long value);
short fn_4157f3();
void fn_415811();
void fn_41581b(short flag);
unsigned short toLowerAscii(unsigned short c);
unsigned short toUpperAscii(unsigned short c);
void fn_415a11(long value);
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
void fn_446962(long, long);
void findGameData();
short preferFirstFile(const char *first, const char *fallback);
long fn_455013(long, long);
int isMousePresent();
void freeAndClear(void **block);
char *intToDecimal(int value, char *buffer);
char *unsignedToDecimal(unsigned long value, char *buffer);
void fn_455e26(long);
void fn_455e2d(long);
long fn_455e85(long, long);
void fn_456a2f(long value);
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
