/*
 * lilly (0x424274-0x42f920): 'Lilly.MHK'; 46 KB, so probably several modules
 */

#include <stdio.h>
#include <stdlib.h>

#include "zoombinis.h"

/* A lilly actor's view body (flag 2, a large body). Partly known. */
struct LillyActor
{
    ViewBody body;
    short unknownBc;
    short unknownBe;
    short unknownC0;
    char unknownC2;
    char column; /* +0xc3 */
    char row; /* +0xc4 */
    char unknownC5;
    char unknownC6;
    char unknownC7;
    char unknownC8;
    short startX; /* +0xc9 */
    short startY;
    short targetX; /* +0xcd */
    short targetY;
    short stepX; /* +0xd1 */
    short stepY;
    char unknownD5;
    char unknownD6;
    short unknownD7;
    short unknownD9; /* +0xd9 */
    char unknownDb;
    short unknownDc;
    char unknownDe;
    char unknownDf;
    char unknownE0; /* +0xe0: added to its second part's image */
    char unknownE1;
    char unknownE2;
    char unknownE3; /* +0xe3: added to its third part's image */
    short unknownE4;
    char unknownE6[12];
    short grid[12][13]; /* +0xf2 */
};

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

/* @zoombi32 0x0042b258 */
short fn_42b258(unsigned short event)
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
            unionRgnRect(region, &g_4a16c4[2].rect);
        }
    } else if (g_4a1aac) {
        g_4a1aac = 0;
        unionRgnRect(region, &g_4a16c4[2].rect);
    }
    if (!g_4a1aae) {
        g_4a1aae = 1;
        unionRgnRect(region, &g_4a16c4[1].rect);
    }
}

/* @zoombi32 0x00428c45 */
void fn_428c45(View *view, short region)
{
    if (g_4af36a) {
        if (!g_4a1d6c) {
            g_4a1d6c = 1;
            unionRgnRect(region, &g_4a1b28[2].rect);
        }
    } else if (g_4a1d6c) {
        g_4a1d6c = 0;
        unionRgnRect(region, &g_4a1b28[2].rect);
    }
    if (!g_4a1d6e) {
        g_4a1d6e = 1;
        unionRgnRect(region, &g_4a1b28[1].rect);
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

/* @zoombi32 0x0042a7b6 */
void fn_42a7b6(View *view, short event)
{
    ViewBody *body = &view->body;
    LillyActor *actor = (LillyActor *)&view->body;

    switch (event) {
    case 60:
        actor->body.x = body->cels[0].x;
        actor->body.y = body->cels[0].y;
        actor->body.unknownAa = body->cels[0].x;
        actor->body.unknownAc = body->cels[0].y;
        g_4acff4[actor->row][actor->column + 1].attributes[0] = 0;
        g_4acdf4[g_4ace1c] = view->id;
        g_4ace1c++;
        g_4acfee = 0;
        break;
    }
}

/* @zoombi32 0x0042adb5 */
void fn_42adb5(View *view, short event)
{
    ViewBody *body = &view->body;
    LillyActor *actor = (LillyActor *)&view->body;

    switch (event) {
    case 49:
        actor->body.x = body->cels[0].x;
        actor->body.y = body->cels[0].y;
        actor->body.unknownAa = body->cels[0].x;
        actor->body.unknownAc = body->cels[0].y;
        actor->unknownC2 = 0;
        view->flags = 0x980002;
        g_4acfe8 = view->id;
        break;
    }
}

/* @zoombi32 0x00427644 */
void fn_427644(short id)
{
    View *view = findView(id);

    if (view) {
        short script;

        if (g_4ac0d8 < 3)
            script = g_4ac0ec + 13000;
        else
            script = g_4ac0ec % 5 + 13025;
        startSnoidScript(viewSnoid(view), script, 0, 0);
        view->notifyEnd = 1;
        view->notify = fn_4276d0;
        view->interval = 3;
        g_4ac0d0 = groupViews(g_4abec6[g_4ac0ec], view->id, 0, 0, 0, 0);
    }
}

/* Shows the view `id` at the board's square (row, column). */
/* @zoombi32 0x0042e4b6 */
void fn_42e4b6(short id, short row, short column)
{
    View *view = findView(id);

    if (view) {
        view->body.running = 1;
        view->body.bounds.left = g_4acff4[row][column].rect.left;
        view->body.bounds.top = g_4acff4[row][column].rect.top;
        view->body.bounds.right = view->body.bounds.left + 20;
        view->body.bounds.bottom = view->body.bounds.top + 15;
    }
}

/* Not exact: the original keeps `region` in esi. */
/* @zoombi32 0x0042c9aa */
void fn_42c9aa(View *view, short region)
{
    ShortRect rect;

    if (!g_4b9684) {
        if (view->reset) {
            view->reset = 0;
            view->nextUpdate = 0;
            view->body.running = 0;
            unionRgnRect(region, &g_4a1e32);
        }
        if (view->body.running) {
            rect.left = view->body.bounds.left - 17;
            rect.top = view->body.bounds.top - 14;
            rect.right = view->body.bounds.right;
            rect.bottom = view->body.bounds.bottom;
            unionRgnRect(region, &rect);
            view->changed = 1;
        }
    }
}

/* Draws button `which` (1, 2), lit or not, and shows it if asked. */
/* @zoombi32 0x0042492b */
void fn_42492b(short which, short lit, short show)
{
    short image = 0;

    switch (which) {
    case 1:
        image = 5;
        break;
    case 2:
        image = 2;
        if (!g_4abec2) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        drawImageData((unsigned short *)((char *)g_4a1a18 + g_4a1a18->offsets[image]), g_4a16c4[which].rect.left,
                      g_4a16c4[which].rect.top, 8);
        if (show)
            showRect(&g_4a16c4[which].rect);
    }
}

/* Draws button `which` (1, 2) of the other set, lit or not, and shows it if asked. */
/* @zoombi32 0x00428b8f */
void fn_428b8f(short which, short lit, short show)
{
    short image = 0;

    switch (which) {
    case 1:
        image = 5;
        break;
    case 2:
        image = 2;
        if (!g_4af36a) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        drawImageData((unsigned short *)((char *)g_4a1d68 + g_4a1d68->offsets[image]), g_4a1b28[which].rect.left,
                      g_4a1b28[which].rect.top, 8);
        if (show)
            showRect(&g_4a1b28[which].rect);
    }
}

/* Functional: the original reads each cel's words as it pushes them
   (`drawImageData(image(*cel++), *cel++, *cel++, 8)`, relying on BCC's
   left-to-right evaluation); this reads them first. */
/* @zoombi32-functional 0x00426f38 */
void fn_426f38(View *view)
{
    if (view->body.running) {
        short *cel = (short *)view->body.cels;

        if (g_4ac0d8 == 2) {
            while (*cel) {
                unsigned short *image = (unsigned short *)(g_4ac178->offsets[*cel++] + (char *)g_4ac178);
                short x = *cel++;
                short y = *cel++;

                drawImageData(image, x, y, 8);
            }
        } else if (g_4ac0d8 == 3) {
            while (*cel) {
                unsigned short *image = (unsigned short *)(g_4ac17c->offsets[*cel++] + (char *)g_4ac17c);
                short x = *cel++;
                short y = *cel++;

                drawImageData(image, x, y, 8);
            }
        }
    }
}

/* @zoombi32 0x00427e34 */
void fn_427e34()
{
    short i;

    if (!g_4ac0d8) {
        for (i = 4; i < g_4ac0ee; i += 5) {
            startView(g_4abec6[i], i + 6063, 0, 0);
            View *view = findView(g_4abec6[i]);

            view->notify = (ViewNotify)fn_427e1a;
            view->interval = 1;
        }
    } else if (g_4ac0d8 <= 2) {
        for (i = 0; i < g_4ac0ee; i++) {
            startView(g_4abec6[i], i + 6063, 0, 0);
            View *view = findView(g_4abec6[i]);

            view->notify = (ViewNotify)fn_427e1a;
            view->interval = 1;
        }
    }
}

/* @zoombi32 0x0042e6b5 */
void fn_42e6b5()
{
    short count = g_4af0ea;

    for (short i = 0; i < g_4af102; i++) {
        View *view = findView(g_4aed64[i]);

        if (view) {
            LillyActor *actor = (LillyActor *)&view->body;

            if (actor->unknownC2 && actor->unknownD6 == 11) {
                count++;
                short *parts = (short *)&view->body;
                View *rider = findView(parts[13]);

                if (rider)
                    viewSnoid(rider)->unknownF7 = 1;
            }
        }
    }
    if (count < g_4af0e8)
        if (randomBetween(0, 4) > g_4a1b1c - 1 || (*(short *)(g_4a4ba0 + 0x34) & 0xfff) <= 3)
            queueViewSound(randomBetween(20045, 20048), 0);
}

/* Not exact: the original keeps `region` in esi. */
/* @zoombi32 0x0042c306 */
void fn_42c306(View *view, short region)
{
    if (!g_4b9684) {
        if (view->reset) {
            view->body.running = 0;
            view->nextUpdate = 0;
            view->reset = 0;
            unionRgnRect(region, &g_4a1e32);
        }
        if (view->body.running) {
            LillyCell *cell = &g_4acff4[g_4af346][g_4af344];

            g_4af5a8.left = cell->rect.left - 18;
            g_4af5a8.top = cell->rect.top - 15;
            g_4af5a8.right = cell->rect.right - 17;
            g_4af5a8.bottom = cell->rect.bottom - 14;
            unionRgnRect(region, &g_4af5a8);
        }
    }
}

/* Places a lilly actor's cels by their hot spots, its second part's image
   offset by unknownE0. */
/* @zoombi32 0x0042f3ed */
void fn_42f3ed(View *view)
{
    short *cel = (short *)&view->body;
    LillyActor *actor = (LillyActor *)&view->body;
    short image;

    image = *cel++;
    *cel++ -= g_4ac950[image];
    *cel++ -= g_4ac954[image];
    if (*cel > 0) {
        *cel += actor->unknownE0;
        image = *cel++;
        *cel++ -= g_4ac950[image];
        *cel++ -= g_4ac954[image];
        while (*cel) {
            image = *cel++;
            *cel++ -= g_4ac950[image];
            *cel++ -= g_4ac954[image];
        }
    }
}

/* The same, the second part's image offset by unknownE0 - 7. */
/* @zoombi32 0x0042f336 */
void fn_42f336(View *view)
{
    short *cel = (short *)&view->body;
    LillyActor *actor = (LillyActor *)&view->body;
    short image;

    image = *cel++;
    *cel++ -= g_4ac950[image];
    *cel++ -= g_4ac954[image];
    if (*cel > 0) {
        *cel = actor->unknownE0 + *cel - 7;
        image = *cel++;
        *cel++ -= g_4ac950[image];
        *cel++ -= g_4ac954[image];
        while (*cel) {
            image = *cel++;
            *cel++ -= g_4ac950[image];
            *cel++ -= g_4ac954[image];
        }
    }
}

/* The same for three parts, the second and third offset by unknownE0 and
   unknownE3. */
/* @zoombi32 0x0042f192 */
void fn_42f192(View *view)
{
    short *cel = (short *)&view->body;
    LillyActor *actor = (LillyActor *)&view->body;
    short image;

    image = *cel++;
    if (image) {
        *cel++ -= g_4ac950[image];
        *cel++ -= g_4ac954[image];
    }
    if (*cel > 0) {
        *cel += actor->unknownE0;
        image = *cel++;
        *cel++ -= g_4ac950[image];
        *cel++ -= g_4ac954[image];
        if (*cel > 0) {
            *cel += actor->unknownE3;
            image = *cel++;
            *cel++ -= g_4ac950[image];
            *cel++ -= g_4ac954[image];
        }
    }
}

/* @zoombi32 0x0042aaba */
void fn_42aaba(View *view, short event)
{
    ViewBody *body = &view->body;
    LillyActor *actor = (LillyActor *)&view->body;

    switch (event) {
    case 54:
        actor->body.x = body->cels[0].x;
        actor->body.y = body->cels[0].y;
        actor->body.unknownAa = body->cels[0].x;
        actor->body.unknownAc = body->cels[0].y;
        g_4acff4[actor->row][actor->column + 1].attributes[0] = 0;
        g_4acff2 = g_4acff0;
        for (short i = 0; i < 13; i++)
            if (g_4aed64[i] == g_4acff2) {
                for (; g_4aed64[i]; i++)
                    g_4aed64[i] = g_4aed64[i + 1];
                i = 13;
            }
        break;
    }
}

/* Loads `count` scripts ('SCRB' 10000 on), swapping their words. */
/* @zoombi32 0x0042bc61 */
void loadLillyScripts(long *resources, short *handles, short count)
{
    for (short i = 0; i < count; i++) {
        short *at;

        fn_46c4fe(&resources[i], RESOURCE_TYPE('S', 'C', 'R', 'B'), i + 10000L, 0, 1);
        handles[i] = fn_46beac(resources[i]);
        fn_48ea00(handles[i]);
        at = (short *)handleData(handles[i]);
        for (unsigned long size = handleSize(handles[i]); size; size -= 2) {
            *at = swapShort(*at);
            at++;
        }
    }
}

/* Darkens the palette's colours 10-245 (to 88-92% by the level). */
/* @zoombi32 0x00426c33 */
void darkenPalette()
{
    PALETTEENTRY colors[256];
    short percent = 92;

    if (!g_4ac0d8)
        percent = 88;
    else if (g_4ac0d8 == 2)
        percent = 90;
    getColors(&colors[10], 10, 236);
    for (short i = 10; i < 246; i++) {
        colors[i].peRed = colors[i].peRed * percent / 100;
        colors[i].peGreen = colors[i].peGreen * percent / 100;
        colors[i].peBlue = colors[i].peBlue * percent / 100;
    }
    fadePalette(colors, 10, 236, 0, 0, 0);
}

/* @zoombi32 0x0042a6fa */
void fn_42a6fa(View *view, short event)
{
    ViewBody *body = &view->body;
    LillyActor *actor = (LillyActor *)&view->body;

    switch (event) {
    case 44:
        actor->body.x = g_4ac950[body->cels[0].image] + body->cels[0].x;
        actor->body.y = g_4ac954[body->cels[0].image] + body->cels[0].y;
        actor->body.unknownAa = g_4ac950[body->cels[0].image] + body->cels[0].x;
        actor->body.unknownAc = g_4ac954[body->cels[0].image] + body->cels[0].y;
        g_4acff4[actor->row][actor->column + 1].attributes[0] = 0;
        g_4ace1e[g_4ace46] = view->id;
        g_4ace46++;
        break;
    }
}

/* Counts how many different values of each feature the chosen Zoombinis have. */
/* @zoombi32 0x00426cef */
void countFeatureValues()
{
    short counts[4][6];

    g_4ac106[0] = 0;
    g_4ac106[1] = 0;
    g_4ac106[2] = 0;
    g_4ac106[3] = 0;
    g_4ac508 = listChosenSnoids();
    g_4ac0e8 = g_4ac508->count;
    fillMemory(counts, 0, sizeof counts);
    for (short i = 0; i < g_4ac0e8; i++)
        for (short j = 0; j < 4; j++)
            counts[j][g_4ac508->features[i][j]]++;
    for (short feature = 0; feature < 4; feature++)
        for (short value = 1; value < 6; value++)
            if (counts[feature][value])
                g_4ac106[feature]++;
}

/* Draws `number` in a box at `rect`. */
/* @zoombi32 0x0042b708 */
void drawNumberBox(ShortRect rect, short number)
{
    Color saved;
    char text[8];

    saved = setForeColor(Color(0xb));
    fillPortRect(Rect(rect), Color(0xe), 0);
    frameRect(Rect(rect));
    itoa(number, text, 10);
    drawText(Rect(rect), 0x22, text, 0xffff);
    setForeColor(saved);
    showRect(&rect);
}

/* Places a lilly actor's cels by their hot spots: its parts showing image
   0x110 are offset by unknownE0, and all but the first are hidden while
   its ninth cel's x is set. */
/* @zoombi32 0x0042f24c */
void fn_42f24c(View *view)
{
    short *cel = (short *)&view->body;
    LillyActor *actor = (LillyActor *)&view->body;
    short shown = !cel[25];
    short image;

    image = *cel++;
    if (image) {
        *cel++ -= g_4ac950[image];
        *cel++ -= g_4ac954[image];
    }
    while (*cel) {
        if (*cel == 0x110 && shown) {
            *cel += actor->unknownE0;
            image = *cel++;
            *cel++ -= g_4ac950[image];
            *cel++ -= g_4ac954[image];
        } else if (!shown) {
            *cel++ = 0;
            cel++;
            cel++;
        } else {
            image = *cel++;
            *cel++ -= g_4ac950[image];
            *cel++ -= g_4ac954[image];
        }
    }
}

/* @zoombi32 0x0042a077 */
void fn_42a077(View *view, short event)
{
    ViewBody *body = &view->body;
    LillyActor *actor = (LillyActor *)&view->body;

    switch (event) {
    case 70:
        actor->body.x = body->cels[0].x;
        actor->body.y = body->cels[0].y;
        actor->body.unknownAa = body->cels[0].x;
        actor->body.unknownAc = body->cels[0].y;
        g_4ac9e6[g_4acb06] = view->id;
        g_4acb06++;
        break;
    case 80:
        g_4acff4[actor->row][actor->column].attributes[0] = 0;
        g_4aed80[g_4af0e0] = view->id;
        g_4af0e0++;
        for (short i = 0; i < g_4af0e6; i++)
            if (g_4aeea0[i] == view->id) {
                for (; g_4aeea0[i]; i++)
                    g_4aeea0[i] = g_4aeea0[i + 1];
                g_4af0e6--;
            }
        break;
    }
}

/* Shows a view id in a box (a debugging aid). */
/* Not exact: the frame's layout (the original puts `text` above `saved`
   and the temporaries; BCC 4.5 puts this array below them). */
/* @zoombi32 0x00428009 */
void drawIdBox(short id)
{
    ShortRect rect = g_4a1ae6;
    char text[12];

    sprintf(text, "id=%u", id);
    Color saved;

    saved = setForeColor(Color(0xb));
    fillPortRect(Rect(rect), Color(0xe), 0);
    frameRect(Rect(rect));
    drawText(Rect(rect), 0x22, text, 0xffff);
    setForeColor(saved);
    showRect(&rect);
}

/* Mirrors a 12 by 12 grid left to right (`how` 0) or top to bottom (1). */
/* @zoombi32 0x0042d875 */
void mirrorGrid(short (*grid)[12], short how)
{
    short copy[12][12];
    short changed;
    short row;
    short column;

    for (row = 0; row < 12; row++)
        for (column = 0; column < 12; column++)
            copy[row][column] = 0;
    changed = 0;
    if (!how) {
        short last = 11;

        changed = 1;
        for (row = 0; row < 12; row++)
            for (column = 0; column < 12; column++)
                copy[row][last - column] = grid[row][column];
    } else if (how == 1) {
        short last = 11;

        changed = 1;
        for (column = 0; column < 12; column++)
            for (row = 0; row < 12; row++)
                copy[last - row][column] = grid[row][column];
    }
    if (changed)
        for (row = 0; row < 12; row++)
            for (column = 0; column < 12; column++)
                grid[row][column] = copy[row][column];
}

/* Moves the Zoombinis standing near x (on the bank, y 400-440) out of the
   way, to either side at random. */
/* Not exact: the original keeps viewY in edx; this spills it to the stack. */
/* @zoombi32 0x00427edc */
void clearWay(short x)
{
    for (short i = 0; i < g_4ac0e8; i++) {
        View *view = findView(partyViews[i]);

        if (view && view->body.running) {
            short viewX = view->body.x;
            short viewY = view->body.y;

            if (viewY >= 400 && viewY <= 440 && viewX >= x - 20 && viewX <= x + 20) {
                Point target;

                target.y = 440;
                if (randomUpTo(1)) {
                    target.x = randomUpTo(3) * 30 + x + 50;
                    if (target.x > 520)
                        target.x = x - 50 - randomUpTo(3) * 30;
                } else {
                    target.x = x - 50 - randomUpTo(3) * 30;
                    if (target.x < 15)
                        target.x = randomUpTo(3) * 30 + x + 50;
                }
                setSnoidAction(viewSnoid(view), 7, 0);
                *(Point *)&viewSnoid(view)->targetX = target;
            }
        }
    }
}

/* @zoombi32 0x00426aff */
short fn_426aff(short a, short b, short n)
{
    short i;

    if (g_4abec4)
        return 1;
    if (!g_4ac1a8[n] && !g_4ac1da[n]) {
        for (i = 0; i < g_4ac0ee; i++) {
            if (b == g_4ac1da[i])
                return 0;
            if (a == g_4ac1a8[i])
                return 0;
        }
        return 1;
    }
    if (a == g_4ac1a8[n] && b == g_4ac1da[n])
        return 1;
    if (g_4ac1a8[n] && a != g_4ac1a8[n])
        return 0;
    if (g_4ac1da[n] && b != g_4ac1da[n])
        return 0;
    if (a == g_4ac1a8[n] && !g_4ac1da[n]) {
        for (i = 0; i < g_4ac0ee; i++)
            if (b == g_4ac1da[i])
                return 0;
        return 1;
    }
    if (b == g_4ac1da[n] && !g_4ac1a8[n]) {
        for (i = 0; i < g_4ac0ee; i++)
            if (a == g_4ac1a8[i])
                return 0;
        return 1;
    }
    return 0;
}

/* Places the view `id` on the board's square (row, column), by the
   square's image's hot spot. */
/* @zoombi32 0x0042e542 */
void fn_42e542(short id, short row, short column)
{
    View *view = findView(id);

    if (view) {
        view->body.running = 0;
        view->body.bounds.left = g_4acff4[row][column].rect.left - g_4ac948[g_4acff4[row][column].attributes[2] + 1];
        view->body.bounds.top = g_4acff4[row][column].rect.top - g_4ac94c[g_4acff4[row][column].attributes[2] + 1];
        view->body.bounds.right = g_4acff4[row][column].rect.right - g_4ac948[g_4acff4[row][column].attributes[2] + 1];
        view->body.bounds.bottom = g_4acff4[row][column].rect.bottom - g_4ac94c[g_4acff4[row][column].attributes[2] + 1];
        unionRgnRect(removedRgn, &view->body.bounds);
    }
}

/* Draws the board's square (row, column): its image (offset by `offset`)
   and its overlay, by their hot spots. */
/* @zoombi32 0x0042c3b6 */
void fn_42c3b6(short row, short column, char offset)
{
    LillyCell *cell = &g_4acff4[row][column];
    short image = g_4a1e20[g_4acff4[row][column].attributes[2]] + offset;
    unsigned short *data;
    short x;
    short y;

    if (image > 0 && image < 36) {
        data = (unsigned short *)((char *)g_4af5a0 + g_4af5a0->offsets[image]);
        x = cell->rect.left - g_4ac948[g_4acff4[row][column].attributes[2] + 1];
        y = cell->rect.top - g_4ac94c[g_4acff4[row][column].attributes[2] + 1];
        drawImageData(data, x, y, 8);
    }
    image = g_4acff4[row][column].attributes[4];
    if (image > 0 && image < 36) {
        data = (unsigned short *)((char *)g_4af5a0 + g_4af5a0->offsets[image]);
        x = cell->rect.left - g_4ac948[g_4acff4[row][column].attributes[4]];
        y = cell->rect.top - g_4ac94c[g_4acff4[row][column].attributes[4]];
        drawImageData(data, x, y, 8);
    }
}

/* Moves a lilly actor down a row if it can: the script to run next, or 0. */
/* Not exact: register allocation (the original keeps the view in edx and
   the actor in eax). */
/* @zoombi32 0x0042f7a5 */
short fn_42f7a5(View *view)
{
    short *body = (short *)&view->body;
    LillyActor *actor = (LillyActor *)&view->body;
    short blocked;
    char row;
    char column;

    if (body[26]) {
        g_4acec6[g_4acfe6] = view->id;
        g_4acfe6++;
        body[25] = 0;
        body[26] = 0;
        return 0;
    }
    row = actor->row;
    column = actor->column;
    blocked = 0;
    row++;
    if (row > 11) {
        row = 11;
        blocked = 1;
    }
    if (!blocked) {
        if (!g_4acff4[row][column].attributes[0]) {
            actor->unknownDe = g_4aece8;
            actor->unknownDf = g_4acff4[row][column].attributes[actor->unknownDe];
            actor->unknownE0 = g_4a1b1e[g_4a1b38[actor->unknownDe]] + actor->unknownDf;
            actor->grid[row][column] = 1;
            body[26] = 1;
        } else if (g_4aebae[row][column] != 1 && !g_4aebae[row][column]) {
            return 0;
        }
    }
    if (blocked)
        return actor->unknownD9 = 10069;
    g_4acff4[row][column].attributes[0] = 1;
    return actor->unknownD9 = 10073;
}

/* Shows which features (H, E, N, F) the puzzle's rows and columns (and
   layers, at level 3) sort by. */
/* @zoombi32 0x00426daf */
void drawFeatureLabels()
{
    ShortRect whole = {500, 1, 600, 27};
    ShortRect left = {500, 1, 549, 27};
    ShortRect right = {550, 1, 600, 27};
    Color saved;
    char names[8] = "H\0E\0N\0F";

    saved = setForeColor(Color(0xb));
    fillPortRect(Rect(whole), Color(0xe), 0);
    frameRect(Rect(whole));
    drawText(Rect(left), 0x22, &names[g_4ac0de * 2], 0xffff);
    drawText(Rect(whole), 0x22, &names[g_4ac0e0 * 2], 0xffff);
    if (g_4ac0d8 == 3)
        drawText(Rect(right), 0x22, &names[g_4ac0e2 * 2], 0xffff);
    setForeColor(saved);
    showRect(&whole);
}

/* Turns a 12 by 12 grid a quarter (`how` 0), half (1) or three quarters
   (2) round. */
/* @zoombi32 0x0042d6e9 */
void turnGrid(short (*grid)[12], short how)
{
    short copy[12][12];
    short lastRow;
    short lastColumn;
    short changed;
    short row;
    short column;

    for (row = 0; row < 12; row++)
        for (column = 0; column < 12; column++)
            copy[row][column] = 0;
    changed = 0;
    if (!how) {
        lastRow = 11;
        changed = 1;
        for (row = 0; row < 12; row++)
            for (column = 0; column < 12; column++)
                copy[column][lastRow - row] = grid[row][column];
    } else if (how == 1) {
        lastRow = 11;
        lastColumn = 11;
        changed = 1;
        for (row = 0; row < 12; row++)
            for (column = 0; column < 12; column++)
                copy[lastColumn - row][lastRow - column] = grid[row][column];
    } else if (how == 2) {
        lastColumn = 11;
        changed = 1;
        for (row = 0; row < 12; row++)
            for (column = 0; column < 12; column++)
                copy[lastColumn - column][row] = grid[row][column];
    } else if (how == 4) {
    }
    if (changed)
        for (row = 0; row < 12; row++)
            for (column = 0; column < 12; column++)
                grid[row][column] = copy[row][column];
}

/* Adds a lilly actor (showing `value`), placed among views 4-7 at random. */
/* @zoombi32 0x0042bacf */
short addLillyActor(short value)
{
    LillyActor actor;

    actor.unknownE3 = 0;
    actor.unknownE4 = 0;
    actor.unknownE2 = 0;
    actor.column = 0;
    actor.row = 0;
    actor.unknownC5 = 0;
    actor.unknownC6 = 0;
    actor.unknownC7 = 0;
    actor.unknownC8 = 0;
    actor.unknownD5 = 2;
    actor.unknownBc = 0;
    actor.unknownBe = value;
    actor.unknownC0 = 1;
    actor.unknownDc = 0;
    actor.unknownDb = 0;
    actor.unknownC2 = 0;
    actor.unknownD7 = 0;
    actor.unknownD6 = 11;
    actor.body.celsEnd = 0;
    actor.body.script = 0;
    actor.body.scriptGroup = 0;
    actor.body.frameOffset = 1;
    actor.body.running = 0;
    actor.startX = 0;
    actor.startY = 0;
    actor.body.x = 100;
    actor.body.y = 25;
    actor.body.unknownAa = 100;
    actor.body.unknownAc = 25;
    for (short row = 0; row < 12; row++)
        for (short column = 0; column < 12; column++)
            actor.grid[row][column] = 0;
    actor.unknownD9 = 63;
    actor.unknownBe = value;
    short id = addView(0x980002, drawCels, runViewScript, 10067, 8, &actor, randomBetween(4, 7), 0);
    View *view = findView(id);

    if (view) {
        view->flags = 0x980002;
        view->placed = fn_42f192;
        LillyActor *added = (LillyActor *)&view->body;

        added->body.running = 0;
    }
    return id;
}

/* @zoombi32 0x0042ae14 */
void fn_42ae14(View *view, short event)
{
    ViewBody *body = &view->body;
    LillyActor *actor = (LillyActor *)&view->body;

    switch (event) {
    case 30: {
        actor->body.x = g_4ac950[body->cels[0].image] + body->cels[0].x;
        actor->body.y = g_4ac954[body->cels[0].image] + body->cels[0].y;
        actor->body.unknownAa = g_4ac950[body->cels[0].image] + body->cels[0].x;
        actor->body.unknownAc = g_4ac954[body->cels[0].image] + body->cels[0].y;
        g_4ac91c++;
        if (g_4ac91c == 1)
            g_4af36a = 1;
        actor->unknownDb++;
        actor->unknownD6 = 0;
        if (actor->unknownDb == 2) {
            g_4acdca[g_4acdf2] = view->id;
            g_4acdf2++;
        } else {
            g_4acda0[g_4acdc8] = view->id;
            g_4acdc8++;
        }
        g_4acff4[actor->row][actor->column].attributes[0] = 0;
        body->cels[2].image = 0;
        moveView(actor->unknownE1, 0, g_4aed14);
        View *other = findView(actor->unknownE1);

        if (other) {
            other->body.running = 1;
            setViewScript(other, actor->row + 10129, 1);
            other->placed = fn_42afbe;
            other->notify = fn_42b003;
            short *parts = (short *)&other->body;
            View *rider = findView(parts[13]);

            if (rider) {
                g_4af0ea++;
                viewSnoid(rider)->unknownF7 = 1;
            }
        }
        if (g_4af0ea == g_4af0e8)
            queueViewSound(randomBetween(20055, 20063), 0);
        break;
    }
    }
}

/* Draws the board's square (row, column): its image and its overlay. */
/* @zoombi32 0x0042bd71 */
void drawSquare(short row, short column)
{
    short image = g_4acff4[row][column].attributes[2] + 1;
    unsigned short *data;
    short x;
    short y;

    if (image > 0 && image < 22) {
        data = (unsigned short *)((char *)g_4af5a0 + g_4af5a0->offsets[image]);
        x = g_4acff4[row][column].rect.left - g_4ac948[g_4acff4[row][column].attributes[2] + 1];
        y = g_4acff4[row][column].rect.top - g_4ac94c[g_4acff4[row][column].attributes[2] + 1];
        drawImageData(data, x, y, 8);
    }
    image = g_4acff4[row][column].attributes[4];
    if (image > 0 && image < 22) {
        data = (unsigned short *)((char *)g_4af5a0 + g_4af5a0->offsets[image]);
        x = g_4acff4[row][column].rect.left - g_4ac948[g_4acff4[row][column].attributes[4]];
        y = g_4acff4[row][column].rect.top - g_4ac94c[g_4acff4[row][column].attributes[4]];
        drawImageData(data, x, y, 8);
    }
}

/* Draws the whole board. */
/* @zoombi32 0x0042bf1b */
void drawBoard(short)
{
    short image;
    unsigned short *data;
    short x;
    short y;

    for (short row = 0; row < 12; row++)
        for (short column = 0; column < 12; column++) {
            image = g_4acff4[row][column].attributes[2] + 1;
            if (image > 0 && image < 22) {
                data = (unsigned short *)((char *)g_4af5a0 + g_4af5a0->offsets[image]);
                x = g_4acff4[row][column].rect.left - g_4ac948[g_4acff4[row][column].attributes[2] + 1];
                y = g_4acff4[row][column].rect.top - g_4ac94c[g_4acff4[row][column].attributes[2] + 1];
                drawImageData(data, x, y, 8);
            }
            image = g_4acff4[row][column].attributes[4];
            if (image > 0 && image < 22) {
                data = (unsigned short *)((char *)g_4af5a0 + g_4af5a0->offsets[image]);
                x = g_4acff4[row][column].rect.left - g_4ac948[g_4acff4[row][column].attributes[4]];
                y = g_4acff4[row][column].rect.top - g_4ac94c[g_4acff4[row][column].attributes[4]];
                drawImageData(data, x, y, 8);
            }
        }
}

/* Draws the square under the cursor, its image animating. */
/* @zoombi32 0x0042c119 */
void drawCursorSquare(View *view)
{
    if (view->body.running) {
        LillyCell *cell = &g_4acff4[g_4af346][g_4af344];
        short image = g_4a1e16[g_4acff4[g_4af346][g_4af344].attributes[2]] + g_4a1e28[g_4a1e30];
        unsigned short *data;
        short x;
        short y;

        if (image > 0 && image < 36) {
            data = (unsigned short *)((char *)g_4af5a0 + g_4af5a0->offsets[image]);
            x = cell->rect.left - g_4ac948[g_4acff4[g_4af346][g_4af344].attributes[2] + 1];
            y = cell->rect.top - g_4ac94c[g_4acff4[g_4af346][g_4af344].attributes[2] + 1];
            drawImageData(data, x, y, 8);
        }
        image = g_4acff4[g_4af346][g_4af344].attributes[4];
        if (image > 0 && image < 36) {
            data = (unsigned short *)((char *)g_4af5a0 + g_4af5a0->offsets[image]);
            x = cell->rect.left - g_4ac948[g_4acff4[g_4af346][g_4af344].attributes[4]];
            y = cell->rect.top - g_4ac94c[g_4acff4[g_4af346][g_4af344].attributes[4]];
            drawImageData(data, x, y, 8);
        }
        if (clockTime() >= view->nextUpdate) {
            view->nextUpdate = clockTime() + view->interval;
            g_4a1e30++;
            if (g_4a1e30 > 3)
                g_4a1e30 = 0;
        }
    }
}

/*
 * One step of a search over layer `layer` from (row, column): marks each
 * unmarked neighbour whose square's attribute `attribute` is `layer`, with
 * the way back and the distance, and queues it.
 */
/* Not exact: register allocation (the original keeps `layer` in esi, the
   direction in ecx and `open` in edi). */
/* @zoombi32 0x0042ea3d */
void searchStep(short attribute, short layer, short row, short column)
{
    short back;
    short open;
    char c;
    char r;

    if (g_4ad7e0[layer].marks[row][column])
        for (short direction = 0; direction < 4; direction++) {
            open = 1;
            c = column;
            r = row;
            switch (direction) {
            case 0:
                r--;
                if (r < 1) {
                    r++;
                    open = 0;
                }
                back = 2;
                break;
            case 1:
                c++;
                if (c > 11) {
                    c--;
                    open = 0;
                }
                back = 3;
                break;
            case 2:
                r++;
                if (r > 12) {
                    r--;
                    open = 0;
                }
                back = 0;
                break;
            case 3:
                c--;
                if (c < 0) {
                    c++;
                    open = 0;
                }
                back = 1;
                break;
            }
            if (open && g_4acff4[r - 1][c].attributes[attribute] == layer && !g_4ad7e0[layer].marks[r][c]) {
                g_4af668[g_4af8a8].x = c;
                g_4af668[g_4af8a8].y = r;
                g_4af8a8++;
                g_4ad7e0[layer].ways[r][c] = back;
                g_4ad7e0[layer].steps[r][c] = g_4ad7e0[layer].steps[row][column] + 1;
                g_4ad7e0[layer].marks[r][c] = g_4ad7e0[layer].marks[row][column];
            }
        }
}

/* Places a lilly actor's two parts as it jumps to (targetX, targetY):
   frames 0-2 where it is, 3-4 part way (by steps that double), 5-9 there. */
/* @zoombi32 0x0042a4d2 */
void placeJumper(View *view)
{
    LillyActor *actor = (LillyActor *)&view->body;
    short *cel;
    short image;

    switch (actor->body.frame) {
    case 0:
        actor->targetX = 599;
        actor->targetY = 55;
        actor->stepX = (actor->targetX - actor->body.x) / 3;
        actor->stepY = (actor->targetY - actor->body.y) / 3;
        /* fall through */
    case 1:
    case 2:
        cel = (short *)&view->body;
        image = *cel++;
        *cel++ -= g_4ac950[image];
        *cel++ -= g_4ac954[image];
        *cel += actor->unknownE0;
        image = *cel++;
        *cel++ -= g_4ac950[image];
        *cel -= g_4ac954[image];
        break;
    case 3:
    case 4:
        cel = (short *)&view->body;
        image = *cel++;
        *cel++ = actor->body.x + actor->stepX - g_4ac950[image];
        *cel++ = actor->body.y + actor->stepY - g_4ac954[image];
        *cel += actor->unknownE0;
        image = *cel++;
        *cel++ = actor->body.x + actor->stepX - g_4ac950[image];
        *cel = actor->body.y + actor->stepY - g_4ac954[image];
        actor->stepX += actor->stepX;
        actor->stepY += actor->stepY;
        break;
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
        cel = (short *)&view->body;
        image = *cel++;
        *cel++ = actor->targetX - g_4ac950[image];
        *cel++ = actor->targetY - g_4ac954[image];
        *cel += actor->unknownE0;
        image = *cel++;
        *cel++ = actor->targetX - g_4ac950[image];
        *cel = actor->targetY - g_4ac954[image];
        break;
    }
}

/*
 * Deals out the twelve squares' contents: three sets (3, 4 and 5 entries,
 * copied from g_4a1e3c, g_4a1e56 and g_4a1b1e), each square taking one of
 * its set's remaining entries at random.
 */
/* Not exact: register allocation (the original caches g_4af5b0's address
   in edi and keeps the square in esi). */
/* @zoombi32 0x0042ca39 */
void dealSquares()
{
    short left1;
    short left2;
    short left3;
    short *left;
    short i;

    for (i = 0; i < 3; i++) {
        g_4af5bc[i] = g_4a1e3c[i];
        g_4af5c4[i] = g_4a1e56[i];
        g_4af5cc[i] = g_4a1b1e[i];
    }
    for (i = 3; i < 7; i++) {
        g_4af5d4[i - 3] = g_4a1e3c[i];
        g_4af5de[i - 3] = g_4a1e56[i];
        g_4af5e8[i - 3] = g_4a1b1e[i];
    }
    for (i = 7; i < 12; i++) {
        g_4af5f2[i - 7] = g_4a1e3c[i];
        g_4af5fe[i - 7] = g_4a1e56[i];
        g_4af60a[i - 7] = g_4a1b1e[i];
    }
    left1 = 2;
    left2 = 3;
    left3 = 4;
    for (short n = 1; n < 13; n++) {
        switch (n) {
        case 1:
        case 2:
        case 3:
            g_4af5b0[0] = g_4af5bc;
            g_4af5b0[1] = g_4af5c4;
            g_4af5b0[2] = g_4af5cc;
            left = &left1;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            g_4af5b0[0] = g_4af5d4;
            g_4af5b0[1] = g_4af5de;
            g_4af5b0[2] = g_4af5e8;
            left = &left2;
            break;
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
            g_4af5b0[0] = g_4af5f2;
            g_4af5b0[1] = g_4af5fe;
            g_4af5b0[2] = g_4af60a;
            left = &left3;
            break;
        }
        short k = randomBetween(0, *left);

        g_4af616[n].a = g_4af5b0[0][k];
        g_4af616[n].b = g_4af5b0[1][k];
        g_4af616[n].c = g_4af5b0[2][k];
        for (; k < *left + 1; k++) {
            g_4af5b0[0][k] = g_4af5b0[0][k + 1];
            g_4af5b0[1][k] = g_4af5b0[1][k + 1];
            g_4af5b0[2][k] = g_4af5b0[2][k + 1];
        }
        (*left)--;
    }
}

/*
 * A lilly view's update: when reset, lays out the first frame of its script
 * ('SCRB' by its kind), offset by where it stands if it has flag 0x800000,
 * and sets its bounds. The original stores each cel's y in the rectangle's
 * right edge rather than its top, which is kept as written.
 */
/* Not exact: register allocation (the original keeps `left` in edi and
   reads `view` from the stack each time). */
/* @zoombi32 0x00426fd3 */
void layOutLillyView(View *view, short region)
{
    ShortRect rect;
    ShortRect bounds;
    unsigned short *image;
    short offsetX;
    short offsetY;
    long resource;
    short *at;
    short *cel;
    short left;
    short word;

    resource = 0;
    if (view->reset) {
        view->changed = 1;
        view->reset = 0;
        at = loadSwappedResource(&resource, view->kind, RESOURCE_TYPE('S', 'C', 'R', 'B')) + 1;
        if (view->flags & 0x800000) {
            view->body.unknownAa = at[1];
            view->body.unknownAc = at[2];
            offsetX = view->body.x - view->body.unknownAa;
            offsetY = view->body.y - view->body.unknownAc;
        } else {
            offsetX = offsetY = 0;
        }
        cel = (short *)&view->body;
        left = 24;
        bounds.left = bounds.right = bounds.top = bounds.bottom = 0;
        do {
            left--;
            word = *at++;
            if (!word) {
                at += 2;
                *cel++ = 0;
                *cel++ = 0;
                *cel++ = 0;
            } else if (word > 0) {
                *cel++ = word;
                if (g_4ac0d8 == 2) {
                    image = (unsigned short *)((char *)g_4ac178 + g_4ac178->offsets[word]);
                    *cel++ = rect.left = offsetX + *at++ - g_4ac1a0[word];
                    *cel++ = rect.right = offsetY + *at++ - g_4ac1a4[word];
                } else if (g_4ac0d8 == 3) {
                    image = (unsigned short *)((char *)g_4ac17c + g_4ac17c->offsets[word]);
                    *cel++ = rect.left = offsetX + *at++;
                    *cel++ = rect.right = offsetY + *at++;
                }
                rect.right = swapShort(image[0]) + rect.left;
                rect.bottom = swapShort(image[1]) + rect.top;
                unionRect(&bounds, &rect);
            } else {
                if (word < -0x100)
                    at++;
                if (left)
                    *cel = left = 0;
            }
        } while (left);
        view->body.bounds = bounds;
        fn_46c602(&resource);
    }
}

/*
 * Follows the rising numbers in an actor's grid from where it is (clearing
 * them as it goes) until it reaches `limit` across (unknownC0 0) or down
 * (1), or 200 steps.
 */
/* Not exact: register allocation (the original keeps `actor` in esi and
   the direction in ecx, `reach` on the stack). */
/* @zoombi32 0x0042ef4d */
void followGrid(LillyActor *actor, short limit)
{
    short best;
    short bestX;
    short bestY;
    short x;
    short y;
    short value;
    short reach;
    short steps;
    char direction;

    if (!actor->unknownC0) {
        actor->grid[actor->unknownC8][actor->unknownC7] = 0;
        x = actor->unknownC7;
        y = actor->unknownC8;
    } else {
        actor->grid[actor->row][actor->column] = 0;
        x = actor->column;
        y = actor->row;
    }
    steps = 0;
    bestX = x;
    bestY = y;
    if (!actor->unknownC0)
        reach = actor->column;
    else
        reach = actor->row;
    best = 0;
    while (reach < limit && steps < 200) {
        for (direction = 0; direction < 4; direction++) {
            char c = x;
            char r = y;

            switch (direction) {
            case 0:
                r--;
                if (r < 0) {
                    r++;
                    value = 0;
                } else {
                    value = actor->grid[r][c];
                }
                break;
            case 1:
                c++;
                if (c > 11) {
                    c--;
                    value = 0;
                } else {
                    value = actor->grid[r][c];
                }
                break;
            case 2:
                r++;
                if (r > 11) {
                    r--;
                    value = 0;
                } else {
                    value = actor->grid[r][c];
                }
                break;
            case 3:
                c--;
                if (c < 0) {
                    c++;
                    value = 0;
                } else {
                    value = actor->grid[r][c];
                }
                break;
            }
            if (value > best) {
                best = value;
                bestX = c;
                bestY = r;
                if (!actor->unknownC0 && c > reach)
                    reach = c;
                else if (actor->unknownC0 == 1 && r > reach)
                    reach = r;
            }
        }
        actor->grid[bestY][bestX] = 0;
        x = bestX;
        y = bestY;
        steps++;
    }
}

/* The same, jumping to its place in g_4a1ca4 (by unknownBe). */
/* @zoombi32 0x0042ab6f */
void placeJumperAt(View *view)
{
    LillyActor *actor = (LillyActor *)&view->body;
    short *cel;
    short image;

    switch (actor->body.frame) {
    case 0:
        actor->targetX = g_4a1ca4[actor->unknownBe].x;
        actor->targetY = g_4a1ca4[actor->unknownBe].y;
        actor->stepX = (actor->targetX - actor->body.x) / 3;
        actor->stepY = (actor->targetY - actor->body.y) / 3;
        /* fall through */
    case 1:
    case 2:
        cel = (short *)&view->body;
        image = *cel++;
        *cel++ -= g_4ac950[image];
        *cel++ -= g_4ac954[image];
        *cel += actor->unknownE0;
        image = *cel++;
        *cel++ -= g_4ac950[image];
        *cel -= g_4ac954[image];
        break;
    case 3:
    case 4:
        cel = (short *)&view->body;
        image = *cel++;
        *cel++ = actor->body.x + actor->stepX - g_4ac950[image];
        *cel++ = actor->body.y + actor->stepY - g_4ac954[image];
        *cel += actor->unknownE0;
        image = *cel++;
        *cel++ = actor->body.x + actor->stepX - g_4ac950[image];
        *cel = actor->body.y + actor->stepY - g_4ac954[image];
        actor->stepX += actor->stepX;
        actor->stepY += actor->stepY;
        break;
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
        cel = (short *)&view->body;
        image = *cel++;
        *cel++ = actor->targetX - g_4ac950[image];
        *cel++ = actor->targetY - g_4ac954[image];
        *cel += actor->unknownE0;
        image = *cel++;
        *cel++ = actor->targetX - g_4ac950[image];
        *cel = actor->targetY - g_4ac954[image];
        break;
    }
}

/* Adds the lilly actors (g_4af102 of them), each dealt a random entry of
   g_4a1dcc/g_4a1de4. */
/* @zoombi32 0x0042b857 */
void addLillyActors()
{
    LillyActor actor;
    short i;

    g_4af5a6 = 11;
    for (i = 0; i < g_4af5a6 + 1; i++)
        g_4a1db2[i] = i;
    for (i = 0; i < g_4af102; i++) {
        actor.unknownBc = 0;
        actor.unknownBe = i;
        actor.unknownC0 = 0;
        actor.column = 0;
        actor.row = 0;
        actor.unknownC5 = 0;
        actor.unknownC6 = 0;
        actor.unknownC7 = 0;
        actor.unknownC8 = 0;
        actor.unknownD7 = 0;
        actor.unknownD5 = 1;
        actor.unknownD6 = 11;
        actor.unknownDc = 0;
        actor.unknownDb = 0;
        actor.unknownC2 = 0;
        actor.unknownE1 = 0;
        actor.unknownE2 = 0;
        actor.unknownE3 = 0;
        actor.unknownE4 = 0;
        actor.body.celsEnd = 0;
        actor.body.running = 1;
        actor.startX = 0;
        actor.startY = 0;
        actor.targetX = 0;
        actor.targetY = 0;
        actor.body.x = 0;
        actor.body.y = 0;
        actor.body.unknownAa = 0;
        actor.body.unknownAc = 0;
        for (short row = 0; row < 12; row++)
            for (short column = 0; column < 12; column++)
                actor.grid[row][column] = 0;
        short k = randomBetween(0, g_4af5a6);

        actor.unknownDe = g_4a1dcc[g_4a1db2[k]];
        actor.unknownDf = g_4a1de4[g_4a1db2[k]];
        actor.unknownE0 = g_4a1db2[k];
        g_4ac95a[i] = actor.unknownDe;
        g_4ac972[i] = actor.unknownDf;
        for (; k < g_4af5a6 + 1; k++)
            g_4a1db2[k] = g_4a1db2[k + 1];
        g_4af5a6--;
        g_4aed64[i] = addView(0x180002, drawCels, runViewScript, i + 10043, 7, &actor, randomBetween(3, 6), 0);
        View *view = findView(g_4aed64[i]);

        if (view) {
            short *parts = (short *)&view->body;

            parts[20] = i;
            setViewScript(view, i + 10043, 1);
            view->flags = 0x980002;
            view->placed = fn_42f192;
        }
    }
}

/* The same, landing at (484, 450), with any further parts placed there too. */
/* @zoombi32 0x0042a840 */
void placeLander(View *view)
{
    LillyActor *actor = (LillyActor *)&view->body;
    short *cel;
    short image;

    switch (actor->body.frame) {
    case 0:
        actor->targetX = 484;
        actor->targetY = 450;
        actor->stepX = (actor->targetX - actor->body.x) / 3;
        actor->stepY = (actor->targetY - actor->body.y) / 3;
        /* fall through */
    case 1:
    case 2:
        cel = (short *)&view->body;
        image = *cel++;
        *cel++ -= g_4ac950[image];
        *cel++ -= g_4ac954[image];
        *cel += actor->unknownE0;
        image = *cel++;
        *cel++ -= g_4ac950[image];
        *cel -= g_4ac954[image];
        break;
    case 3:
    case 4:
        cel = (short *)&view->body;
        image = *cel++;
        *cel++ = actor->body.x + actor->stepX - g_4ac950[image];
        *cel++ = actor->body.y + actor->stepY - g_4ac954[image];
        *cel += actor->unknownE0;
        image = *cel++;
        *cel++ = actor->body.x + actor->stepX - g_4ac950[image];
        *cel = actor->body.y + actor->stepY - g_4ac954[image];
        actor->stepX += actor->stepX;
        actor->stepY += actor->stepY;
        break;
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
        cel = (short *)&view->body;
        image = *cel++;
        *cel++ = actor->targetX - g_4ac950[image];
        *cel++ = actor->targetY - g_4ac954[image];
        if (*cel == 0x5b) {
            *cel += actor->unknownE0;
            image = *cel++;
            *cel++ = actor->targetX - g_4ac950[image];
            *cel++ = actor->targetY - g_4ac954[image];
        }
        while (*cel) {
            image = *cel++;
            *cel++ = actor->targetX - g_4ac950[image];
            *cel++ = actor->targetY - g_4ac954[image];
        }
        break;
    }
}

/*
 * Picks a lilly actor's next square: the first neighbour that's free, of
 * its kind and nearer (by its layer's search), claiming it; the script to
 * run next, or 0.
 */
/* Not exact: register allocation (the original keeps `actor` on the stack
   and the direction in eax). */
/* @zoombi32 0x0042f506 */
short fn_42f506(View *view)
{
    LillyActor *actor = (LillyActor *)&view->body;
    short done = 0;
    short best;
    short bestColumn;
    short bestRow;
    short column = actor->column;
    short row = actor->row + 1;
    char direction = 0;

    best = 5;
    while (direction < 4 && !done) {
        short open = 1;
        char c = column;
        char r = row;

        switch (direction) {
        case 0:
            r--;
            if (r < 1) {
                r = 1;
                open = 0;
            }
            break;
        case 1:
            c++;
            if (c > 11) {
                c = 11;
                open = 0;
            }
            break;
        case 2:
            r++;
            if (r > 12) {
                r = 12;
                open = 0;
                done = 1;
                direction = 4;
            }
            break;
        case 3:
            c--;
            if (c < 0) {
                c = 0;
                open = 0;
            }
            break;
        }
        if (open && !g_4acff4[r - 1][c].attributes[0]
            && g_4acff4[r - 1][c].attributes[actor->unknownDe] == actor->unknownDf
            && g_4ad7e0[actor->unknownDf].steps[r][c] < g_4ad7e0[actor->unknownDf].steps[actor->row + 1][actor->column]) {
            best = direction;
            bestColumn = c;
            bestRow = r - 1;
            direction = 4;
        }
        direction++;
    }
    if (done)
        return actor->unknownD9 = 10069;
    switch (best) {
    case 0:
        actor->unknownD5 = 0;
        g_4acff4[bestRow][bestColumn].attributes[0] = 1;
        return actor->unknownD9 = 10071;
    case 1:
        actor->unknownD5 = 1;
        g_4acff4[bestRow][bestColumn].attributes[0] = 1;
        return actor->unknownD9 = 10077;
    case 2:
        actor->unknownD5 = 2;
        g_4acff4[bestRow][bestColumn].attributes[0] = 1;
        return actor->unknownD9 = 10073;
    case 3:
        actor->unknownD5 = 3;
        g_4acff4[bestRow][bestColumn].attributes[0] = 1;
        return actor->unknownD9 = 10075;
    }
    return 0;
}

/* A hopping lilly actor's script events: 11 sets off for the next square
   (unknownD5: 0 up, 1 right, 2 down, 3 left), 12 puts it halfway, 13-14
   there, 10 and 15 end the hop. */
/* @zoombi32 0x00429d94 */
void hopNotify(View *view, short event)
{
    short *cel = (short *)&view->body;
    LillyActor *actor = (LillyActor *)&view->body;

    switch (event) {
    case 11:
        actor->unknownC5 = actor->column;
        actor->unknownC6 = actor->row;
        switch (actor->unknownD5) {
        case 0:
            actor->targetX = g_4ac940[--actor->row + 1] + actor->column * 35;
            break;
        case 2:
            actor->targetX = g_4ac940[++actor->row + 1] + actor->column * 35;
            break;
        case 1:
            actor->targetX = g_4ac940[actor->row + 1] + ++actor->column * 35;
            break;
        case 3:
            actor->targetX = g_4ac940[actor->row + 1] + --actor->column * 35;
            break;
        }
        actor->targetY = g_4ac944[actor->row + 1] + g_4a1d70[actor->column];
        *(Point *)&actor->startX = *(Point *)&actor->body.x;
        break;
    case 12:
        switch (actor->unknownD5) {
        case 0:
        case 2:
            while (*cel++) {
                cel++;
                *cel++ = (actor->targetY - actor->startY) / 2 + actor->body.y;
            }
            break;
        case 1:
        case 3:
            while (*cel++) {
                *cel++ = (actor->targetX - actor->startX) / 2 + actor->body.x;
                cel++;
            }
            break;
        }
        g_4acff4[actor->unknownC6][actor->unknownC5].attributes[0] = 0;
        break;
    case 13:
    case 14:
        switch (actor->unknownD5) {
        case 0:
        case 2:
            while (*cel++) {
                *cel++ = actor->targetX;
                cel++;
            }
            break;
        case 1:
        case 3:
            while (*cel++) {
                *cel++ = actor->targetX;
                cel++;
            }
            break;
        }
        break;
    case 10:
        actor->body.x = actor->targetX;
        actor->body.y = actor->targetY;
        actor->body.unknownAa = actor->targetX;
        actor->body.unknownAc = actor->targetY;
        g_4acd4c[g_4acd74] = view->id;
        g_4acd74++;
        view->nextUpdate = clockTime() + 30;
        break;
    case 15:
        g_4acd4c[g_4acd74] = view->id;
        g_4acd74++;
        break;
    }
}

/*
 * Picks a lilly actor's next square by its grid of visits: the least
 * visited neighbour that's free and of its kind, trying directions from
 * the one it faces; off the right edge it jumps (4) if it can. Claims the
 * square, notes the visit and returns the script to run (0: none).
 */
/* @zoombi32 0x0042b276 */
short fn_42b276(LillyActor *actor)
{
    char direction;
    char tries;
    short blocked;
    short best;
    short value;
    short current;
    short bestColumn;
    short bestRow;
    short i;

    if (actor->grid[actor->row][actor->column] >= 10000) {
        for (i = 0; i < 12; i++)
            for (short j = 0; j < 12; j++)
                actor->grid[i][j] = 0;
        actor->grid[actor->row][actor->column] = 1;
    }
    value = actor->grid[actor->row][actor->column];
    if (!value) {
        value = 1;
        actor->grid[actor->row][actor->column] = 1;
    }
    current = value;
    direction = actor->unknownD5;
    blocked = 0;
    tries = 0;
    best = 5;
    char c;
    char r;

    while (tries < 4 && !blocked) {
        short open = 1;

        c = actor->column;
        r = actor->row;
        switch (direction) {
        case 0:
            r--;
            if (r < 0) {
                r = 0;
                open = 0;
            }
            break;
        case 1:
            c++;
            if (c > 11) {
                c = 11;
                open = 0;
                blocked = 1;
            }
            break;
        case 2:
            r++;
            if (r > 11) {
                r = 11;
                open = 0;
            }
            break;
        case 3:
            c--;
            if (c < 0) {
                c = 0;
                open = 0;
            }
            break;
        }
        if (open) {
            if (!g_4acff4[r][c].attributes[0]) {
                switch (actor->unknownDe) {
                case 1:
                    if (g_4acff4[r][c].attributes[1] != actor->unknownDf)
                        open = 0;
                    break;
                case 2:
                    if (g_4acff4[r][c].attributes[2] != actor->unknownDf)
                        open = 0;
                    break;
                case 3:
                    if (g_4acff4[r][c].attributes[3] != actor->unknownDf)
                        open = 0;
                    break;
                }
            } else {
                open = 0;
            }
            if (open && actor->grid[r][c] < value) {
                best = direction;
                value = actor->grid[r][c];
                bestColumn = c;
                bestRow = r;
            }
        }
        direction++;
        if (direction > 3)
            direction = 0;
        tries++;
    }
    if (blocked) {
        if (!g_4acff4[r][c + 1].attributes[0]) {
            best = 4;
            g_4acff4[r][c + 1].attributes[0] = 1;
        } else {
            best = 5;
        }
    }
    switch (best) {
    case 0:
        actor->unknownD5 = 0;
        actor->unknownD9 = g_4a1d8a[0][actor->unknownD5];
        actor->grid[actor->row - 1][actor->column] = current + 1;
        g_4acff4[bestRow][bestColumn].attributes[0] = 1;
        return actor->unknownD9;
    case 1:
        actor->unknownD5 = 1;
        actor->unknownD9 = g_4a1d8a[1][actor->unknownD5];
        actor->grid[actor->row][actor->column + 1] = current + 1;
        g_4acff4[bestRow][bestColumn].attributes[0] = 1;
        return actor->unknownD9;
    case 2:
        actor->unknownD5 = 2;
        actor->unknownD9 = g_4a1d8a[2][actor->unknownD5];
        actor->grid[actor->row + 1][actor->column] = current + 1;
        g_4acff4[bestRow][bestColumn].attributes[0] = 1;
        return actor->unknownD9;
    case 3:
        actor->unknownD5 = 3;
        actor->unknownD9 = g_4a1d8a[3][actor->unknownD5];
        actor->grid[actor->row][actor->column - 1] = current + 1;
        g_4acff4[bestRow][bestColumn].attributes[0] = 1;
        return actor->unknownD9;
    case 4:
        actor->unknownD5 = 1;
        actor->unknownD9 = 10031;
        return actor->unknownD9;
    default:
        return 0;
    }
    return 0;
}

/*
 * Plans a lilly actor's way across (unknownC0 0) or down (1) to `limit`,
 * numbering the squares it would visit in its grid: from each square, the
 * least visited neighbour that's free and of its kind, trying directions
 * from the last one taken; at most 200 steps. How far it got.
 */
/* @zoombi32 0x0042ec2a */
short planWay(LillyActor *actor, short limit)
{
    char direction;
    unsigned short tries;
    short best;
    short value;
    short count;
    short bestColumn;
    short bestRow;
    short column;
    short row;
    short steps;
    short reach;
    short heading;

    if (!actor->unknownC0)
        reach = actor->column;
    else
        reach = actor->row;
    if (actor->unknownD6 == 11) {
        column = actor->column;
        row = actor->row;
    } else {
        column = actor->unknownC7;
        row = actor->unknownC8;
    }
    bestColumn = column;
    bestRow = row;
    heading = actor->unknownD5;
    actor->grid[row][column] = actor->unknownD7;
    value = actor->unknownD7;
    count = value;
    steps = 0;
    while (steps < 200 && reach < limit) {
        direction = heading;
        tries = 0;
        while (tries < 4 && reach < limit) {
            short open = 1;
            char c = column;
            char r = row;

            switch (direction) {
            case 0:
                r--;
                if (r < 0) {
                    r++;
                    open = 0;
                }
                break;
            case 1:
                c++;
                if (c > 11) {
                    c--;
                    open = 0;
                    if (!actor->unknownC0)
                        reach = c;
                }
                break;
            case 2:
                r++;
                if (r > 11) {
                    r--;
                    open = 0;
                    if (actor->unknownC0 == 1)
                        reach = r;
                }
                break;
            case 3:
                c--;
                if (c < 0) {
                    c++;
                    open = 0;
                }
                break;
            }
            if (open) {
                switch (actor->unknownDe) {
                case 1:
                    if (g_4acff4[r][c].attributes[1] != actor->unknownDf)
                        open = 0;
                    break;
                case 2:
                    if (g_4acff4[r][c].attributes[2] != actor->unknownDf)
                        open = 0;
                    break;
                case 3:
                    if (g_4acff4[r][c].attributes[3] != actor->unknownDf)
                        open = 0;
                    break;
                }
            } else {
                open = 0;
            }
            if (open && actor->grid[r][c] < value) {
                best = direction;
                value = actor->grid[r][c];
                bestColumn = c;
                bestRow = r;
                if (!actor->unknownC0) {
                    if (c > reach)
                        reach = c;
                    if (c < actor->unknownC7) {
                        actor->unknownC7 = c;
                        actor->unknownC8 = r;
                    }
                } else {
                    if (r > reach)
                        reach = r;
                    if (r < actor->unknownC8) {
                        actor->unknownC7 = c;
                        actor->unknownC8 = r;
                    }
                }
            }
            direction++;
            if (direction > 3)
                direction = 0;
            tries++;
        }
        heading = best;
        actor->grid[bestRow][bestColumn] = count + 1;
        column = bestColumn;
        row = bestRow;
        value = count + 1;
        count = value;
        steps++;
    }
    actor->unknownD7 = actor->grid[row][column];
    return actor->unknownD6 = reach;
}

/* Places a hopping lilly actor's cels through its hop (by frame), its
   parts showing image 0x110 offset by unknownE0 and the rest hidden while
   its ninth cel's x is set. */
/* @zoombi32 0x0042a163 */
void placeHopper(View *view)
{
    short *cel = (short *)&view->body;
    LillyActor *actor = (LillyActor *)&view->body;
    short x;
    short y;
    short shown = !cel[25];
    short image;

    switch (actor->body.frame) {
    case 0:
        actor->unknownC5 = actor->column;
        actor->unknownC6 = actor->row;
        switch (actor->unknownD9) {
        case 10071:
            actor->targetX = g_4ac940[--actor->row + 1] + actor->column * 35;
            break;
        case 10073:
            actor->targetX = g_4ac940[++actor->row + 1] + actor->column * 35;
            break;
        case 10077:
            actor->targetX = g_4ac940[actor->row + 1] + ++actor->column * 35;
            break;
        case 10075:
            actor->targetX = g_4ac940[actor->row + 1] + --actor->column * 35;
            break;
        }
        actor->targetY = g_4ac944[actor->row + 1] + g_4a1d70[actor->column];
        *(Point *)&actor->startX = *(Point *)&actor->body.x;
        /* fall through */
    case 1:
    case 3:
    case 4:
    case 5:
        x = cel[1];
        y = cel[2];
        break;
    case 2:
        x = (actor->targetX - actor->startX) / 2 + actor->body.x;
        y = (actor->targetY - actor->startY) / 2 + actor->body.y;
        break;
    case 6:
        x = actor->targetX;
        y = actor->targetY;
        if (!cel[25])
            g_4acff4[actor->unknownC6][actor->unknownC5].attributes[0] = 0;
        break;
    case 7:
        if (cel[26])
            shown = cel[26];
        x = actor->targetX;
        y = actor->targetY;
        actor->body.x = x;
        actor->body.y = y;
        actor->body.unknownAa = x;
        actor->body.unknownAc = y;
        view->nextUpdate = clockTime() + 35;
        g_4ac9e6[g_4acb06] = view->id;
        g_4acb06++;
        break;
    }
    image = *cel++;
    if (image) {
        *cel++ = x - g_4ac950[image];
        *cel++ = y - g_4ac954[image];
    }
    if (*cel == 0x110 && shown) {
        *cel += actor->unknownE0;
        image = *cel++;
        *cel++ = x - g_4ac950[image];
        *cel++ = y - g_4ac954[image];
    } else if (!shown) {
        *cel++ = 0;
        cel++;
        cel++;
    }
    while (*cel) {
        image = *cel++;
        *cel++ = x - g_4ac950[image];
        *cel++ = y - g_4ac954[image];
    }
}

/* Whether a, b and c fit square n of the 5 by 5 by 5 puzzle: its row,
   layer and column sort by them, or can, the values being unused elsewhere. */
/* @zoombi32 0x00427217 */
short fn_427217(short a, short b, short c, short n)
{
    short column;
    short layer;
    short row;
    short i;

    if (g_4abec4)
        return 1;
    column = n % 5;
    layer = n / 25;
    row = n % 25;
    row = row / 5;
    if (!g_4ac1a8[row] && !g_4ac1da[layer] && !g_4ac20c[column]) {
        for (i = 0; i < 5; i++) {
            if (b == g_4ac1da[i])
                return 0;
            if (a == g_4ac1a8[i])
                return 0;
            if (c == g_4ac20c[i])
                return 0;
        }
        return 1;
    }
    if (!g_4ac1a8[row])
        for (i = 0; i < 5; i++)
            if (a == g_4ac1a8[i])
                return 0;
    if (!g_4ac1da[layer])
        for (i = 0; i < 5; i++)
            if (b == g_4ac1da[i])
                return 0;
    if (!g_4ac20c[column])
        for (i = 0; i < 5; i++)
            if (c == g_4ac20c[i])
                return 0;
    if (g_4ac20c[column] && c != g_4ac20c[column])
        return 0;
    if (g_4ac1a8[row] && a != g_4ac1a8[row])
        return 0;
    if (g_4ac1da[layer] && b != g_4ac1da[layer])
        return 0;
    if (a == g_4ac1a8[row] && c == g_4ac20c[column] && b == g_4ac1da[layer])
        return 1;
    if (a == g_4ac1a8[row] && !g_4ac1da[layer])
        for (i = 0; i < 5; i++)
            if (b == g_4ac1da[i])
                return 0;
    if (b == g_4ac1da[layer] && !g_4ac1a8[row])
        for (i = 0; i < 5; i++)
            if (a == g_4ac1a8[i])
                return 0;
    if (a == g_4ac1a8[row] && !g_4ac20c[column])
        for (i = 0; i < 5; i++)
            if (c == g_4ac20c[i])
                return 0;
    if (b == g_4ac1da[layer] && !g_4ac20c[column])
        for (i = 0; i < 5; i++)
            if (c == g_4ac20c[i])
                return 0;
    if (c == g_4ac20c[column] && !g_4ac1da[layer])
        for (i = 0; i < 5; i++)
            if (b == g_4ac1da[i])
                return 0;
    if (c == g_4ac20c[column] && !g_4ac1a8[row])
        for (i = 0; i < 5; i++)
            if (a == g_4ac1a8[i])
                return 0;
    return 1;
}

/*
 * Sends the Zoombini `id` onto square g_4ac0ec: works out where it stands
 * (g_4ac510/g_4ac512) and the area it covers (g_4ac514), and starts its
 * script (by the level, the square and its feet).
 */
/* Not exact: register allocation (the original keeps `lift` on the stack
   and the column in ecx). */
/* @zoombi32 0x0042790a */
void fn_42790a(short id)
{
    Point place;
    short extra;
    short lift;
    short script;
    View *view = findView(id);

    if (g_4ac0d8 != 3)
        place = g_4a1788[g_4ac0ec];
    else
        place = g_4a17f0[g_4ac0ec];
    if (g_4ac0d8 != 3) {
        place.x += 24;
        place.y -= 7;
        g_4ac512 = place.y - 2;
        if (!g_4ac0d8) {
            if (g_4ac0ec == 4)
                g_4ac510 = place.x - 5;
            else if (g_4ac0ec == 9)
                g_4ac510 = place.x - 7;
            else if (g_4ac0ec == 14)
                g_4ac510 = place.x - 3;
            else if (g_4ac0ec == 19)
                g_4ac510 = place.x - 3;
            else if (g_4ac0ec == 24)
                g_4ac510 = place.x - 3;
        } else {
            if (g_4ac0ec <= 4)
                g_4ac510 = place.x - 8;
            else if (g_4ac0ec <= 9)
                g_4ac510 = place.x - 6;
            else if (g_4ac0ec <= 14)
                g_4ac510 = place.x - 5;
            else if (g_4ac0ec <= 19)
                g_4ac510 = place.x - 4;
            else if (g_4ac0ec <= 24)
                g_4ac510 = place.x - 5;
        }
        if (!g_4abdc0[g_4ac0ec] || g_4abdc0[g_4ac0ec] % 3 == 1) {
            g_4ac510 -= 8;
            g_4ac512 = g_4abdc0[g_4ac0ec] + g_4ac512 - 1;
        } else if (g_4abdc0[g_4ac0ec] % 3 == 2) {
            g_4ac510--;
            g_4ac512 = g_4abdc0[g_4ac0ec] + g_4ac512 - 1;
        } else {
            g_4ac510 += 6;
            g_4ac512 = g_4abdc0[g_4ac0ec] + g_4ac512 - 1;
        }
    } else {
        place.x += 5;
        place.y -= 15;
        g_4ac510 = place.x;
        g_4ac512 = place.y - 2;
        if (g_4abdc0[g_4ac0ec] > 1) {
            g_4ac510 -= (g_4abdc0[g_4ac0ec] - 1) * 2;
            g_4ac512 -= g_4abdc0[g_4ac0ec] - 1;
        }
    }
    if (view) {
        switch (g_4ac0d8) {
        case 0:
        case 1:
        case 2:
            if (g_4ac0ec < 10) {
                script = 13030;
                lift = 3;
            } else if (g_4ac0ec < 15) {
                script = 13035;
                lift = 0;
            } else {
                script = 13040;
                lift = 5;
            }
            g_4ac514.left = g_4a1788[g_4ac0ec].x - 16;
            g_4ac514.top = g_4a1788[g_4ac0ec].y - 30;
            g_4ac514.right = g_4ac514.left + 52;
            g_4ac514.bottom = g_4ac514.top + 82;
            if (g_4ac0d8 == 1 || g_4ac0d8 == 2) {
                short column = g_4ac0ec % 5;

                if (column <= 2)
                    g_4ac514.top -= lift;
            }
            break;
        case 3: {
            script = g_4ac0ec % 5 * 5 + 13045;
            g_4ac514.left = g_4ac190[g_4ac0ec / 5 + 1] + g_4a1a04[g_4ac0ec % 5];
            g_4ac514.top = g_4ac194[g_4ac0ec / 5 + 1] + g_4a1a0e[g_4ac0ec % 5];
            g_4ac514.right = g_4ac514.left + 22;
            g_4ac514.bottom = g_4ac514.top + 72;
            short row = g_4ac0ec % 25 / 5;

            if (g_4ac0ec % 25 == 1)
                g_4ac514.left += 5;
            else if (g_4ac0ec % 25 == 3)
                g_4ac514.left += 3;
            else
                g_4ac514.left += 4;
            if (g_4ac0ec % 5 >= 3) {
                extra = 1;
                if (g_4ac0ec % 5 == 3) {
                    g_4ac514.left--;
                    extra++;
                }
                g_4ac514.left += extra;
                g_4ac514.right += extra;
            }
            if (g_4ac0ec % 5 == 4)
                g_4ac514.left--;
            else if (g_4ac0ec % 5 == 3 && row)
                g_4ac514.left--;
            break;
        }
        }
        script = script + viewSnoid(view)->features[3] - 1;
        if (g_4ac0d8 <= 2)
            startSnoidScript(viewSnoid(view), script, 0, 0);
        else
            startSnoidScript(viewSnoid(view), script, &place, 0);
        view->notify = fn_4276d0;
        view->notifyEnd = 0;
        g_4ac0ea = view->id;
        g_4ac0f8 = groupViews(g_4ac0ea, g_4ac0ea, 0, 0, 0, 0);
    }
}

/* Adds the puzzle's views for the level: the squares' parts, the labels,
   the pieces already placed (level 2 and up) and the places to put them. */
/* Not exact: register allocation in the level-3 loops (the original keeps
   the x offset in edx and the y offset on the stack). */
/* @zoombi32 0x00425821 */
void addLillyViews()
{
    short count;
    short offsetY;
    Point place;
    short offsetsX[5] = {0, 23, 46, 69, 94};
    short offsetsY[5] = {0, 10, 16, 20, 23};
    short i;

    if (!g_4ac0d8) {
        for (i = 4; i < g_4ac0ee; i += 5)
            g_4abfc0[i] = addView(0x4188000, drawCels, runViewScript, i + 6013, 6, 0, 0, 0);
        g_4ac0bc = addView(0x4008000, drawCels, runViewScript, 11504, 6, 0, 0, 0);
        for (i = 4; i < g_4ac0ee; i += 5)
            g_4abec6[i] = addView(0xc188000, drawCels, runViewScript, i + 6038, 3, 0, 0, 0);
    } else if (g_4ac0d8 < 3) {
        for (i = 0; i < g_4ac0ee; i++)
            g_4abfc0[i] = addView(0x4188000, drawCels, runViewScript, i + 6013, 6, 0, 0, 0);
        if (g_4ac0d8 == 2) {
            count = 0;
            for (i = 0; i < g_4ac0ee; i++)
                if (g_4abdc0[i] == -1) {
                    place.x = g_4ac188[i + 1];
                    place.y = g_4ac18c[i + 1];
                    deleteView(g_4ac216[i]);
                    g_4ac216[i] = addView(0x808000, fn_426f38, layOutLillyView, g_4ac10e[count++] + 11004, 0, &place, 0, 0);
                }
        }
        g_4ac0bc = addView(0x4008000, drawCels, runViewScript, 11503, 6, 0, 0, 0);
        for (i = 0; i < g_4ac0ee; i++)
            g_4abec6[i] = addView(0xc188000, drawCels, runViewScript, i + 6038, 3, 0, 0, 0);
    } else {
        for (i = 0; i < g_4ac0ee; i++) {
            short column = i % 5;
            short row = i / 5 + 1;
            short offsetX = g_4a1a04[column];

            offsetY = g_4a1a0e[column];
            place.x = offsetX + g_4ac190[row];
            place.y = g_4ac194[row] + offsetY;
            g_4ac310[i] = addView(0x4988000, drawCels, runViewScript, column + 9002, 6, &place, 0, 0);
        }
        g_4ac0bc = addView(0x4008000, drawCels, runViewScript, 11505, 6, 0, 0, 0);
        for (i = 0; i < g_4ac0ee; i++) {
            short column = i % 5;
            short row = i / 5 + 1;
            short offsetX = offsetsX[column];

            offsetY = offsetsY[column];
            place.x = offsetX + g_4ac198[row];
            place.y = g_4ac19c[row] + offsetY;
            g_4ac40a[i] = addView(0xc988000, drawCels, runViewScript, column + 9007, 3, &place, 0, 0);
        }
        count = 0;
        for (i = 0; i < g_4ac0ee; i++)
            if (g_4abdc0[i] == -1) {
                short column = i % 5;
                short row = i / 5 + 1;
                short offsetX = offsetsX[column];

                offsetY = offsetsY[column];
                place.x = offsetX + g_4ac190[row];
                place.y = g_4ac194[row] + offsetY;
                if (!column)
                    place.y += 5;
                if (column == 2)
                    place.y += 2;
                g_4ac216[i] = addView(0x808000, fn_426f38, layOutLillyView, g_4ac10e[count++] + 12000, 0, &place, 0, 0);
            }
    }
    if (!g_4ac0d8) {
        g_4b755e += 10;
        for (i = 4; i < g_4ac0ee; i += 5)
            placedViews[(i - 4) / 5] = addView(0x508a000, drawCels, runViewScript, i + 10000, 7, &g_4a1788[i], 0, 0);
    } else if (g_4ac0d8 < 3) {
        g_4b755e += 10;
        for (i = 0; i < g_4ac0ee; i++)
            placedViews[i] = addView(0x508a000, drawCels, runViewScript, i + 10000, 7, &g_4a1788[i], 0, 0);
    } else if (g_4ac0d8 == 3) {
        g_4b755e = 10;
        for (i = 0; i < g_4ac0ee; i++)
            placedViews[i] = addView(0x108a000, drawCels, runViewScript, i + 10025, 6, &g_4a17f0[i], 0, 0);
    }
}

/* Draws both buttons, unlit. */
/* @zoombi32 0x004249c4 */
void drawButtons(short)
{
    fn_42492b(1, 0, 0);
    fn_42492b(2, 0, 0);
}

/* Draws the other set's buttons, unlit. */
/* @zoombi32 0x00428c28 */
void drawOtherButtons(short)
{
    fn_428b8f(1, 0, 0);
    fn_428b8f(2, 0, 0);
}

/* Flashes square (g_4af34a, g_4af348) until g_4af352 reaches g_4af350. */
/* @zoombi32 0x0042c52f */
void flashSquare(View *view)
{
    if (view->body.running) {
        if (g_4af352 >= g_4af350) {
            view->body.running = 0;
            unionRgnRect(removedRgn, &g_4acff4[g_4af34a][g_4af348].rect);
        } else {
            if (clockTime() >= view->nextUpdate) {
                view->nextUpdate = clockTime() + view->interval;
                g_4a1e3a++;
                if (g_4a1e3a > 1)
                    g_4a1e3a = 0;
            }
            fn_42c3b6(g_4af34a, g_4af348, g_4a1e3a);
        }
    }
}

/* The puzzle's keys (debugging ones only while debugging messages are on). */
/* @zoombi32 0x00426831 */
short lillyKey(unsigned short key)
{
    if (!g_4b8803 && key != 367)
        return 0;
    switch (key) {
    case 367:
        fn_466b93();
        return 1;
    case 'A':
    case 'a':
        drawFeatureLabels();
        return 1;
    case 'H':
    case 'h':
        if (!g_4ac0d6)
            g_4ac0d6 = 1;
        return 1;
    case 'I':
    case 'i':
        if (!g_4ac0d6)
            g_4ac0d6 = 0;
        else
            g_4ac0d6++;
        return 1;
    case 'R':
        g_4abec4 = 1;
        return 1;
    case 'W':
    case 'w': {
        if (++g_4ac0c4 > 9)
            g_4ac0c4 = 0;
        View *view = findView(g_4ac0ba);

        if (view) {
            setViewScript(view, g_4ac0c4 + 7000, 1);
        } else {
            g_4ac0ba = addView(0x8108000, drawCels, runViewScript, g_4ac0c4 + 7000, 6, 0, 0, 0);
            view = findView(g_4ac0ba);
        }
        return 1;
    }
    case 'E':
    case 'e': {
        if (++g_4ac0c6 > 17)
            g_4ac0c6 = 10;
        View *view = findView(g_4ac0ba);

        if (view) {
            setViewScript(view, g_4ac0c6 + 7000, 1);
        } else {
            g_4ac0ba = addView(0x8108000, drawCels, runViewScript, g_4ac0c6 + 7000, 6, 0, 0, 0);
            view = findView(g_4ac0ba);
        }
        return 1;
    }
    case ' ':
        g_4ac0fe = g_4ac102;
        g_4ac100 = g_4ac104;
        if (g_4ac0d8 != 3)
            startView(g_4ac0fa, g_4ac0fe + 6000, 0, 0);
        fadePalette(g_4ac51c, 10, 236, 0, 0, 0);
        return 1;
    default:
        return 0;
    }
}

/* Plans a lilly actor's way afresh from where it is. */
/* @zoombi32 0x0042e760 */
void startPlan(LillyActor *actor)
{
    for (short row = 0; row < 12; row++)
        for (short column = 0; column < 12; column++)
            actor->grid[row][column] = 0;
    actor->unknownC7 = actor->column;
    actor->unknownC8 = actor->row;
    actor->unknownD6 = 11;
    actor->unknownD7 = 1;
    planWay(actor, actor->unknownD6);
    planWay(actor, actor->unknownD6);
    followGrid(actor, actor->unknownD6);
    actor->grid[actor->row][actor->column] = actor->unknownD7;
}

/* Closes the other puzzle. */
/* @zoombi32 0x00428cb7 */
void closeOtherPuzzle()
{
    if (g_4af368) {
        g_4af368 = 0;
        short saved = fn_46bee9(1);

        fn_465175();
        setSnoidsRunning(1);
        clearViews();
        fn_46c602(&g_4a1b48);
        fn_46c602(&g_4a1b44);
        freeLockedResources(g_4af108, g_4af278, 91);
        freeResourcePair(g_4ac928);
        freeResourcePair(g_4ac938);
        freeResourcePair(g_4ac930);
        freeLockedResource(&g_4ac98c, &g_4ac994);
        freeLockedResource(&g_4ac998, &g_4ac9a0);
        freeLockedResource(&g_4ac9a4, &g_4ac9ac);
        fn_46c602(&g_4a1be8);
        unloadSounds();
        fn_46bee9(saved);
        fn_46ca9c(&g_4af364);
        fadeOutViews();
        fn_4624fc();
    }
}

/* Closes the puzzle. */
/* @zoombi32 0x00424a53 */
void closeLillyPuzzle()
{
    if (g_4abec0) {
        g_4abec0 = 0;
        short saved = fn_46bee9(1);

        clearViews();
        unloadSounds();
        fn_46c602(&g_4ac144);
        fn_46c602(&g_4ac148);
        fn_46c602(&g_4ac14c);
        fn_46c602(&g_4ac150);
        fn_46c602(&g_4ac170);
        fn_46c602(&g_4ac174);
        fn_46c602(&g_4ac154);
        fn_46c602(&g_4ac158);
        fn_46c602(&g_4ac160);
        fn_46c602(&g_4ac164);
        fn_46c602(&g_4ac168);
        fn_46c602(&g_4ac16c);
        fn_46c602(&g_4ac154);
        fn_46c602(&g_4ac158);
        useAltSnoids(1);
        fn_46bee9(saved);
        fn_46ca9c(&g_4abebc);
        fadeOutViews();
        fn_4624fc();
    }
}

/* Told of a Zoombini's script's events on the puzzle. */
/* @zoombi32 0x004276d0 */
void fn_4276d0(View *view, short event)
{
    Snoid *snoid = viewSnoid(view);
    View *other;

    switch (event) {
    case 250:
    case 251:
    case 252:
    case 253:
        setSnoidFacing(snoid, event - 250);
        break;
    case 240:
    case 241:
    case 242:
    case 243:
        g_4ac13e = event - 239;
        break;
    case 0:
        snoid->unknownF2 = !snoid->unknownF2;
        if (g_4ac13e) {
            setSnoidFacing(snoid, g_4ac13e - 1);
            g_4ac13e = 0;
        }
        break;
    case 15:
        moveView(g_4ac136, 0, g_4ac0bc);
        other = findView(g_4ac136);
        if (other)
            other->flags |= 0x4008000;
        view->body.clipped = 1;
        view->body.clip = g_4ac514;
        break;
    case -1:
        other = findView(g_4ac136);
        if (other) {
            Point place;
            short script;

            if (g_4ac0d8 < 3)
                place.x = g_4a1788[g_4ac0ec].x - 23;
            else
                place.x = g_4a17f0[g_4ac0ec].x - 15;
            if (g_4ac0d8 != 3) {
                place.y = g_4ac0ec / 5 * 5 + 410;
                script = g_4ac0ec + 14000;
            } else {
                short row = g_4ac0ec % 25;

                row = row / 5;
                place.y = g_4ac0ec / 25 * 5 + 410;
                script = g_4ac0ec % 5 + row * 5 + 14025;
            }
            startSnoidScript(viewSnoid(other), script, &place, 0);
            other->nextUpdate = 0;
            other->notify = fn_4276d0;
            clearWay(place.x);
            g_4ac0ce = 0;
        }
        break;
    }
}
