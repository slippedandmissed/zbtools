/*
 * lilly (0x424274-0x42f920): 'Lilly.MHK'; 46 KB, so probably several modules
 */

#include "zoombinis.h"

/* A lilly actor's view body (flag 2, a large body). Partly known. */
struct LillyActor
{
    ViewBody body;
    char unknownBc[6];
    char unknownC2;
    char column; /* +0xc3 */
    char row; /* +0xc4 */
    char unknownC5[17];
    char unknownD6;
    char unknownD7[9];
    char unknownE0; /* +0xe0: added to its second part's image */
    char unknownE1[2];
    char unknownE3; /* +0xe3: added to its third part's image */
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
        g_4acff4[actor->row][actor->column + 1].unknown8 = 0;
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
        g_4acff4[actor->row][actor->column + 1].unknown8 = 0;
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
