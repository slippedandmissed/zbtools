/*
 * lilly (0x424274-0x42f920): 'Lilly.MHK'; 46 KB, so probably several modules
 */

#include "zoombinis.h"

/* @zoombi32 0x00427e1a */
void fn_427e1a(Flagged *object, short code)
{
    switch (code) {
    case 10:
        object->flags |= 0x20000L;
    }
}

/* @zoombi32 0x0042c10b */
void fn_42c10b(long)
{
}

/* @zoombi32 0x0042c112 */
void fn_42c112(long, long)
{
}

/* @zoombi32 0x0042c6cb */
void fn_42c6cb(short value)
{
    g_4af350 = value;
}

/* @zoombi32 0x0042e693 */
short fn_42e693()
{
    return g_4af35a;
}

/* @zoombi32 0x0042e69a */
void fn_42e69a()
{
    g_4b83e4[g_4af35a] = 0;
    g_4af35a = 0;
}

/* @zoombi32 0x0042d64c */
void freeResourcePair(long *resources)
{
    fn_46c602(resources);
    fn_46c602(resources + 1);
}

/* Not exact: the original tests `event` with 16-bit operations. */
/* @zoombi32 0x0042b258 */
short fn_42b258(short event)
{
    switch (event) {
    case 367:
        fn_466b93();
        return 1;
    }
    return 0;
}

/* Unlocks and releases a resource held locked. */
/* @zoombi32 0x0042d996 */
void freeLockedResource(long *resource, short *handle)
{
    if (*handle) {
        unlockHandle(*handle);
        fn_46c602(resource);
        *handle = 0;
        *resource = 0;
    }
}

/* The sound a Zoombini makes (by its feet) for `which` (0 or 1). */
/* @zoombi32 0x004278d9 */
short fn_4278d9(View *view, short which)
{
    short sound = 0;
    Snoid *snoid = viewSnoid(view);
    short feet = snoid->features[3];

    switch (which) {
    case 0:
        sound = feet + 12999;
        break;
    case 1:
        sound = feet + 12999;
        break;
    }
    return sound;
}

/* @zoombi32 0x0042c0d9 */
void fn_42c0d9(View *view, short region)
{
    ShortRect unused;

    if (!g_4b9684 && view->reset) {
        view->reset = 0;
        unionRgnRect(region, &g_4a1dfc);
    }
}

/* @zoombi32 0x004275dc */
void fn_4275dc(short n)
{
    short which = n % 5;

    startView(g_4ac310[n], which + 9002, 0, 0);
}

/* @zoombi32 0x00427610 */
void fn_427610(short n)
{
    short which = n % 5;

    startView(g_4ac40a[n], which + 9007, 0, 0);
}

/* Loads two 'REGS' tables (`id` and the next) into *first and *second. */
/* @zoombi32 0x0042d616 */
void loadTablePair(long *resources, short id, short **first, short **second)
{
    resources[0] = 0;
    resources[1] = 0;
    *first = loadShortTable(id, resources);
    *second = loadShortTable(++id, ++resources);
}

/* @zoombi32 0x004280fd */
void fn_4280fd()
{
    deleteView(g_4ac0ba);
    g_4ac0ba = 0;
    if (g_4b87fe && lastViewSound) {
        stopSounds(lastViewSound, RESOURCE_TYPE(0, 'S', 'N', 'D'));
        lastViewSound = 0;
    }
}

/* @zoombi32 0x0042afbe */
void fn_42afbe(View *view)
{
    short *cel = (short *)view->body.cels;

    if (*cel) {
        *cel += cel[10];
        short image = *cel;

        cel++;
        *cel -= g_4ac950[image];
        cel++;
        *cel -= g_4ac954[image];
    }
}

/* Unlocks and releases `count` resources held locked. */
/* @zoombi32 0x0042bd19 */
void freeLockedResources(long *resources, short *handles, short count)
{
    for (short i = 0; i < count; i++)
        if (handles[i]) {
            unlockHandle(handles[i]);
            fn_46c602(&resources[i]);
            handles[i] = 0;
            resources[i] = 0;
        }
}

/* @zoombi32 0x0042f49d */
void fn_42f49d(View *view, short event)
{
    short *body = (short *)&view->body;
    View *other;

    switch (event) {
    default:
        break;
    case 26:
        other = findView(body[14]);
        if (other) {
            setViewScript(other, 10000, 1);
            other->placed = fn_42f192;
            other->notify = fn_42f49d;
        }
        break;
    case 20:
        g_4acd4c[g_4acd74] = view->id;
        g_4acd74++;
        break;
    case 25:
        break;
    }
}

/* Sets column n % 5 of g_4ac1a8 to `b` and row n / 5 of g_4ac1da to `a`. */
/* Not exact: register allocation (the original keeps n in esi and n / 5 in edi). */
/* @zoombi32 0x00426a92 */
void fn_426a92(short b, short a, short n)
{
    short row;
    short first;
    short column;

    row = n / 5;
    first = n - n % 5;
    column = n - row * 5;
    for (short i = 0; i < 5; i++) {
        g_4ac1a8[column + i * 5] = b;
        g_4ac1da[first + i] = a;
    }
}

/* @zoombi32 0x0042756d */
void fn_42756d(short a, short b, short c, short n)
{
    short column = n % 5;
    short layer = n / 25;
    short row = n % 25;

    row /= 5;
    g_4ac1a8[row] = a;
    g_4ac1da[layer] = b;
    g_4ac20c[column] = c;
}

/* @zoombi32 0x00428140 */
void fn_428140()
{
    short ids[15] = {0, 1, 2, 3, 4, 7, 10, 11, 12, 13, 14, 20, 22, 23, 24};

    if (!g_4ac0e4) {
        for (short i = 0; i < 15; i++)
            startView(g_4abfc0[ids[i]], ids[i] + 6013, 0, 0);
        g_4ac0d6 = 3;
        queueViewSound(7047, 0);
    }
}

/* @zoombi32 0x0042b7e7 */
void fn_42b7e7(short level)
{
    g_4ac91e = 0;
    switch (level) {
    case 1:
        g_4ac920 = 0;
        g_4af0f8 = 0;
        break;
    case 2:
        g_4ac920 = 4;
        g_4af0f8 = 0;
        break;
    case 3:
        g_4ac920 = 5;
        g_4af0f8 = 2;
        break;
    case 4:
        g_4ac920 = 6;
        g_4af0f8 = 3;
        break;
    }
}

/* @zoombi32 0x004249e1 */
void fn_4249e1(View *view, short region)
{
    if (g_4abec2) {
        if (!g_4a1aac) {
            g_4a1aac = 1;
            unionRgnRect(region, &g_4a170c);
        }
    } else if (g_4a1aac) {
        g_4a1aac = 0;
        unionRgnRect(region, &g_4a170c);
    }
    if (!g_4a1aae) {
        g_4a1aae = 1;
        unionRgnRect(region, &g_4a16e8);
    }
}

/* @zoombi32 0x00428c45 */
void fn_428c45(View *view, short region)
{
    if (g_4af36a) {
        if (!g_4a1d6c) {
            g_4a1d6c = 1;
            unionRgnRect(region, &g_4a1b70);
        }
    } else if (g_4a1d6c) {
        g_4a1d6c = 0;
        unionRgnRect(region, &g_4a1b70);
    }
    if (!g_4a1d6e) {
        g_4a1d6e = 1;
        unionRgnRect(region, &g_4a1b4c);
    }
}

/* @zoombi32 0x0042b003 */
void fn_42b003(View *view, short event)
{
    View *self = view;

    switch (event) {
    case 1:
        fn_42c6cb(5);
        g_4b755a--;
        if (g_4b755a < 0)
            g_4b755a = 0;
        break;
    case 2:
        g_4ace72[g_4ace9a] = self->id;
        g_4ace9a++;
        break;
    case 3:
        g_4ace9c[g_4acec4] = self->id;
        g_4acec4++;
        break;
    case 0:
    case 4:
    case 5:
        break;
    }
}

/* Loads a 'REGS' table (swapping its words) and keeps it locked. */
/* @zoombi32 0x0042d667 */
void loadLockedTable(long *resource, short *handle, short id, short **locked)
{
    short *at;

    *resource = 0;
    fn_46c4fe(resource, RESOURCE_TYPE('R', 'E', 'G', 'S'), id, 0, 1);
    *handle = fn_46beac(*resource);
    *locked = (short *)lockHandle(*handle);
    at = (short *)handleData(*handle);
    for (unsigned long size = handleSize(*handle); size; size -= 2) {
        *at = swapShort(*at);
        at++;
    }
}
