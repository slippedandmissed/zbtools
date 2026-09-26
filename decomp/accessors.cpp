/*
 * Functions that just return or set a global. Named after their address until
 * their purpose is known.
 */

extern long g_4b9d00;
extern long g_4b9d04;
extern long g_4b9d08;
extern short g_4af35a;
extern short g_4b2d38;
extern short g_4b99d4;
extern short g_4b9cf0;
extern short g_4b9cf8;
extern short g_4b9d4c;
extern short g_4ab49e;
extern short g_4a7b94;
extern long g_4aa4c4;
extern long g_4a07ac;
extern long g_4a07b0;
extern long g_4a07b4;
extern long g_4a07c4;
extern long g_4a07e8;
extern long g_4a07ec;
extern long g_4a4a14;
extern long g_4a4a00;
extern long g_4b7b68;
extern short g_4af350;

/* @zoombi32 0x0046dd21 */
long fn_46dd21()
{
    return g_4b9d00;
}

/* @zoombi32 0x0046dd27 */
long fn_46dd27()
{
    return g_4b9d04;
}

/* @zoombi32 0x0046dd2d */
long fn_46dd2d()
{
    return g_4b9d08;
}

/* @zoombi32 0x0042e693 */
short fn_42e693()
{
    return g_4af35a;
}

/* @zoombi32 0x00456bf6 */
short fn_456bf6()
{
    return g_4b2d38;
}

/* @zoombi32 0x0046bee2 */
short fn_46bee2()
{
    return g_4b99d4;
}

/* @zoombi32 0x0046d9c8 */
short fn_46d9c8()
{
    return g_4b9cf0;
}

/* @zoombi32 0x0046dff0 */
short fn_46dff0()
{
    return g_4b9cf8;
}

/* @zoombi32 0x0046e5ed */
short fn_46e5ed()
{
    return g_4b9d4c;
}

/* @zoombi32 0x00415811 */
void fn_415811()
{
    g_4ab49e = 1;
}

/* @zoombi32 0x00465175 */
void fn_465175()
{
    g_4a7b94 = 1;
}

/* @zoombi32 0x00413bcf */
void fn_413bcf(long value)
{
    g_4aa4c4 = value;
}

/* @zoombi32 0x004153b0 */
void fn_4153b0(long value)
{
    g_4a07ac = value;
}

/* @zoombi32 0x004153bf */
void fn_4153bf(long value)
{
    g_4a07b0 = value;
}

/* @zoombi32 0x004153ce */
void fn_4153ce(long value)
{
    g_4a07b4 = value;
}

/* @zoombi32 0x00415604 */
void fn_415604(long value)
{
    g_4a07c4 = value;
}

/* @zoombi32 0x00415a11 */
void fn_415a11(long value)
{
    g_4a07e8 = value;
}

/* @zoombi32 0x00415a20 */
void fn_415a20(long value)
{
    g_4a07ec = value;
}

/* @zoombi32 0x00456a2f */
void fn_456a2f(long value)
{
    g_4a4a14 = value;
}

/* @zoombi32 0x00456a55 */
void fn_456a55(long value)
{
    g_4a4a00 = value;
}

/* @zoombi32 0x0045bfc0 */
void fn_45bfc0(long value)
{
    g_4b7b68 = value;
}

/* @zoombi32 0x0042c6cb */
void fn_42c6cb(short value)
{
    g_4af350 = value;
}

/* @zoombi32 0x0041581b */
void fn_41581b(short flag)
{
    if (flag)
        fn_415811();
}

extern short g_4a4ce6;

/* @zoombi32 0x0045b39a */
void fn_45b39a(short value)
{
    g_4a4ce6 = value & 3;
}

extern long g_4a4a18;
extern long g_4a4a1c;

/* @zoombi32 0x00456a3e */
void fn_456a3e(long first, long second)
{
    g_4a4a18 = first;
    g_4a4a1c = second;
}

extern short g_4b7b38;
extern short g_4b7b3a;

/* @zoombi32 0x00457fbb */
short fn_457fbb()
{
    if (g_4b7b3a)
        return g_4b7b38 + 1;
    return 0;
}

/* Sets g_4b99d4, returning its old value. */
/* @zoombi32 0x0046bee9 */
short fn_46bee9(short value)
{
    short old = g_4b99d4;
    g_4b99d4 = value;
    return old;
}

extern short g_4b0d52;
extern short g_4b0d54;

/* @zoombi32 0x0046b747 */
void fn_46b747(long, short id)
{
    if (id == 30)
        g_4b0d52 = g_4b0d54;
}

extern short g_4b83e4[];

/* @zoombi32 0x0042e69a */
void fn_42e69a()
{
    g_4b83e4[g_4af35a] = 0;
    g_4af35a = 0;
}

/* Something with flags at +0x20. */
struct Flagged
{
    char unknown0[0x20];
    long flags;
};

/* @zoombi32 0x00427e1a */
void fn_427e1a(Flagged *object, short code)
{
    switch (code) {
    case 10:
        object->flags |= 0x20000L;
    }
}

extern long g_4aa498;

/* Calls callback with g_4aa498 if there is one; returns whether it did. */
/* @zoombi32 0x00412b4d */
short fn_412b4d(void (*callback)(long))
{
    if (!callback)
        return 0;
    callback(g_4aa498);
    return 1;
}

extern short g_4ab49c;

/* Returns whether either flag was set, and clears both. */
/* @zoombi32 0x004157f3 */
short fn_4157f3()
{
    short either = g_4ab49c | g_4ab49e;
    g_4ab49c = g_4ab49e = 0;
    return either;
}

extern short g_4afb90;

/* @zoombi32 0x004334f0 */
void fn_4334f0(long, short value)
{
    if (value == -1 && g_4afb90 < 0)
        g_4afb90 = -g_4afb90;
}

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

/* @zoombi32 0x0042fc89 */
void fn_42fc89(Counters *object)
{
    Triple *counters = &object->counters;
    if (object->mode == 2) {
        counters->a--;
        counters->b++;
        counters->c += 2;
    }
}

/* Engine functions whose conventions aren't known yet: these
   declarations produce the calls the game makes. */
void fn_476622(long handle);
void fn_4812bc(short handle);
void fn_48f660(long handle, long, long);

/* Something with a handle at +6. */
struct HasHandle
{
    char unknown0[6];
    long handle;
};

/* @zoombi32 0x004117a8 */
void fn_4117a8(HasHandle *object)
{
    if (object->handle) {
        fn_476622(object->handle);
        object->handle = 0;
    }
}

/* @zoombi32 0x00414ce7 */
void fn_414ce7(short *handle)
{
    if (*handle) {
        fn_4812bc(*handle);
        *handle = 0;
    }
}

/* @zoombi32 0x0046ca9c */
void fn_46ca9c(long *handle)
{
    if (*handle) {
        fn_48f660(*handle, 0, 0);
        *handle = 0;
    }
}

/* A 22-byte entry with a value at +4. */
struct Entry22
{
    long unknown0;
    long value;
    char unknown8[14];
};

extern Entry22 *g_4ab64c;

/* The index of the last of 625 entries with a value, or 0. */
/* @zoombi32 0x00419f1a */
short fn_419f1a()
{
    for (short i = 0x270; i >= 0; i--)
        if (g_4ab64c[i].value)
            return i;
    return 0;
}
