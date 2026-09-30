/*
 * lilly (0x424274-0x42f920): 'Lilly.MHK'; 46 KB, so probably several modules
 */

#include <stdio.h>
#include <stdlib.h>

#include "zoombinis.h"
#include "basecamp.h"
#include "debug.h"
#include "e2memory.h"
#include "events.h"
#include "features.h"
#include "focus.h"
#include "graphics.h"
#include "lilly.h"
#include "module_4623b8.h"
#include "net.h"
#include "platform.h"
#include "random.h"
#include "snoids.h"
#include "sound.h"
#include "view.h"

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
void roomViewNotify(View *view, short event)
{
    switch (event) {
    case 10:
        view->flags |= 0x20000L;
    }
}

/* @zoombi32 0x0042c10b */
void lillyNoDraw(View *)
{
}

/* @zoombi32 0x0042c112 */
void lillyNoUpdate(View *, short)
{
}

/* @zoombi32 0x0042c6cb */
void setLillyStage(short value)
{
    lillyStage = value;
}

/* @zoombi32 0x0042e693 */
short lillyClaimIndex()
{
    return lillyClaim;
}

/* @zoombi32 0x0042e69a */
void releaseLillyClaim()
{
    placeClaims[lillyClaim] = 0;
    lillyClaim = 0;
}

/* @zoombi32 0x0042d64c */
void freeResourcePair(long *resources)
{
    freeResource(resources);
    freeResource(resources + 1);
}

/* @zoombi32 0x0042b258 */
short lillyKey(unsigned short event)
{
    switch (event) {
    case 367:
        replayHint();
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
        freeResource(resource);
        *handle = 0;
        *resource = 0;
    }
}

/* The sound a Zoombini makes (by its feet) for `which` (0 or 1). */
/* @zoombi32 0x004278d9 */
short footSound(View *view, short which)
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
void updateLillyBackdrop(View *view, short region)
{
    ShortRect unused;

    if (!dialogFlags && view->reset) {
        view->reset = 0;
        unionRgnRect(region, &lillyArea);
    }
}

/* @zoombi32 0x004275dc */
void startView9002(short n)
{
    short which = n % 5;

    startView(room9002Views[n], which + 9002, 0, 0);
}

/* @zoombi32 0x00427610 */
void startView9007(short n)
{
    short which = n % 5;

    startView(room9007Views[n], which + 9007, 0, 0);
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
void deleteHotelTalker()
{
    deleteView(hotelTalkerView);
    hotelTalkerView = 0;
    if (soundOn && lastViewSound) {
        stopSounds(lastViewSound, RESOURCE_TYPE(0, 'S', 'N', 'D'));
        lastViewSound = 0;
    }
}

/* @zoombi32 0x0042afbe */
void placeByHotSpot(View *view)
{
    short *cel = (short *)view->body.cels;

    if (*cel) {
        *cel += cel[10];
        short image = *cel;

        cel++;
        *cel -= actorHotSpotsX[image];
        cel++;
        *cel -= actorHotSpotsY[image];
    }
}

/* Unlocks and releases `count` resources held locked. */
/* @zoombi32 0x0042bd19 */
void freeLockedResources(long *resources, short *handles, short count)
{
    for (short i = 0; i < count; i++)
        if (handles[i]) {
            unlockHandle(handles[i]);
            freeResource(&resources[i]);
            handles[i] = 0;
            resources[i] = 0;
        }
}

/* @zoombi32 0x0042f49d */
void hopperNotify(View *view, short event)
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
            other->placed = placeActorCels3;
            other->notify = hopperNotify;
        }
        break;
    case 20:
        planQueue[planQueueCount] = view->id;
        planQueueCount++;
        break;
    case 25:
        break;
    }
}

/* Sets column n % 5 of roomRowValues to `b` and row n / 5 of roomLayerValues to `a`. */
/* Not exact: register allocation (the original keeps n in esi and n / 5 in edi). */
/* @zoombi32 0x00426a92 */
void setRowAndColumn(short b, short a, short n)
{
    short row;
    short first;
    short column;

    row = n / 5;
    first = n - n % 5;
    column = n - row * 5;
    for (short i = 0; i < 5; i++) {
        roomRowValues[column + i * 5] = b;
        roomLayerValues[first + i] = a;
    }
}

/* @zoombi32 0x0042756d */
void setRowLayerColumn(short a, short b, short c, short n)
{
    short column = n % 5;
    short layer = n / 25;
    short row = n % 25;

    row /= 5;
    roomRowValues[row] = a;
    roomLayerValues[layer] = b;
    roomColumnValues[column] = c;
}

/* @zoombi32 0x00428140 */
void startRoomAnimations()
{
    short ids[15] = {0, 1, 2, 3, 4, 7, 10, 11, 12, 13, 14, 20, 22, 23, 24};

    if (!roomsFilled) {
        for (short i = 0; i < 15; i++)
            startView(roomDoorViews[ids[i]], ids[i] + 6013, 0, 0);
        roomAnimStage = 3;
        queueViewSound(7047, 0);
    }
}

/* @zoombi32 0x0042b7e7 */
void setLillyLevel(short level)
{
    unusedLillyLevel = 0;
    switch (level) {
    case 1:
        lillyLevelParam = 0;
        startCount = 0;
        break;
    case 2:
        lillyLevelParam = 4;
        startCount = 0;
        break;
    case 3:
        lillyLevelParam = 5;
        startCount = 2;
        break;
    case 4:
        lillyLevelParam = 6;
        startCount = 3;
        break;
    }
}

/* @zoombi32 0x004249e1 */
void updateHotelButtons(View *view, short region)
{
    if (hotelGoReady) {
        if (!hotelButton2Lit) {
            hotelButton2Lit = 1;
            unionRgnRect(region, &hotelButtons[2].rect);
        }
    } else if (hotelButton2Lit) {
        hotelButton2Lit = 0;
        unionRgnRect(region, &hotelButtons[2].rect);
    }
    if (!hotelButton1Drawn) {
        hotelButton1Drawn = 1;
        unionRgnRect(region, &hotelButtons[1].rect);
    }
}

/* @zoombi32 0x00428c45 */
void updateLillyButtons(View *view, short region)
{
    if (lillyGoReady) {
        if (!lillyButton2Lit) {
            lillyButton2Lit = 1;
            unionRgnRect(region, &lillyButtons[2].rect);
        }
    } else if (lillyButton2Lit) {
        lillyButton2Lit = 0;
        unionRgnRect(region, &lillyButtons[2].rect);
    }
    if (!lillyButton1Drawn) {
        lillyButton1Drawn = 1;
        unionRgnRect(region, &lillyButtons[1].rect);
    }
}

/* @zoombi32 0x0042b003 */
void lillyActorNotify(View *view, short event)
{
    View *self = view;

    switch (event) {
    case 1:
        setLillyStage(5);
        snoidsOnTheirWay--;
        if (snoidsOnTheirWay < 0)
            snoidsOnTheirWay = 0;
        break;
    case 2:
        event2Views[event2Count] = self->id;
        event2Count++;
        break;
    case 3:
        event3Views[event3Count] = self->id;
        event3Count++;
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
    loadResourceAs(resource, RESOURCE_TYPE('R', 'E', 'G', 'S'), id, 0, 1);
    *handle = usedResourceHandle(*resource);
    *locked = (short *)lockHandle(*handle);
    at = (short *)handleData(*handle);
    for (unsigned long size = handleSize(*handle); size; size -= 2) {
        *at = swapShort(*at);
        at++;
    }
}

/* @zoombi32 0x0042a7b6 */
void lillyNotify60(View *view, short event)
{
    ViewBody *body = &view->body;
    LillyActor *actor = (LillyActor *)&view->body;

    switch (event) {
    case 60:
        actor->body.x = body->cels[0].x;
        actor->body.y = body->cels[0].y;
        actor->body.unknownAa = body->cels[0].x;
        actor->body.unknownAc = body->cels[0].y;
        lillyBoard[actor->row][actor->column + 1].attributes[0] = 0;
        event60Views[event60Count] = view->id;
        event60Count++;
        jumperBusy = 0;
        break;
    }
}

/* @zoombi32 0x0042adb5 */
void lillyNotify49(View *view, short event)
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
        landedJumper = view->id;
        break;
    }
}

/* @zoombi32 0x00427644 */
void startSnoidInRoom(short id)
{
    View *view = findView(id);

    if (view) {
        short script;

        if (hotelLevel < 3)
            script = hotelRoom + 13000;
        else
            script = hotelRoom % 5 + 13025;
        startSnoidScript(viewSnoid(view), script, 0, 0);
        view->notifyEnd = 1;
        view->notify = hotelSnoidNotify;
        view->interval = 3;
        roomGroup = groupViews(roomColumnViews[hotelRoom], view->id, 0, 0, 0, 0);
    }
}

/* Shows the view `id` at the board's square (row, column). */
/* @zoombi32 0x0042e4b6 */
void showViewOnSquare(short id, short row, short column)
{
    View *view = findView(id);

    if (view) {
        view->body.running = 1;
        view->body.bounds.left = lillyBoard[row][column].rect.left;
        view->body.bounds.top = lillyBoard[row][column].rect.top;
        view->body.bounds.right = view->body.bounds.left + 20;
        view->body.bounds.bottom = view->body.bounds.top + 15;
    }
}

/* Not exact: the original keeps `region` in esi. */
/* @zoombi32 0x0042c9aa */
void updateMarkerView(View *view, short region)
{
    ShortRect rect;

    if (!dialogFlags) {
        if (view->reset) {
            view->reset = 0;
            view->nextUpdate = 0;
            view->body.running = 0;
            unionRgnRect(region, &markerArea);
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
void drawHotelButton(short which, short lit, short show)
{
    short image = 0;

    switch (which) {
    case 1:
        image = 5;
        break;
    case 2:
        image = 2;
        if (!hotelGoReady) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        drawImageData((unsigned short *)((char *)hotelButtonImages + hotelButtonImages->offsets[image]), hotelButtons[which].rect.left,
                      hotelButtons[which].rect.top, 8);
        if (show)
            showRect(&hotelButtons[which].rect);
    }
}

/* Draws button `which` (1, 2) of the other set, lit or not, and shows it if asked. */
/* @zoombi32 0x00428b8f */
void drawLillyButton(short which, short lit, short show)
{
    short image = 0;

    switch (which) {
    case 1:
        image = 5;
        break;
    case 2:
        image = 2;
        if (!lillyGoReady) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        drawImageData((unsigned short *)((char *)lillyButtonImages + lillyButtonImages->offsets[image]), lillyButtons[which].rect.left,
                      lillyButtons[which].rect.top, 8);
        if (show)
            showRect(&lillyButtons[which].rect);
    }
}

/* Functional: the original reads each cel's words as it pushes them
   (`drawImageData(image(*cel++), *cel++, *cel++, 8)`, relying on BCC's
   left-to-right evaluation); this reads them first. */
/* @zoombi32-functional 0x00426f38 */
void drawRoomView(View *view)
{
    if (view->body.running) {
        short *cel = (short *)view->body.cels;

        if (hotelLevel == 2) {
            while (*cel) {
                unsigned short *image = (unsigned short *)(roomImages->offsets[*cel++] + (char *)roomImages);
                short x = *cel++;
                short y = *cel++;

                drawImageData(image, x, y, 8);
            }
        } else if (hotelLevel == 3) {
            while (*cel) {
                unsigned short *image = (unsigned short *)(roomImages3d->offsets[*cel++] + (char *)roomImages3d);
                short x = *cel++;
                short y = *cel++;

                drawImageData(image, x, y, 8);
            }
        }
    }
}

/* @zoombi32 0x00427e34 */
void startRoomColumnViews()
{
    short i;

    if (!hotelLevel) {
        for (i = 4; i < roomCount; i += 5) {
            startView(roomColumnViews[i], i + 6063, 0, 0);
            View *view = findView(roomColumnViews[i]);

            view->notify = roomViewNotify;
            view->interval = 1;
        }
    } else if (hotelLevel <= 2) {
        for (i = 0; i < roomCount; i++) {
            startView(roomColumnViews[i], i + 6063, 0, 0);
            View *view = findView(roomColumnViews[i]);

            view->notify = roomViewNotify;
            view->interval = 1;
        }
    }
}

/* @zoombi32 0x0042e6b5 */
void checkLillyArrivals()
{
    short count = padsArrived;

    for (short i = 0; i < actorCount; i++) {
        View *view = findView(actorViews[i]);

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
    if (count < lillyPartySize)
        if (randomBetween(0, 4) > lillyLevel - 1 || (*(short *)(gameState + 0x34) & 0xfff) <= 3)
            queueViewSound(randomBetween(20045, 20048), 0);
}

/* Not exact: the original keeps `region` in esi. */
/* @zoombi32 0x0042c306 */
void updateSquareHighlight(View *view, short region)
{
    if (!dialogFlags) {
        if (view->reset) {
            view->body.running = 0;
            view->nextUpdate = 0;
            view->reset = 0;
            unionRgnRect(region, &markerArea);
        }
        if (view->body.running) {
            LillyCell *cell = &lillyBoard[cursorRow][cursorColumn];

            cursorRect.left = cell->rect.left - 18;
            cursorRect.top = cell->rect.top - 15;
            cursorRect.right = cell->rect.right - 17;
            cursorRect.bottom = cell->rect.bottom - 14;
            unionRgnRect(region, &cursorRect);
        }
    }
}

/* Places a lilly actor's cels by their hot spots, its second part's image
   offset by unknownE0. */
/* @zoombi32 0x0042f3ed */
void placeActorCels(View *view)
{
    short *cel = (short *)&view->body;
    LillyActor *actor = (LillyActor *)&view->body;
    short image;

    image = *cel++;
    *cel++ -= actorHotSpotsX[image];
    *cel++ -= actorHotSpotsY[image];
    if (*cel > 0) {
        *cel += actor->unknownE0;
        image = *cel++;
        *cel++ -= actorHotSpotsX[image];
        *cel++ -= actorHotSpotsY[image];
        while (*cel) {
            image = *cel++;
            *cel++ -= actorHotSpotsX[image];
            *cel++ -= actorHotSpotsY[image];
        }
    }
}

/* The same, the second part's image offset by unknownE0 - 7. */
/* @zoombi32 0x0042f336 */
void placeActorCelsLow(View *view)
{
    short *cel = (short *)&view->body;
    LillyActor *actor = (LillyActor *)&view->body;
    short image;

    image = *cel++;
    *cel++ -= actorHotSpotsX[image];
    *cel++ -= actorHotSpotsY[image];
    if (*cel > 0) {
        *cel = actor->unknownE0 + *cel - 7;
        image = *cel++;
        *cel++ -= actorHotSpotsX[image];
        *cel++ -= actorHotSpotsY[image];
        while (*cel) {
            image = *cel++;
            *cel++ -= actorHotSpotsX[image];
            *cel++ -= actorHotSpotsY[image];
        }
    }
}

/* The same for three parts, the second and third offset by unknownE0 and
   unknownE3. */
/* @zoombi32 0x0042f192 */
void placeActorCels3(View *view)
{
    short *cel = (short *)&view->body;
    LillyActor *actor = (LillyActor *)&view->body;
    short image;

    image = *cel++;
    if (image) {
        *cel++ -= actorHotSpotsX[image];
        *cel++ -= actorHotSpotsY[image];
    }
    if (*cel > 0) {
        *cel += actor->unknownE0;
        image = *cel++;
        *cel++ -= actorHotSpotsX[image];
        *cel++ -= actorHotSpotsY[image];
        if (*cel > 0) {
            *cel += actor->unknownE3;
            image = *cel++;
            *cel++ -= actorHotSpotsX[image];
            *cel++ -= actorHotSpotsY[image];
        }
    }
}

/* @zoombi32 0x0042aaba */
void lillyNotify54(View *view, short event)
{
    ViewBody *body = &view->body;
    LillyActor *actor = (LillyActor *)&view->body;

    switch (event) {
    case 54:
        actor->body.x = body->cels[0].x;
        actor->body.y = body->cels[0].y;
        actor->body.unknownAa = body->cels[0].x;
        actor->body.unknownAc = body->cels[0].y;
        lillyBoard[actor->row][actor->column + 1].attributes[0] = 0;
        finishedLander = landerBusy;
        for (short i = 0; i < 13; i++)
            if (actorViews[i] == finishedLander) {
                for (; actorViews[i]; i++)
                    actorViews[i] = actorViews[i + 1];
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

        loadResourceAs(&resources[i], RESOURCE_TYPE('S', 'C', 'R', 'B'), i + 10000L, 0, 1);
        handles[i] = usedResourceHandle(resources[i]);
        lockHandleAlias(handles[i]);
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

    if (!hotelLevel)
        percent = 88;
    else if (hotelLevel == 2)
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
void lillyNotify44(View *view, short event)
{
    ViewBody *body = &view->body;
    LillyActor *actor = (LillyActor *)&view->body;

    switch (event) {
    case 44:
        actor->body.x = actorHotSpotsX[body->cels[0].image] + body->cels[0].x;
        actor->body.y = actorHotSpotsY[body->cels[0].image] + body->cels[0].y;
        actor->body.unknownAa = actorHotSpotsX[body->cels[0].image] + body->cels[0].x;
        actor->body.unknownAc = actorHotSpotsY[body->cels[0].image] + body->cels[0].y;
        lillyBoard[actor->row][actor->column + 1].attributes[0] = 0;
        event44Views[event44Count] = view->id;
        event44Count++;
        break;
    }
}

/* Counts how many different values of each feature the chosen Zoombinis have. */
/* @zoombi32 0x00426cef */
void countFeatureValues()
{
    short counts[4][6];

    hotelValueCounts[0] = 0;
    hotelValueCounts[1] = 0;
    hotelValueCounts[2] = 0;
    hotelValueCounts[3] = 0;
    hotelChosen = listChosenSnoids();
    hotelPartySize = hotelChosen->count;
    fillMemory(counts, 0, sizeof counts);
    for (short i = 0; i < hotelPartySize; i++)
        for (short j = 0; j < 4; j++)
            counts[j][hotelChosen->features[i][j]]++;
    for (short feature = 0; feature < 4; feature++)
        for (short value = 1; value < 6; value++)
            if (counts[feature][value])
                hotelValueCounts[feature]++;
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
void placeActorCelsHidden(View *view)
{
    short *cel = (short *)&view->body;
    LillyActor *actor = (LillyActor *)&view->body;
    short shown = !cel[25];
    short image;

    image = *cel++;
    if (image) {
        *cel++ -= actorHotSpotsX[image];
        *cel++ -= actorHotSpotsY[image];
    }
    while (*cel) {
        if (*cel == 0x110 && shown) {
            *cel += actor->unknownE0;
            image = *cel++;
            *cel++ -= actorHotSpotsX[image];
            *cel++ -= actorHotSpotsY[image];
        } else if (!shown) {
            *cel++ = 0;
            cel++;
            cel++;
        } else {
            image = *cel++;
            *cel++ -= actorHotSpotsX[image];
            *cel++ -= actorHotSpotsY[image];
        }
    }
}

/* @zoombi32 0x0042a077 */
void lillyNotify70(View *view, short event)
{
    ViewBody *body = &view->body;
    LillyActor *actor = (LillyActor *)&view->body;

    switch (event) {
    case 70:
        actor->body.x = body->cels[0].x;
        actor->body.y = body->cels[0].y;
        actor->body.unknownAa = body->cels[0].x;
        actor->body.unknownAc = body->cels[0].y;
        hopperQueue[hopperQueueCount] = view->id;
        hopperQueueCount++;
        break;
    case 80:
        lillyBoard[actor->row][actor->column].attributes[0] = 0;
        event80Views[event80Count] = view->id;
        event80Count++;
        for (short i = 0; i < actorsOutCount; i++)
            if (actorsOut[i] == view->id) {
                for (; actorsOut[i]; i++)
                    actorsOut[i] = actorsOut[i + 1];
                actorsOutCount--;
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
    ShortRect rect = viewIdBoxRect;
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
    for (short i = 0; i < hotelPartySize; i++) {
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
short fitsRoom(short a, short b, short n)
{
    short i;

    if (hotelAnyFits)
        return 1;
    if (!roomRowValues[n] && !roomLayerValues[n]) {
        for (i = 0; i < roomCount; i++) {
            if (b == roomLayerValues[i])
                return 0;
            if (a == roomRowValues[i])
                return 0;
        }
        return 1;
    }
    if (a == roomRowValues[n] && b == roomLayerValues[n])
        return 1;
    if (roomRowValues[n] && a != roomRowValues[n])
        return 0;
    if (roomLayerValues[n] && b != roomLayerValues[n])
        return 0;
    if (a == roomRowValues[n] && !roomLayerValues[n]) {
        for (i = 0; i < roomCount; i++)
            if (b == roomLayerValues[i])
                return 0;
        return 1;
    }
    if (b == roomLayerValues[n] && !roomRowValues[n]) {
        for (i = 0; i < roomCount; i++)
            if (a == roomRowValues[i])
                return 0;
        return 1;
    }
    return 0;
}

/* Places the view `id` on the board's square (row, column), by the
   square's image's hot spot. */
/* @zoombi32 0x0042e542 */
void placeViewOnSquare(short id, short row, short column)
{
    View *view = findView(id);

    if (view) {
        view->body.running = 0;
        view->body.bounds.left = lillyBoard[row][column].rect.left - squareHotSpotsX[lillyBoard[row][column].attributes[2] + 1];
        view->body.bounds.top = lillyBoard[row][column].rect.top - squareHotSpotsY[lillyBoard[row][column].attributes[2] + 1];
        view->body.bounds.right = lillyBoard[row][column].rect.right - squareHotSpotsX[lillyBoard[row][column].attributes[2] + 1];
        view->body.bounds.bottom = lillyBoard[row][column].rect.bottom - squareHotSpotsY[lillyBoard[row][column].attributes[2] + 1];
        unionRgnRect(removedRgn, &view->body.bounds);
    }
}

/* Draws the board's square (row, column): its image (offset by `offset`)
   and its overlay, by their hot spots. */
/* @zoombi32 0x0042c3b6 */
void drawSquareImage(short row, short column, char offset)
{
    LillyCell *cell = &lillyBoard[row][column];
    short image = squareImageBase[lillyBoard[row][column].attributes[2]] + offset;
    unsigned short *data;
    short x;
    short y;

    if (image > 0 && image < 36) {
        data = (unsigned short *)((char *)lillyImages + lillyImages->offsets[image]);
        x = cell->rect.left - squareHotSpotsX[lillyBoard[row][column].attributes[2] + 1];
        y = cell->rect.top - squareHotSpotsY[lillyBoard[row][column].attributes[2] + 1];
        drawImageData(data, x, y, 8);
    }
    image = lillyBoard[row][column].attributes[4];
    if (image > 0 && image < 36) {
        data = (unsigned short *)((char *)lillyImages + lillyImages->offsets[image]);
        x = cell->rect.left - squareHotSpotsX[lillyBoard[row][column].attributes[4]];
        y = cell->rect.top - squareHotSpotsY[lillyBoard[row][column].attributes[4]];
        drawImageData(data, x, y, 8);
    }
}

/* Moves a lilly actor down a row if it can: the script to run next, or 0. */
/* Not exact: register allocation (the original keeps the view in edx and
   the actor in eax). */
/* @zoombi32 0x0042f7a5 */
short moveActorDown(View *view)
{
    short *body = (short *)&view->body;
    LillyActor *actor = (LillyActor *)&view->body;
    short blocked;
    char row;
    char column;

    if (body[26]) {
        replanQueue[replanQueueCount] = view->id;
        replanQueueCount++;
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
        if (!lillyBoard[row][column].attributes[0]) {
            actor->unknownDe = lillyStarts[0].attribute;
            actor->unknownDf = lillyBoard[row][column].attributes[actor->unknownDe];
            actor->unknownE0 = squareSetC[attributeImageIndex[actor->unknownDe]] + actor->unknownDf;
            actor->grid[row][column] = 1;
            body[26] = 1;
        } else if (squareClaims[row][column] != 1 && !squareClaims[row][column]) {
            return 0;
        }
    }
    if (blocked)
        return actor->unknownD9 = 10069;
    lillyBoard[row][column].attributes[0] = 1;
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
    drawText(Rect(left), 0x22, &names[rowSortFeature * 2], 0xffff);
    drawText(Rect(whole), 0x22, &names[columnSortFeature * 2], 0xffff);
    if (hotelLevel == 3)
        drawText(Rect(right), 0x22, &names[layerSortFeature * 2], 0xffff);
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
        view->placed = placeActorCels3;
        LillyActor *added = (LillyActor *)&view->body;

        added->body.running = 0;
    }
    return id;
}

/* @zoombi32 0x0042ae14 */
void lillyNotify30(View *view, short event)
{
    ViewBody *body = &view->body;
    LillyActor *actor = (LillyActor *)&view->body;

    switch (event) {
    case 30: {
        actor->body.x = actorHotSpotsX[body->cels[0].image] + body->cels[0].x;
        actor->body.y = actorHotSpotsY[body->cels[0].image] + body->cels[0].y;
        actor->body.unknownAa = actorHotSpotsX[body->cels[0].image] + body->cels[0].x;
        actor->body.unknownAc = actorHotSpotsY[body->cels[0].image] + body->cels[0].y;
        firstArrivals++;
        if (firstArrivals == 1)
            lillyGoReady = 1;
        actor->unknownDb++;
        actor->unknownD6 = 0;
        if (actor->unknownDb == 2) {
            landerQueue[landerQueueCount] = view->id;
            landerQueueCount++;
        } else {
            jumperQueue[jumperQueueCount] = view->id;
            jumperQueueCount++;
        }
        lillyBoard[actor->row][actor->column].attributes[0] = 0;
        body->cels[2].image = 0;
        moveView(actor->unknownE1, 0, lillyLayerView3);
        View *other = findView(actor->unknownE1);

        if (other) {
            other->body.running = 1;
            setViewScript(other, actor->row + 10129, 1);
            other->placed = placeByHotSpot;
            other->notify = lillyActorNotify;
            short *parts = (short *)&other->body;
            View *rider = findView(parts[13]);

            if (rider) {
                padsArrived++;
                viewSnoid(rider)->unknownF7 = 1;
            }
        }
        if (padsArrived == lillyPartySize)
            queueViewSound(randomBetween(20055, 20063), 0);
        break;
    }
    }
}

/* Draws the board's square (row, column): its image and its overlay. */
/* @zoombi32 0x0042bd71 */
void drawSquare(short row, short column)
{
    short image = lillyBoard[row][column].attributes[2] + 1;
    unsigned short *data;
    short x;
    short y;

    if (image > 0 && image < 22) {
        data = (unsigned short *)((char *)lillyImages + lillyImages->offsets[image]);
        x = lillyBoard[row][column].rect.left - squareHotSpotsX[lillyBoard[row][column].attributes[2] + 1];
        y = lillyBoard[row][column].rect.top - squareHotSpotsY[lillyBoard[row][column].attributes[2] + 1];
        drawImageData(data, x, y, 8);
    }
    image = lillyBoard[row][column].attributes[4];
    if (image > 0 && image < 22) {
        data = (unsigned short *)((char *)lillyImages + lillyImages->offsets[image]);
        x = lillyBoard[row][column].rect.left - squareHotSpotsX[lillyBoard[row][column].attributes[4]];
        y = lillyBoard[row][column].rect.top - squareHotSpotsY[lillyBoard[row][column].attributes[4]];
        drawImageData(data, x, y, 8);
    }
}

/* Draws the whole board. */
/* @zoombi32 0x0042bf1b */
void drawBoard(View *)
{
    short image;
    unsigned short *data;
    short x;
    short y;

    for (short row = 0; row < 12; row++)
        for (short column = 0; column < 12; column++) {
            image = lillyBoard[row][column].attributes[2] + 1;
            if (image > 0 && image < 22) {
                data = (unsigned short *)((char *)lillyImages + lillyImages->offsets[image]);
                x = lillyBoard[row][column].rect.left - squareHotSpotsX[lillyBoard[row][column].attributes[2] + 1];
                y = lillyBoard[row][column].rect.top - squareHotSpotsY[lillyBoard[row][column].attributes[2] + 1];
                drawImageData(data, x, y, 8);
            }
            image = lillyBoard[row][column].attributes[4];
            if (image > 0 && image < 22) {
                data = (unsigned short *)((char *)lillyImages + lillyImages->offsets[image]);
                x = lillyBoard[row][column].rect.left - squareHotSpotsX[lillyBoard[row][column].attributes[4]];
                y = lillyBoard[row][column].rect.top - squareHotSpotsY[lillyBoard[row][column].attributes[4]];
                drawImageData(data, x, y, 8);
            }
        }
}

/* Draws the square under the cursor, its image animating. */
/* @zoombi32 0x0042c119 */
void drawCursorSquare(View *view)
{
    if (view->body.running) {
        LillyCell *cell = &lillyBoard[cursorRow][cursorColumn];
        short image = cursorImageBase[lillyBoard[cursorRow][cursorColumn].attributes[2]] + cursorFrameOffsets[cursorSquareFrame];
        unsigned short *data;
        short x;
        short y;

        if (image > 0 && image < 36) {
            data = (unsigned short *)((char *)lillyImages + lillyImages->offsets[image]);
            x = cell->rect.left - squareHotSpotsX[lillyBoard[cursorRow][cursorColumn].attributes[2] + 1];
            y = cell->rect.top - squareHotSpotsY[lillyBoard[cursorRow][cursorColumn].attributes[2] + 1];
            drawImageData(data, x, y, 8);
        }
        image = lillyBoard[cursorRow][cursorColumn].attributes[4];
        if (image > 0 && image < 36) {
            data = (unsigned short *)((char *)lillyImages + lillyImages->offsets[image]);
            x = cell->rect.left - squareHotSpotsX[lillyBoard[cursorRow][cursorColumn].attributes[4]];
            y = cell->rect.top - squareHotSpotsY[lillyBoard[cursorRow][cursorColumn].attributes[4]];
            drawImageData(data, x, y, 8);
        }
        if (clockTime() >= view->nextUpdate) {
            view->nextUpdate = clockTime() + view->interval;
            cursorSquareFrame++;
            if (cursorSquareFrame > 3)
                cursorSquareFrame = 0;
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

    if (layerSearches[layer].marks[row][column])
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
            if (open && lillyBoard[r - 1][c].attributes[attribute] == layer && !layerSearches[layer].marks[r][c]) {
                searchQueue[searchQueueEnd].x = c;
                searchQueue[searchQueueEnd].y = r;
                searchQueueEnd++;
                layerSearches[layer].ways[r][c] = back;
                layerSearches[layer].steps[r][c] = layerSearches[layer].steps[row][column] + 1;
                layerSearches[layer].marks[r][c] = layerSearches[layer].marks[row][column];
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
        *cel++ -= actorHotSpotsX[image];
        *cel++ -= actorHotSpotsY[image];
        *cel += actor->unknownE0;
        image = *cel++;
        *cel++ -= actorHotSpotsX[image];
        *cel -= actorHotSpotsY[image];
        break;
    case 3:
    case 4:
        cel = (short *)&view->body;
        image = *cel++;
        *cel++ = actor->body.x + actor->stepX - actorHotSpotsX[image];
        *cel++ = actor->body.y + actor->stepY - actorHotSpotsY[image];
        *cel += actor->unknownE0;
        image = *cel++;
        *cel++ = actor->body.x + actor->stepX - actorHotSpotsX[image];
        *cel = actor->body.y + actor->stepY - actorHotSpotsY[image];
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
        *cel++ = actor->targetX - actorHotSpotsX[image];
        *cel++ = actor->targetY - actorHotSpotsY[image];
        *cel += actor->unknownE0;
        image = *cel++;
        *cel++ = actor->targetX - actorHotSpotsX[image];
        *cel = actor->targetY - actorHotSpotsY[image];
        break;
    }
}

/*
 * Deals out the twelve squares' contents: three sets (3, 4 and 5 entries,
 * copied from squareSetA, squareSetB and squareSetC), each square taking one of
 * its set's remaining entries at random.
 */
/* Not exact: register allocation (the original caches squareSets's address
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
        squareSetA3[i] = squareSetA[i];
        squareSetB3[i] = squareSetB[i];
        squareSetC3[i] = squareSetC[i];
    }
    for (i = 3; i < 7; i++) {
        squareSetA4[i - 3] = squareSetA[i];
        squareSetB4[i - 3] = squareSetB[i];
        squareSetC4[i - 3] = squareSetC[i];
    }
    for (i = 7; i < 12; i++) {
        squareSetA5[i - 7] = squareSetA[i];
        squareSetB5[i - 7] = squareSetB[i];
        squareSetC5[i - 7] = squareSetC[i];
    }
    left1 = 2;
    left2 = 3;
    left3 = 4;
    for (short n = 1; n < 13; n++) {
        switch (n) {
        case 1:
        case 2:
        case 3:
            squareSets[0] = squareSetA3;
            squareSets[1] = squareSetB3;
            squareSets[2] = squareSetC3;
            left = &left1;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            squareSets[0] = squareSetA4;
            squareSets[1] = squareSetB4;
            squareSets[2] = squareSetC4;
            left = &left2;
            break;
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
            squareSets[0] = squareSetA5;
            squareSets[1] = squareSetB5;
            squareSets[2] = squareSetC5;
            left = &left3;
            break;
        }
        short k = randomBetween(0, *left);

        squareDeals[n].a = squareSets[0][k];
        squareDeals[n].b = squareSets[1][k];
        squareDeals[n].c = squareSets[2][k];
        for (; k < *left + 1; k++) {
            squareSets[0][k] = squareSets[0][k + 1];
            squareSets[1][k] = squareSets[1][k + 1];
            squareSets[2][k] = squareSets[2][k + 1];
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
                if (hotelLevel == 2) {
                    image = (unsigned short *)((char *)roomImages + roomImages->offsets[word]);
                    *cel++ = rect.left = offsetX + *at++ - lillyViewHotX[word];
                    *cel++ = rect.right = offsetY + *at++ - lillyViewHotY[word];
                } else if (hotelLevel == 3) {
                    image = (unsigned short *)((char *)roomImages3d + roomImages3d->offsets[word]);
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
        freeResource(&resource);
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

/* The same, jumping to its place in jumpPlaces (by unknownBe). */
/* @zoombi32 0x0042ab6f */
void placeJumperAt(View *view)
{
    LillyActor *actor = (LillyActor *)&view->body;
    short *cel;
    short image;

    switch (actor->body.frame) {
    case 0:
        actor->targetX = jumpPlaces[actor->unknownBe].x;
        actor->targetY = jumpPlaces[actor->unknownBe].y;
        actor->stepX = (actor->targetX - actor->body.x) / 3;
        actor->stepY = (actor->targetY - actor->body.y) / 3;
        /* fall through */
    case 1:
    case 2:
        cel = (short *)&view->body;
        image = *cel++;
        *cel++ -= actorHotSpotsX[image];
        *cel++ -= actorHotSpotsY[image];
        *cel += actor->unknownE0;
        image = *cel++;
        *cel++ -= actorHotSpotsX[image];
        *cel -= actorHotSpotsY[image];
        break;
    case 3:
    case 4:
        cel = (short *)&view->body;
        image = *cel++;
        *cel++ = actor->body.x + actor->stepX - actorHotSpotsX[image];
        *cel++ = actor->body.y + actor->stepY - actorHotSpotsY[image];
        *cel += actor->unknownE0;
        image = *cel++;
        *cel++ = actor->body.x + actor->stepX - actorHotSpotsX[image];
        *cel = actor->body.y + actor->stepY - actorHotSpotsY[image];
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
        *cel++ = actor->targetX - actorHotSpotsX[image];
        *cel++ = actor->targetY - actorHotSpotsY[image];
        *cel += actor->unknownE0;
        image = *cel++;
        *cel++ = actor->targetX - actorHotSpotsX[image];
        *cel = actor->targetY - actorHotSpotsY[image];
        break;
    }
}

/* Adds the lilly actors (actorCount of them), each dealt a random entry of
   actorDealKinds/actorDealValues. */
/* @zoombi32 0x0042b857 */
void addLillyActors()
{
    LillyActor actor;
    short i;

    actorDealLast = 11;
    for (i = 0; i < actorDealLast + 1; i++)
        actorDealOrder[i] = i;
    for (i = 0; i < actorCount; i++) {
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
        short k = randomBetween(0, actorDealLast);

        actor.unknownDe = actorDealKinds[actorDealOrder[k]];
        actor.unknownDf = actorDealValues[actorDealOrder[k]];
        actor.unknownE0 = actorDealOrder[k];
        dealtKinds[i] = actor.unknownDe;
        dealtValues[i] = actor.unknownDf;
        for (; k < actorDealLast + 1; k++)
            actorDealOrder[k] = actorDealOrder[k + 1];
        actorDealLast--;
        actorViews[i] = addView(0x180002, drawCels, runViewScript, i + 10043, 7, &actor, randomBetween(3, 6), 0);
        View *view = findView(actorViews[i]);

        if (view) {
            short *parts = (short *)&view->body;

            parts[20] = i;
            setViewScript(view, i + 10043, 1);
            view->flags = 0x980002;
            view->placed = placeActorCels3;
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
        *cel++ -= actorHotSpotsX[image];
        *cel++ -= actorHotSpotsY[image];
        *cel += actor->unknownE0;
        image = *cel++;
        *cel++ -= actorHotSpotsX[image];
        *cel -= actorHotSpotsY[image];
        break;
    case 3:
    case 4:
        cel = (short *)&view->body;
        image = *cel++;
        *cel++ = actor->body.x + actor->stepX - actorHotSpotsX[image];
        *cel++ = actor->body.y + actor->stepY - actorHotSpotsY[image];
        *cel += actor->unknownE0;
        image = *cel++;
        *cel++ = actor->body.x + actor->stepX - actorHotSpotsX[image];
        *cel = actor->body.y + actor->stepY - actorHotSpotsY[image];
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
        *cel++ = actor->targetX - actorHotSpotsX[image];
        *cel++ = actor->targetY - actorHotSpotsY[image];
        if (*cel == 0x5b) {
            *cel += actor->unknownE0;
            image = *cel++;
            *cel++ = actor->targetX - actorHotSpotsX[image];
            *cel++ = actor->targetY - actorHotSpotsY[image];
        }
        while (*cel) {
            image = *cel++;
            *cel++ = actor->targetX - actorHotSpotsX[image];
            *cel++ = actor->targetY - actorHotSpotsY[image];
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
short pickNextSquare(View *view)
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
        if (open && !lillyBoard[r - 1][c].attributes[0]
            && lillyBoard[r - 1][c].attributes[actor->unknownDe] == actor->unknownDf
            && layerSearches[actor->unknownDf].steps[r][c] < layerSearches[actor->unknownDf].steps[actor->row + 1][actor->column]) {
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
        lillyBoard[bestRow][bestColumn].attributes[0] = 1;
        return actor->unknownD9 = 10071;
    case 1:
        actor->unknownD5 = 1;
        lillyBoard[bestRow][bestColumn].attributes[0] = 1;
        return actor->unknownD9 = 10077;
    case 2:
        actor->unknownD5 = 2;
        lillyBoard[bestRow][bestColumn].attributes[0] = 1;
        return actor->unknownD9 = 10073;
    case 3:
        actor->unknownD5 = 3;
        lillyBoard[bestRow][bestColumn].attributes[0] = 1;
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
            actor->targetX = rowLeft[--actor->row + 1] + actor->column * 35;
            break;
        case 2:
            actor->targetX = rowLeft[++actor->row + 1] + actor->column * 35;
            break;
        case 1:
            actor->targetX = rowLeft[actor->row + 1] + ++actor->column * 35;
            break;
        case 3:
            actor->targetX = rowLeft[actor->row + 1] + --actor->column * 35;
            break;
        }
        actor->targetY = rowTop[actor->row + 1] + columnDy[actor->column];
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
        lillyBoard[actor->unknownC6][actor->unknownC5].attributes[0] = 0;
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
        planQueue[planQueueCount] = view->id;
        planQueueCount++;
        view->nextUpdate = clockTime() + 30;
        break;
    case 15:
        planQueue[planQueueCount] = view->id;
        planQueueCount++;
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
short pickLeastVisited(LillyActor *actor)
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
            if (!lillyBoard[r][c].attributes[0]) {
                switch (actor->unknownDe) {
                case 1:
                    if (lillyBoard[r][c].attributes[1] != actor->unknownDf)
                        open = 0;
                    break;
                case 2:
                    if (lillyBoard[r][c].attributes[2] != actor->unknownDf)
                        open = 0;
                    break;
                case 3:
                    if (lillyBoard[r][c].attributes[3] != actor->unknownDf)
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
        if (!lillyBoard[r][c + 1].attributes[0]) {
            best = 4;
            lillyBoard[r][c + 1].attributes[0] = 1;
        } else {
            best = 5;
        }
    }
    switch (best) {
    case 0:
        actor->unknownD5 = 0;
        actor->unknownD9 = hopDirections[0][actor->unknownD5];
        actor->grid[actor->row - 1][actor->column] = current + 1;
        lillyBoard[bestRow][bestColumn].attributes[0] = 1;
        return actor->unknownD9;
    case 1:
        actor->unknownD5 = 1;
        actor->unknownD9 = hopDirections[1][actor->unknownD5];
        actor->grid[actor->row][actor->column + 1] = current + 1;
        lillyBoard[bestRow][bestColumn].attributes[0] = 1;
        return actor->unknownD9;
    case 2:
        actor->unknownD5 = 2;
        actor->unknownD9 = hopDirections[2][actor->unknownD5];
        actor->grid[actor->row + 1][actor->column] = current + 1;
        lillyBoard[bestRow][bestColumn].attributes[0] = 1;
        return actor->unknownD9;
    case 3:
        actor->unknownD5 = 3;
        actor->unknownD9 = hopDirections[3][actor->unknownD5];
        actor->grid[actor->row][actor->column - 1] = current + 1;
        lillyBoard[bestRow][bestColumn].attributes[0] = 1;
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
                    if (lillyBoard[r][c].attributes[1] != actor->unknownDf)
                        open = 0;
                    break;
                case 2:
                    if (lillyBoard[r][c].attributes[2] != actor->unknownDf)
                        open = 0;
                    break;
                case 3:
                    if (lillyBoard[r][c].attributes[3] != actor->unknownDf)
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
            actor->targetX = rowLeft[--actor->row + 1] + actor->column * 35;
            break;
        case 10073:
            actor->targetX = rowLeft[++actor->row + 1] + actor->column * 35;
            break;
        case 10077:
            actor->targetX = rowLeft[actor->row + 1] + ++actor->column * 35;
            break;
        case 10075:
            actor->targetX = rowLeft[actor->row + 1] + --actor->column * 35;
            break;
        }
        actor->targetY = rowTop[actor->row + 1] + columnDy[actor->column];
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
            lillyBoard[actor->unknownC6][actor->unknownC5].attributes[0] = 0;
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
        hopperQueue[hopperQueueCount] = view->id;
        hopperQueueCount++;
        break;
    }
    image = *cel++;
    if (image) {
        *cel++ = x - actorHotSpotsX[image];
        *cel++ = y - actorHotSpotsY[image];
    }
    if (*cel == 0x110 && shown) {
        *cel += actor->unknownE0;
        image = *cel++;
        *cel++ = x - actorHotSpotsX[image];
        *cel++ = y - actorHotSpotsY[image];
    } else if (!shown) {
        *cel++ = 0;
        cel++;
        cel++;
    }
    while (*cel) {
        image = *cel++;
        *cel++ = x - actorHotSpotsX[image];
        *cel++ = y - actorHotSpotsY[image];
    }
}

/* Whether a, b and c fit square n of the 5 by 5 by 5 puzzle: its row,
   layer and column sort by them, or can, the values being unused elsewhere. */
/* @zoombi32 0x00427217 */
short fitsRoom3d(short a, short b, short c, short n)
{
    short column;
    short layer;
    short row;
    short i;

    if (hotelAnyFits)
        return 1;
    column = n % 5;
    layer = n / 25;
    row = n % 25;
    row = row / 5;
    if (!roomRowValues[row] && !roomLayerValues[layer] && !roomColumnValues[column]) {
        for (i = 0; i < 5; i++) {
            if (b == roomLayerValues[i])
                return 0;
            if (a == roomRowValues[i])
                return 0;
            if (c == roomColumnValues[i])
                return 0;
        }
        return 1;
    }
    if (!roomRowValues[row])
        for (i = 0; i < 5; i++)
            if (a == roomRowValues[i])
                return 0;
    if (!roomLayerValues[layer])
        for (i = 0; i < 5; i++)
            if (b == roomLayerValues[i])
                return 0;
    if (!roomColumnValues[column])
        for (i = 0; i < 5; i++)
            if (c == roomColumnValues[i])
                return 0;
    if (roomColumnValues[column] && c != roomColumnValues[column])
        return 0;
    if (roomRowValues[row] && a != roomRowValues[row])
        return 0;
    if (roomLayerValues[layer] && b != roomLayerValues[layer])
        return 0;
    if (a == roomRowValues[row] && c == roomColumnValues[column] && b == roomLayerValues[layer])
        return 1;
    if (a == roomRowValues[row] && !roomLayerValues[layer])
        for (i = 0; i < 5; i++)
            if (b == roomLayerValues[i])
                return 0;
    if (b == roomLayerValues[layer] && !roomRowValues[row])
        for (i = 0; i < 5; i++)
            if (a == roomRowValues[i])
                return 0;
    if (a == roomRowValues[row] && !roomColumnValues[column])
        for (i = 0; i < 5; i++)
            if (c == roomColumnValues[i])
                return 0;
    if (b == roomLayerValues[layer] && !roomColumnValues[column])
        for (i = 0; i < 5; i++)
            if (c == roomColumnValues[i])
                return 0;
    if (c == roomColumnValues[column] && !roomLayerValues[layer])
        for (i = 0; i < 5; i++)
            if (b == roomLayerValues[i])
                return 0;
    if (c == roomColumnValues[column] && !roomRowValues[row])
        for (i = 0; i < 5; i++)
            if (a == roomRowValues[i])
                return 0;
    return 1;
}

/*
 * Sends the Zoombini `id` onto square hotelRoom: works out where it stands
 * (standX/standY) and the area it covers (standArea), and starts its
 * script (by the level, the square and its feet).
 */
/* Not exact: register allocation (the original keeps `lift` on the stack
   and the column in ecx). */
/* @zoombi32 0x0042790a */
void sendSnoidToRoom(short id)
{
    Point place;
    short extra;
    short lift;
    short script;
    View *view = findView(id);

    if (hotelLevel != 3)
        place = roomPlaces[hotelRoom];
    else
        place = roomPlaces3d[hotelRoom];
    if (hotelLevel != 3) {
        place.x += 24;
        place.y -= 7;
        standY = place.y - 2;
        if (!hotelLevel) {
            if (hotelRoom == 4)
                standX = place.x - 5;
            else if (hotelRoom == 9)
                standX = place.x - 7;
            else if (hotelRoom == 14)
                standX = place.x - 3;
            else if (hotelRoom == 19)
                standX = place.x - 3;
            else if (hotelRoom == 24)
                standX = place.x - 3;
        } else {
            if (hotelRoom <= 4)
                standX = place.x - 8;
            else if (hotelRoom <= 9)
                standX = place.x - 6;
            else if (hotelRoom <= 14)
                standX = place.x - 5;
            else if (hotelRoom <= 19)
                standX = place.x - 4;
            else if (hotelRoom <= 24)
                standX = place.x - 5;
        }
        if (!roomOccupancy[hotelRoom] || roomOccupancy[hotelRoom] % 3 == 1) {
            standX -= 8;
            standY = roomOccupancy[hotelRoom] + standY - 1;
        } else if (roomOccupancy[hotelRoom] % 3 == 2) {
            standX--;
            standY = roomOccupancy[hotelRoom] + standY - 1;
        } else {
            standX += 6;
            standY = roomOccupancy[hotelRoom] + standY - 1;
        }
    } else {
        place.x += 5;
        place.y -= 15;
        standX = place.x;
        standY = place.y - 2;
        if (roomOccupancy[hotelRoom] > 1) {
            standX -= (roomOccupancy[hotelRoom] - 1) * 2;
            standY -= roomOccupancy[hotelRoom] - 1;
        }
    }
    if (view) {
        switch (hotelLevel) {
        case 0:
        case 1:
        case 2:
            if (hotelRoom < 10) {
                script = 13030;
                lift = 3;
            } else if (hotelRoom < 15) {
                script = 13035;
                lift = 0;
            } else {
                script = 13040;
                lift = 5;
            }
            standArea.left = roomPlaces[hotelRoom].x - 16;
            standArea.top = roomPlaces[hotelRoom].y - 30;
            standArea.right = standArea.left + 52;
            standArea.bottom = standArea.top + 82;
            if (hotelLevel == 1 || hotelLevel == 2) {
                short column = hotelRoom % 5;

                if (column <= 2)
                    standArea.top -= lift;
            }
            break;
        case 3: {
            script = hotelRoom % 5 * 5 + 13045;
            standArea.left = roomColumnX[hotelRoom / 5 + 1] + roomColumnDx[hotelRoom % 5];
            standArea.top = roomColumnY[hotelRoom / 5 + 1] + roomColumnDy[hotelRoom % 5];
            standArea.right = standArea.left + 22;
            standArea.bottom = standArea.top + 72;
            short row = hotelRoom % 25 / 5;

            if (hotelRoom % 25 == 1)
                standArea.left += 5;
            else if (hotelRoom % 25 == 3)
                standArea.left += 3;
            else
                standArea.left += 4;
            if (hotelRoom % 5 >= 3) {
                extra = 1;
                if (hotelRoom % 5 == 3) {
                    standArea.left--;
                    extra++;
                }
                standArea.left += extra;
                standArea.right += extra;
            }
            if (hotelRoom % 5 == 4)
                standArea.left--;
            else if (hotelRoom % 5 == 3 && row)
                standArea.left--;
            break;
        }
        }
        script = script + viewSnoid(view)->features[3] - 1;
        if (hotelLevel <= 2)
            startSnoidScript(viewSnoid(view), script, 0, 0);
        else
            startSnoidScript(viewSnoid(view), script, &place, 0);
        view->notify = hotelSnoidNotify;
        view->notifyEnd = 0;
        arrivingSnoid = view->id;
        arrivingGroup = groupViews(arrivingSnoid, arrivingSnoid, 0, 0, 0, 0);
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

    if (!hotelLevel) {
        for (i = 4; i < roomCount; i += 5)
            roomDoorViews[i] = addView(0x4188000, drawCels, runViewScript, i + 6013, 6, 0, 0, 0);
        roomAnchorView = addView(0x4008000, drawCels, runViewScript, 11504, 6, 0, 0, 0);
        for (i = 4; i < roomCount; i += 5)
            roomColumnViews[i] = addView(0xc188000, drawCels, runViewScript, i + 6038, 3, 0, 0, 0);
    } else if (hotelLevel < 3) {
        for (i = 0; i < roomCount; i++)
            roomDoorViews[i] = addView(0x4188000, drawCels, runViewScript, i + 6013, 6, 0, 0, 0);
        if (hotelLevel == 2) {
            count = 0;
            for (i = 0; i < roomCount; i++)
                if (roomOccupancy[i] == -1) {
                    place.x = roomViewX[i + 1];
                    place.y = roomViewY[i + 1];
                    deleteView(roomViews[i]);
                    roomViews[i] = addView(0x808000, drawRoomView, layOutLillyView, roomViewScripts[count++] + 11004, 0, &place, 0, 0);
                }
        }
        roomAnchorView = addView(0x4008000, drawCels, runViewScript, 11503, 6, 0, 0, 0);
        for (i = 0; i < roomCount; i++)
            roomColumnViews[i] = addView(0xc188000, drawCels, runViewScript, i + 6038, 3, 0, 0, 0);
    } else {
        for (i = 0; i < roomCount; i++) {
            short column = i % 5;
            short row = i / 5 + 1;
            short offsetX = roomColumnDx[column];

            offsetY = roomColumnDy[column];
            place.x = offsetX + roomColumnX[row];
            place.y = roomColumnY[row] + offsetY;
            room9002Views[i] = addView(0x4988000, drawCels, runViewScript, column + 9002, 6, &place, 0, 0);
        }
        roomAnchorView = addView(0x4008000, drawCels, runViewScript, 11505, 6, 0, 0, 0);
        for (i = 0; i < roomCount; i++) {
            short column = i % 5;
            short row = i / 5 + 1;
            short offsetX = offsetsX[column];

            offsetY = offsetsY[column];
            place.x = offsetX + layerRowX[row];
            place.y = layerRowY[row] + offsetY;
            room9007Views[i] = addView(0xc988000, drawCels, runViewScript, column + 9007, 3, &place, 0, 0);
        }
        count = 0;
        for (i = 0; i < roomCount; i++)
            if (roomOccupancy[i] == -1) {
                short column = i % 5;
                short row = i / 5 + 1;
                short offsetX = offsetsX[column];

                offsetY = offsetsY[column];
                place.x = offsetX + roomColumnX[row];
                place.y = roomColumnY[row] + offsetY;
                if (!column)
                    place.y += 5;
                if (column == 2)
                    place.y += 2;
                roomViews[i] = addView(0x808000, drawRoomView, layOutLillyView, roomViewScripts[count++] + 12000, 0, &place, 0, 0);
            }
    }
    if (!hotelLevel) {
        placeSnapRadius += 10;
        for (i = 4; i < roomCount; i += 5)
            placedViews[(i - 4) / 5] = addView(0x508a000, drawCels, runViewScript, i + 10000, 7, &roomPlaces[i], 0, 0);
    } else if (hotelLevel < 3) {
        placeSnapRadius += 10;
        for (i = 0; i < roomCount; i++)
            placedViews[i] = addView(0x508a000, drawCels, runViewScript, i + 10000, 7, &roomPlaces[i], 0, 0);
    } else if (hotelLevel == 3) {
        placeSnapRadius = 10;
        for (i = 0; i < roomCount; i++)
            placedViews[i] = addView(0x108a000, drawCels, runViewScript, i + 10025, 6, &roomPlaces3d[i], 0, 0);
    }
}

/* Draws both buttons, unlit. */
/* @zoombi32 0x004249c4 */
void drawHotelButtons(View *)
{
    drawHotelButton(1, 0, 0);
    drawHotelButton(2, 0, 0);
}

/* Draws the other set's buttons, unlit. */
/* @zoombi32 0x00428c28 */
void drawLillyButtons(View *)
{
    drawLillyButton(1, 0, 0);
    drawLillyButton(2, 0, 0);
}

/* Flashes square (flashRow, flashColumn) until flashCount reaches lillyStage. */
/* @zoombi32 0x0042c52f */
void flashSquare(View *view)
{
    if (view->body.running) {
        if (flashCount >= lillyStage) {
            view->body.running = 0;
            unionRgnRect(removedRgn, &lillyBoard[flashRow][flashColumn].rect);
        } else {
            if (clockTime() >= view->nextUpdate) {
                view->nextUpdate = clockTime() + view->interval;
                flashFrame++;
                if (flashFrame > 1)
                    flashFrame = 0;
            }
            drawSquareImage(flashRow, flashColumn, flashFrame);
        }
    }
}

/* The puzzle's keys (debugging ones only while debugging messages are on). */
/* @zoombi32 0x00426831 */
short hotelKey(unsigned short key)
{
    if (!debugMessagesOn && key != 367)
        return 0;
    switch (key) {
    case 367:
        replayHint();
        return 1;
    case 'A':
    case 'a':
        drawFeatureLabels();
        return 1;
    case 'H':
    case 'h':
        if (!roomAnimStage)
            roomAnimStage = 1;
        return 1;
    case 'I':
    case 'i':
        if (!roomAnimStage)
            roomAnimStage = 0;
        else
            roomAnimStage++;
        return 1;
    case 'R':
        hotelAnyFits = 1;
        return 1;
    case 'W':
    case 'w': {
        if (++debugTalkerScript1 > 9)
            debugTalkerScript1 = 0;
        View *view = findView(hotelTalkerView);

        if (view) {
            setViewScript(view, debugTalkerScript1 + 7000, 1);
        } else {
            hotelTalkerView = addView(0x8108000, drawCels, runViewScript, debugTalkerScript1 + 7000, 6, 0, 0, 0);
            view = findView(hotelTalkerView);
        }
        return 1;
    }
    case 'E':
    case 'e': {
        if (++debugTalkerScript2 > 17)
            debugTalkerScript2 = 10;
        View *view = findView(hotelTalkerView);

        if (view) {
            setViewScript(view, debugTalkerScript2 + 7000, 1);
        } else {
            hotelTalkerView = addView(0x8108000, drawCels, runViewScript, debugTalkerScript2 + 7000, 6, 0, 0, 0);
            view = findView(hotelTalkerView);
        }
        return 1;
    }
    case ' ':
        guideStep = savedGuideStep;
        guideLastStep = savedGuideLastStep;
        if (hotelLevel != 3)
            startView(guideView, guideStep + 6000, 0, 0);
        fadePalette(savedPalette, 10, 236, 0, 0, 0);
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
void closeLilly()
{
    if (lillyOpen) {
        lillyOpen = 0;
        short saved = setFreeAtOnce(1);

        requestViewSort();
        setSnoidsRunning(1);
        clearViews();
        freeResource(&unusedLillyResource);
        freeResource(&lillyImagesResource);
        freeLockedResources(lillyScriptResources, lillyScriptHandles, 91);
        freeResourcePair(rowPlaceResources);
        freeResourcePair(actorHotSpotResources);
        freeResourcePair(squareHotSpotResources);
        freeLockedResource(&grid1Resource, &grid1Handle);
        freeLockedResource(&grid2Resource, &grid2Handle);
        freeLockedResource(&grid3Resource, &grid3Handle);
        freeResource(&lillyButtonResource);
        unloadSounds();
        setFreeAtOnce(saved);
        closeGameFile(&lillyFile);
        fadeOutViews();
        showBusyCursor();
    }
}

/* Closes the puzzle. */
/* @zoombi32 0x00424a53 */
void closeHotel()
{
    if (hotelOpen) {
        hotelOpen = 0;
        short saved = setFreeAtOnce(1);

        clearViews();
        unloadSounds();
        freeResource(&hotelButtonResource);
        freeResource(&roomImagesResource);
        freeResource(&hotelPlaceXResource);
        freeResource(&hotelPlaceYResource);
        freeResource(&lillyViewHotXResource);
        freeResource(&lillyViewHotYResource);
        freeResource(&roomViewXResource);
        freeResource(&roomViewYResource);
        freeResource(&roomColumnXResource);
        freeResource(&roomColumnYResource);
        freeResource(&layerRowXResource);
        freeResource(&layerRowYResource);
        freeResource(&roomViewXResource);
        freeResource(&roomViewYResource);
        useAltSnoids(1);
        setFreeAtOnce(saved);
        closeGameFile(&hotelFile);
        fadeOutViews();
        showBusyCursor();
    }
}

/*
 * The puzzle's idle work, once per pass: ends it when asked, and otherwise
 * moves the guide's scenes along one step (each waits for its group of
 * views to finish): setting the board up again when a round starts, the
 * lily pad count, and a Zoombini's reaction after it is dropped on a square
 * (idleSnoidView): hopping on to the pad, or being sent back.
 */
/* @zoombi32 0x00424b2d */
void hotelFrame()
{
    short sound;
    short backdrop;
    basePort *saved;
    View *view;
    View *other;

    if (inHotelFrame || !hotelOpen)
        return;
    inHotelFrame = 1;
    updateViews();
    if (sceneDue) {
        if (isSoundPlaying(996, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
            inHotelFrame = 0;
            return;
        }
        if (!dialogQuestion || dialogQuestion == 3) {
            if (dialogQuestion == 3)
                chooseSnoids(0, 0);
            if (viewsLocked || !snoidsOnTheirWay) {
                pendingScene = sceneDue;
                sceneDue = 0;
                setCurrentMap(0);
                closeHotel();
                inHotelFrame = 0;
                return;
            }
        } else if (dialogQuestion == 2) {
            dialogQuestion = 0;
            sceneDue = 0;
        }
    }
    if (guideRemarkGroup) {
        if (!groupLeader[guideRemarkGroup]) {
            guideRemarkGroup = 0;
            sound = randomUpTo(3) + 7503;
            if (hotelLevel != 3 && hotelFails) {
                if (!startView(hotelTalkerView, sound, 0, 0)) {
                    hotelTalkerView = addView(0x8108000, drawCels, runViewScript, sound, 6, 0, 0, 0);
                    talkerDoneGroup = groupViews(hotelTalkerView, hotelTalkerView, 0, 0, 0, 0);
                }
                loadViewSounds(hotelTalkerView, 1);
            }
        }
    } else if (talkerDoneGroup) {
        if (!groupLeader[talkerDoneGroup]) {
            talkerDoneGroup = 0;
            hotelGoReady = 1;
        }
    } else if (roundResetGroup) {
        hotelIdleSince = clockTime();
        if (!groupLeader[roundResetGroup] || skipGuide) {
            roundResetGroup = 0;
            if (hotelTalkerView && talkerStage != 4)
                deleteHotelTalker();
            backdrop = 1;
            if (hotelLevel >= 3)
                backdrop++;
            chooseSnoids(1, 0);
            removeDeadViews();
            useAltSnoids(1);
            deleteView(hotelLabelView);
            deleteView(view11800);
            hotelTalkerView = hotelLabelView = 0;
            drawBackdrop(backdrop + 5000);
            saved = getPort();
            setPort(viewPort);
            if (hotelLevel == 0) {
                fillPortRect(Rect(hotelBlankRect1), Color(0x25), 0);
                fillPortRect(Rect(hotelBlankRect2), Color(0x25), 0);
            } else if (hotelLevel <= 2) {
                fillPortRect(Rect(hotelBlankRect3), Color(0x25), 0);
            } else if (hotelLevel == 3) {
                fillPortRect(Rect(hotelBlankRect4), Color(0x25), 0);
            }
            setPort(saved);
            setViewPlaces(16, lillyPlaces, 1);
            makePartySnoids(0);
            addView(0x1000, drawHotelButtons, updateHotelButtons, 0, 0, 0, 0, 0);
            addLillyViews();
            if (hotelLevel != 3) {
                hotelIdleSince = clockTime();
                if (skipGuide) {
                    roundResetGroup = talkerPending = 0;
                    talkerGroup = talkerStarted = skipGuide = 0;
                    if (hotelTalkerView)
                        deleteHotelTalker();
                    guideView = addView(0x108000, drawCels, runViewScript, guideLastStep + 6000, 6, 0, 0, 0);
                    queueViewSound(hotelLevel + 30020, 0);
                } else if (!talkerStage || talkerStage == 4 && talkerPending) {
                    hotelTalkerView = addView(0x8188000, drawCels, runViewScript, hotelLevel + 7500, 6, 0, 0, 0);
                    talkerGroup = groupViews(hotelTalkerView, hotelTalkerView, 0, 0, 0, 0);
                } else if (talkerStage > 4 && !talkerPending) {
                    if (hotelTalkerView)
                        deleteHotelTalker();
                    hotelTalkerView = addView(0x8108000, drawCels, runViewScript, hotelLevel + 7500, 6, 0, 0, 0);
                    loadViewSounds(hotelTalkerView, 1);
                    talkerGroup = groupViews(hotelTalkerView, hotelTalkerView, 0, 0, 0, 0);
                } else {
                    talkerGroup = talkerStarted = skipGuide = 0;
                    guideView = addView(0x108000, drawCels, runViewScript, 6000, 6, 0, 0, 0);
                    guideStepGroup = groupViews(guideView, guideView, 0, 0, 0, 0);
                    queueViewSound(hotelLevel + 30020, 0);
                }
            } else {
                talkerGroup = talkerStarted = skipGuide = 0;
                if (hotelTalkerView)
                    deleteHotelTalker();
                queueViewSound(hotelLevel + 30020, 0);
            }
            chooseSnoids(0, 0);
            drawHotelButton(1, 0, 0);
            drawHotelButton(2, 0, 0);
            updateViews();
            showRect(&shownGameRect);
            resetViewClock();
        }
    } else if ((!talkerStage || talkerStage == 4) && talkerPending && !skipGuide) {
        if (clockTime() - hotelIdleSince > 180) {
            talkerPending = 0;
            startView(hotelTalkerView, hotelLevel + 7500, 0, 0);
            loadViewSounds(hotelTalkerView, 1);
            talkerGroup = groupViews(hotelTalkerView, hotelTalkerView, 0, 0, 0, 0);
        } else if (!talkerStage) {
            talkerPending = 0;
            startView(hotelTalkerView, hotelLevel + 7500, 0, 0);
            loadViewSounds(hotelTalkerView, 1);
            talkerGroup = groupViews(hotelTalkerView, hotelTalkerView, 0, 0, 0, 0);
        }
    } else if (talkerGroup) {
        if (!groupLeader[talkerGroup] || skipGuide) {
            talkerGroup = talkerStarted = 0;
            queueViewSound(hotelLevel + 30020, 0);
            if (skipGuide) {
                skipGuide = 0;
                if (hotelTalkerView)
                    deleteHotelTalker();
                if (hotelLevel != 3)
                    guideView = addView(0x108000, drawCels, runViewScript, guideLastStep + 6000, 6, 0, 0, 0);
            } else if (hotelLevel != 3) {
                guideView = addView(0x108000, drawCels, runViewScript, 6000, 6, 0, 0, 0);
                guideStepGroup = groupViews(guideView, guideView, 0, 0, 0, 0);
            }
        }
    } else if (guideStepGroup) {
        guideStepGroup = 0;
        other = findView(guideView);
        if (guideStep <= guideLastStep) {
            if (!other)
                guideView = addView(0x108000, drawCels, runViewScript, guideStep + 6000, 6, 0, 0, 0);
            else
                startView(guideView, guideStep + 6000, 0, 0);
            guideStepGroup = groupViews(guideView, guideView, 0, 0, 0, 0);
            guideStep++;
        }
    } else if (roomGroup) {
        if (!groupLeader[roomGroup])
            roomGroup = 0;
    } else if (droppedSnoid) {
        view = idleSnoidView(droppedSnoid);
        if (view) {
            droppedSnoid = 0;
            if (viewSnoid(view)->unknownF7) {
                roomsFilled++;
                if (roomOccupancy[hotelRoom] > 0)
                    if (++roomOccupancy[hotelRoom] > 6)
                        roomOccupancy[hotelRoom] = 6;
                sendSnoidToRoom(view->id);
                hotelGoReady = 1;
                if (!roomOccupancy[hotelRoom]) {
                    if (hotelLevel < 3)
                        startView(roomDoorViews[hotelRoom], hotelRoom + 6013, 0, 0);
                    else
                        startView9002(hotelRoom);
                    roomOccupancy[hotelRoom]++;
                }
                hotelWalker = view->id;
            } else {
                snoidRejected = 1;
                if (hotelLevel < 3) {
                    if (guideStep < 11)
                        startView(roomColumnViews[hotelRoom], hotelRoom + 6038, 0, 0);
                    else
                        startRoomColumnViews();
                } else {
                    startView9007(hotelRoom);
                    if (guideStep >= 11 && !*(short *)(gameState + 0x20)) {
                        other = findView(room9007Views[hotelRoom]);
                        other->notify = roomViewNotify;
                    }
                }
                hotelWalker = view->id;
                startSnoidInRoom(hotelWalker);
                if (hotelLevel != 3) {
                    if (!firstPlacementFree) {
                        startView(guideView, ++guideStep + 6000, 0, 0);
                        guideRemarkGroup = groupViews(guideView, guideView, 0, 0, 0, 0);
                    }
                } else {
                    guideStep++;
                }
                darkenPalette();
                if (guideStep == 9) {
                    sound = randomUpTo(2) + 7007;
                    if (!startView(hotelTalkerView, sound, 0, 0)) {
                        hotelTalkerView = addView(0x8108000, drawCels, runViewScript, sound, 6, 0, 0, 0);
                        findView(hotelTalkerView);
                    }
                    loadViewSounds(hotelTalkerView, 1);
                }
                if (guideStep >= 12) {
                    hotelFails++;
                    queueViewSound(6006, 0);
                    updateViews();
                    waitForEventFor(0, 60, 0, 1);
                    if (hotelLevel == 3) {
                        randomUpTo(3);
                        queueViewSound(7500, 0);
                    }
                }
            }
        }
    } else if (arrivingGroup) {
        if (!groupLeader[arrivingGroup]) {
            arrivingGroup = 0;
            other = findView(arrivingSnoid);
            if (other) {
                if (hotelLevel == 3)
                    setSnoidAction(viewSnoid(other), 0, 0);
                else
                    setSnoidAction(viewSnoid(other), 7, 0);
                *(Point *)&viewSnoid(other)->targetX = *(Point *)&standX;
                snoidArriving = 0;
                if (roomsFilled >= hotelPartySize) {
                    if (hotelLevel < 3) {
                        startRoomColumnViews();
                        hotelRoom = 200;
                        other = findView(hotelTalkerView);
                        sound = randomUpTo(2) + 7507;
                        if (other)
                            setViewScript(other, sound, 1);
                        else
                            hotelTalkerView = addView(0x8108000, drawCels, runViewScript, sound, 6, 0, 0, 0);
                        loadViewSounds(hotelTalkerView, 1);
                        talkerDoneGroup = groupViews(hotelTalkerView, hotelTalkerView, 0, 0, 0, 0);
                    } else {
                        hotelRoom = 200;
                    }
                }
            }
        }
    }
    if (roomAnimStage == 2 && hotelLevel == 1)
        startRoomAnimations();
    inHotelFrame = 0;
}

/* Told of a Zoombini's script's events on the puzzle. */
/* @zoombi32 0x004276d0 */
void hotelSnoidNotify(View *view, short event)
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
        hotelFacing = event - 239;
        break;
    case 0:
        snoid->unknownF2 = !snoid->unknownF2;
        if (hotelFacing) {
            setSnoidFacing(snoid, hotelFacing - 1);
            hotelFacing = 0;
        }
        break;
    case 15:
        moveView(hotelWalker, 0, roomAnchorView);
        other = findView(hotelWalker);
        if (other)
            other->flags |= 0x4008000;
        view->body.clipped = 1;
        view->body.clip = standArea;
        break;
    case -1:
        other = findView(hotelWalker);
        if (other) {
            Point place;
            short script;

            if (hotelLevel < 3)
                place.x = roomPlaces[hotelRoom].x - 23;
            else
                place.x = roomPlaces3d[hotelRoom].x - 15;
            if (hotelLevel != 3) {
                place.y = hotelRoom / 5 * 5 + 410;
                script = hotelRoom + 14000;
            } else {
                short row = hotelRoom % 25;

                row = row / 5;
                place.y = hotelRoom / 25 * 5 + 410;
                script = hotelRoom % 5 + row * 5 + 14025;
            }
            startSnoidScript(viewSnoid(other), script, &place, 0);
            other->nextUpdate = 0;
            other->notify = hotelSnoidNotify;
            clearWay(place.x);
            snoidRejected = 0;
        }
        break;
    }
}

/*
 * Searches layer `layer` of the board from the bottom row up: every square
 * whose attribute `attribute` is `layer` is a start, and the search spreads
 * from each (at most 144 squares queued).
 */
/* @zoombi32 0x0042e80b */
void searchLayer(short attribute, short layer)
{
    char unused[24];
    short row;
    short column;

    for (row = 0; row < 13; row++)
        for (column = 0; column < 12; column++) {
            layerSearches[layer].marks[row][column] = 0;
            layerSearches[layer].ways[row][column] = 44;
            layerSearches[layer].steps[row][column] = 0;
        }
    searchQueueEnd = 0;
    searchQueueStart = 0;
    fillMemory(searchQueue, 0, 0x240);
    for (row = 12; row >= 1; row--) {
        for (column = 0; column < 12; column++)
            if (lillyBoard[row - 1][column].attributes[attribute] == layer && !layerSearches[layer].marks[row][column]) {
                if (searchQueueEnd < 144) {
                    searchQueue[searchQueueEnd].x = column;
                    searchQueue[searchQueueEnd].y = row;
                    searchQueueEnd++;
                }
                layerSearches[layer].ways[row][column] = 2;
                layerSearches[layer].steps[row][column]++;
                layerSearches[layer].marks[row][column] = row;
            }
        for (short i = searchQueueStart; i < searchQueueEnd && searchQueueStart < 144 && i < 144; i++)
            if (layerSearches[layer].marks[searchQueue[i].y][searchQueue[i].x]) {
                searchStep(attribute, layer, searchQueue[i].y, searchQueue[i].x);
                searchQueueStart++;
            }
    }
}

/* A layer to search again (by swapSquares). */
struct LayerToSearch
{
    short attribute;
    short layer;
};

/*
 * Swaps the attributes of squares (flashRow, flashColumn) and (swapRow,
 * swapColumn), and replans every actor whose kind either square now has,
 * searching their layers again.
 */
/* @zoombi32 0x0042c6dc */
void swapSquares()
{
    LillyCell *a = &lillyBoard[flashRow][flashColumn];
    LillyCell *b = &lillyBoard[swapRow][swapColumn];
    short first = a->attributes[1];
    short second = a->attributes[2];
    short third = a->attributes[3];
    short fourth = a->attributes[4];
    LayerToSearch layers[5];
    short i;
    LillyActor *actor;

    a->attributes[1] = b->attributes[1];
    a->attributes[2] = b->attributes[2];
    a->attributes[3] = b->attributes[3];
    a->attributes[4] = b->attributes[4];
    b->attributes[1] = first;
    b->attributes[2] = second;
    b->attributes[3] = third;
    b->attributes[4] = fourth;
    for (i = 0; i < actorCount; i++) {
        View *view = findView(actorViews[i]);

        if (view) {
            actor = (LillyActor *)&view->body;
            if (actor->unknownC2) {
                actor->grid[flashRow][flashColumn] = 0;
                actor->grid[swapRow][swapColumn] = 0;
                if (lillyBoard[flashRow][flashColumn].attributes[actor->unknownDe] == actor->unknownDf
                    || lillyBoard[swapRow][swapColumn].attributes[actor->unknownDe] == actor->unknownDf)
                    startPlan(actor);
            }
        }
    }
    mainLoopEvents();
    fillMemory(layers, 0, sizeof layers);
    for (i = 0; i < actorsOutCount; i++) {
        View *view = findView(actorsOut[i]);

        if (view) {
            actor = (LillyActor *)&view->body;
            if (actor->unknownC2) {
                actor->grid[flashRow][flashColumn] = 0;
                actor->grid[swapRow][swapColumn] = 0;
                if (lillyBoard[flashRow][flashColumn].attributes[actor->unknownDe] == actor->unknownDf
                    || lillyBoard[swapRow][swapColumn].attributes[actor->unknownDe] == actor->unknownDf) {
                    startPlan(actor);
                    layers[actor->unknownDf].attribute = actor->unknownDe;
                    layers[actor->unknownDf].layer = actor->unknownDf;
                }
            }
        }
    }
    for (i = 0; i < layersToSearchCount; i++)
        if (layers[i].attribute)
            searchLayer(layers[i].attribute, layers[i].layer);
    mainLoopEvents();
    swapPending = 0;
}

/* Flashes the two squares being swapped, swapping them first. */
/* @zoombi32 0x0042c5d4 */
void flashSwap(View *view)
{
    if (view->body.running) {
        if (!flashCount) {
            swapSquares();
            drawSquareImage(swapRow, swapColumn, 1);
            drawSquareImage(flashRow, flashColumn, 1);
        }
        if (flashCount >= lillyStage) {
            flashCount = 0;
            swapStep = 4;
            view->body.running = 0;
            unionRgnRect(removedRgn, &lillyBoard[swapRow][swapColumn].rect);
        } else {
            if (clockTime() >= view->nextUpdate) {
                view->nextUpdate = clockTime() + view->interval;
                flashCount++;
                swapFlashFrame++;
                if (swapFlashFrame > 1)
                    swapFlashFrame = 0;
            }
            drawSquareImage(swapRow, swapColumn, swapFlashFrame);
        }
    }
}

/* @zoombi32 0x0042b08e */
void lillyViewNotify3(View *view, short event)
{
    View *other;
    short i;

    switch (event) {
    case 3:
        if (lillyLevel > 1)
            for (i = 0; i < lillyPartySize; i++)
                if (i == lillyPartySize - 2 || i == lillyPartySize - 1) {
                    other = findView(snoidPadViews[i]);
                    if (other) {
                        other->body.running = 1;
                        setViewScript(other, i + 10089, 1);
                        other->placed = placeByHotSpot;
                        other->notify = lillyActorNotify;
                    }
                }
        break;
    case 4:
        event4Pad = view->id;
        other = findView(swapToolView);
        if (other) {
            other->flags = 0x980002;
            LillyActor *actor = (LillyActor *)&other->body;

            actor->body.running = 1;
        }
        if (lillyLevel > 2)
            for (i = 0; i < layersToSearchCount; i++)
                searchLayer(lillyStarts[0].attribute, i);
        break;
    case 5:
        switch (swapStep) {
        case 4:
            flashColumn = presetSwapColumns[presetSwapIndex].x;
            flashRow = presetSwapRows[presetSwapIndex].x;
            showViewOnSquare(swapFirstMarker, flashRow, flashColumn);
            swapStep = 5;
            presetSwapIndex++;
            break;
        case 5:
            swapColumn = presetSwapColumns[presetSwapIndex].x;
            swapRow = presetSwapRows[presetSwapIndex].x;
            showViewOnSquare(swapSecondMarker, swapRow, swapColumn);
            swapStep = 6;
            presetSwapIndex++;
            break;
        }
        break;
    case 6:
        break;
    }
}

/*
 * Sets up the puzzle: picks which features its rows, columns (and layers)
 * sort by, so that the chosen Zoombinis fit, and at level 2 places some
 * pieces at random in squares no Zoombini can take.
 */
/* @zoombi32 0x00425dde */
void setUpLillyPuzzle()
{
    short k;
    short usedA[5];
    short usedB[5];
    short second;
    Point place;
    short counts[125];
    short empty[125];
    short ok;
    short i;
    short n;

    countFeatureValues();
    ok = 0;
    do {
        rowSortFeature = randomUpTo(3);
        columnSortFeature = randomUpTo(3);
        layerSortFeature = randomUpTo(3);
        if (hotelLevel < 2) {
            if (hotelValueCounts[rowSortFeature] == 5 && hotelValueCounts[columnSortFeature] == 5 && rowSortFeature != columnSortFeature)
                ok++;
            else if ((hotelValueCounts[0] < 5) + (hotelValueCounts[1] < 5) + (hotelValueCounts[2] < 5) + (hotelValueCounts[3] < 5) >= 3) {
                if (rowSortFeature != columnSortFeature)
                    ok++;
            } else if (rowSortFeature != columnSortFeature && hotelValueCounts[rowSortFeature] >= 4 && hotelValueCounts[columnSortFeature] >= 4)
                ok++;
        } else if (hotelLevel == 2) {
            if ((hotelValueCounts[0] < 4) + (hotelValueCounts[1] < 4) + (hotelValueCounts[2] < 4) + (hotelValueCounts[3] < 4) >= 3) {
                if (rowSortFeature != columnSortFeature)
                    ok++;
            } else if (rowSortFeature != columnSortFeature && hotelValueCounts[rowSortFeature] >= 4 && hotelValueCounts[columnSortFeature] >= 4)
                ok++;
        } else if (hotelLevel == 3) {
            if (rowSortFeature != columnSortFeature && columnSortFeature != layerSortFeature && rowSortFeature != layerSortFeature)
                ok++;
        }
    } while (!ok);
    if (hotelLevel == 2) {
        fillMemory(counts, 0, sizeof counts);
        for (i = 0; i < 5; i++) {
            usedA[i] = 0;
            usedB[i] = 0;
        }
        for (i = 0; i < 5; i++) {
            do
                ok = randomUpTo(4);
            while (usedA[ok]);
            usedA[ok]++;
            do
                k = randomUpTo(4);
            while (usedB[k]);
            usedB[k]++;
            setRowAndColumn(++ok, k + 1, i * 6);
        }
        for (i = 0; i < hotelPartySize; i++) {
            short first = hotelChosen->features[i][rowSortFeature];

            second = hotelChosen->features[i][columnSortFeature];
            for (n = 0; n < 5; n++)
                for (ok = 0; ok < 5; ok++)
                    if (first == roomRowValues[n * 5 + ok] && roomLayerValues[n * 5 + ok] == second)
                        counts[n * 5 + ok]++;
        }
        n = 0;
        for (i = 0; i < 25; i++)
            if (!counts[i])
                empty[n++] = i;
        k = randomUpTo(n - 1) + 1;
        if (k > 8)
            k = 8;
        if (n < k)
            k = n;
        for (i = 0; i < k; i++) {
            do
                ok = randomUpTo(n - 1);
            while (roomOccupancy[empty[ok]] < 0);
            roomOccupancy[empty[ok]] = -1;
            roomViewScripts[i] = randomUpTo(3);
        }
    }
    if (hotelLevel == 2) {
        ok = 0;
        for (i = 0; i < roomCount; i++)
            if (roomOccupancy[i] == -1) {
                place.x = hotelPlaceX[i + 1];
                place.y = hotelPlaceY[i + 1];
                roomViews[i] = addView(0x808000, drawRoomView, layOutLillyView, roomViewScripts[ok++] + 11000, 0, &place, 0, 0);
            }
    }
    fillMemory(roomRowValues, 0, 50);
    fillMemory(roomLayerValues, 0, 50);
}

/* The puzzle's clicks: 1 the leave button, 2 the other button, 3 a
   Zoombini picked up and dropped on a square. */
/* @zoombi32 0x00426230 */
void lillyClick(short action)
{
    ShortRect bounds;
    Point where;
    Point start;
    short third;
    short wrong;
    View *view;
    Snoid *snoid;
    short chosen;
    short i;
    short first;
    short second;

    if (sceneDue) {
        pendingScene = sceneDue;
        sceneDue = 0;
        setCurrentMap(0);
        closeHotel();
        return;
    }
    if (talkerStarted) {
        if (hotelTalkerView) {
            deleteHotelTalker();
            skipGuide++;
        }
        return;
    }
    switch (action) {
    case 1:
        queueViewSound(999, 0);
        drawHotelButton(action, 1, 1);
        waitForEventFor(0, 2, 0, 1);
        drawHotelButton(action, 0, 1);
        sceneDue = 1;
        askKeepParty();
        break;
    case 2:
        if (hotelGoReady) {
            queueViewSound(0, 0);
            drawHotelButton(action, 1, 1);
            waitForEventFor(0, 2, 0, 1);
            drawHotelButton(action, 0, 1);
            queueViewSound(996, 0);
            showBusyCursor();
            sceneDue = 15;
        }
        break;
    case 3:
        if (roundResetGroup || talkerGroup || hotelFails || snoidArriving || snoidsOnTheirWay > 0 || snoidRejected)
            break;
        chosen = 0;
        getCursorPosition(&where);
        view = viewAt(where, 1, 1);
        if (!view)
            view = viewAt(where, 0x8001, 1);
        if (view) {
            snoid = viewSnoid(view);
            chosen = snoid->unknownF7;
            if (snoid->unknownF4 != 9 && snoid->unknownF4 != 8 && snoid->unknownF4 != 7) {
                start = *(Point *)&view->body.x;
                if (chosen) {
                    bounds = snoid->body.bounds;
                    snoid->body.clipped = 0;
                    dragInPlace = 1;
                    placesClaimable = 0;
                    dragSnoid(view, where, 0, 0);
                    placesClaimable = 1;
                    snoid->body.bounds = bounds;
                    snoid->body.clipped = 1;
                    break;
                }
            }
        }
        if (view && !chosen) {
            for (i = 0; i < placedViewCount; i++)
                placeClaims[i] = 0;
            dragSnoid(view, where, 0, 0);
            unloadSounds();
            heldRoomPlace = heldPlaceNumber();
            if (heldRoomPlace > 5 && !hotelLevel)
                heldRoomPlace = 0;
            if (!hotelLevel)
                hotelRoom = (heldRoomPlace - 1) * 5 + 4;
            else
                hotelRoom = heldRoomPlace - 1;
            if (hotelRoom < 0 || roomOccupancy[hotelRoom] < 0)
                heldRoomPlace = 0;
            if (heldRoomPlace) {
                snoidArriving++;
                switch (hotelLevel) {
                case 0:
                    if (firstPlacementFree) {
                        roomRowValues[hotelRoom] = viewSnoid(view)->features[rowSortFeature];
                        wrong = firstPlacementFree = 0;
                        guideStep = guideLastStep;
                    } else {
                        wrong = 0;
                        if (roomRowValues[hotelRoom]) {
                            if (roomRowValues[hotelRoom] != viewSnoid(view)->features[rowSortFeature])
                                wrong = 1;
                            else
                                wrong = 0;
                        } else {
                            for (i = 0; i < 5; i++)
                                if (roomRowValues[i * 5 + 4] == viewSnoid(view)->features[rowSortFeature])
                                    wrong = 1;
                            if (!wrong)
                                roomRowValues[hotelRoom] = viewSnoid(view)->features[rowSortFeature];
                        }
                    }
                    droppedSnoid = view->id;
                    break;
                case 1:
                case 2:
                    if (firstPlacementFree) {
                        first = viewSnoid(view)->features[rowSortFeature];
                        second = viewSnoid(view)->features[columnSortFeature];
                        setRowAndColumn(first, second, hotelRoom);
                        wrong = firstPlacementFree = 0;
                        guideStep = guideLastStep;
                    } else {
                        first = viewSnoid(view)->features[rowSortFeature];
                        second = viewSnoid(view)->features[columnSortFeature];
                        switch (fitsRoom(first, second, hotelRoom)) {
                        case 0:
                            wrong = 1;
                            break;
                        case 1:
                            setRowAndColumn(first, second, hotelRoom);
                            wrong = 0;
                            break;
                        }
                    }
                    droppedSnoid = view->id;
                    break;
                case 3:
                    if (firstPlacementFree) {
                        first = viewSnoid(view)->features[rowSortFeature];
                        second = viewSnoid(view)->features[columnSortFeature];
                        third = viewSnoid(view)->features[layerSortFeature];
                        setRowLayerColumn(first, second, third, hotelRoom);
                        wrong = firstPlacementFree = 0;
                        guideStep = guideLastStep;
                    } else {
                        first = viewSnoid(view)->features[rowSortFeature];
                        second = viewSnoid(view)->features[columnSortFeature];
                        third = viewSnoid(view)->features[layerSortFeature];
                        switch (fitsRoom3d(first, second, third, hotelRoom)) {
                        case 0:
                            wrong = 1;
                            break;
                        case 1:
                            setRowLayerColumn(first, second, third, hotelRoom);
                            wrong = 0;
                            break;
                        }
                    }
                    droppedSnoid = view->id;
                    break;
                }
                if (!wrong) {
                    viewSnoid(view)->unknownF7 = 1;
                    if (countChosenSnoids() == hotelPartySize)
                        queueViewSound(randomUpTo(23) + 175, 0);
                } else {
                    snoidRejected = 1;
                    snoidArriving = 0;
                }
            }
        }
        break;
    }
}

/* Opens the puzzle (Hotel.MHK) at the level reached. */
/* Not exact: register allocation (the original caches hotelLevel's address
   in ebx and keeps `labels` in esi). */
/* @zoombi32 0x00424274 */
void openHotel()
{
    Point places[20] = {{455, 423}, {432, 421}, {412, 420}, {395, 425}, {379, 418}, {365, 433}, {352, 412},
                        {340, 433}, {328, 418}, {314, 432}, {295, 421}, {279, 430}, {264, 437}, {259, 421},
                        {244, 432}, {226, 421}, {211, 427}, {195, 419}, {176, 423}, {158, 431}};

    unloadSounds();
    fillMemory(roomRowValues, 0, 50);
    fillMemory(roomLayerValues, 0, 50);
    fillMemory(roomColumnValues, 0, 10);
    fillMemory(roomOccupancy, 0, 250);
    fillMemory(roomViewScripts, 0, 40);
    droppedSnoid = talkerStage = talkerPending = 0;
    roundResetGroup = talkerGroup = guideStepGroup = talkerDoneGroup = 0;
    rowSortFeature = columnSortFeature = layerSortFeature = guideRemarkGroup = 0;
    snoidRejected = roomGroup = arrivingGroup = arrivingSnoid = 0;
    hotelFails = roomsFilled = skipGuide = 0;
    unusedHotel1 = snoidArriving = 0;
    talkerStarted = 1;
    roomCount = debugTalkerScript1 = debugTalkerScript2 = 25;
    firstPlacementFree = 1;
    claimOnArrival = 0;
    hintSound = roomAnimStage = 0;
    hotelLevel = sceneLevel();
    guideStep = 1;
    switch (hotelLevel) {
    case 0:
        guideLastStep = 5;
        break;
    case 2:
        guideLastStep = 4;
        break;
    default:
        guideLastStep = 2;
        break;
    }
    savedGuideStep = guideStep;
    savedGuideLastStep = guideLastStep;
    if (hotelLevel == 3)
        roomCount = 125;
    sceneDue = hotelAnyFits = 0;
    hotelOpen = hotelGoReady = 0;
    useAltSnoids(0);
    openGameFile(&hotelFile, "Hotel.MHK");
    setCurrentMap(hotelFile);
    loadTerrain(100);
    drawBackdrop(5000);
    setViewPlaces(20, places, 1);
    copyPaletteRange(10, 236);
    if (hotelLevel == 3)
        loadFeatureGroup(9000, 0, 0);
    else
        loadFeatureGroup(6000, 0, 0);
    loadFeatureGroup(7000, 1, 0);
    loadFeatureGroup(10000, 2, 0);
    loadFeatureGroup(11500, 3, 0);
    loadFeatureGroup(11800, 4, 0);
    if (hotelLevel != 3)
        loadFeatureGroup(7500, 5, 0);
    if (hotelLevel == 3)
        loadScripts(9000, 12);
    else
        loadScripts(6000, 88);
    addScripts(7000, 11, 2);
    if (hotelLevel != 3)
        addScripts(10000, 25, 0);
    else
        addScripts(10025, 125, 0);
    addScripts(11500, 6, 0);
    addScripts(11800, 1, 0);
    if (hotelLevel != 3)
        addScripts(7500, 10, 2);
    if (hotelLevel < 3) {
        loadSnoidScripts(14000, 25, 5);
        addSnoidScripts(13000, 70, 5);
    } else {
        loadSnoidScripts(14025, 25, 5);
        addSnoidScripts(13025, 45, 5);
    }
    if (hotelLevel == 2) {
        roomImages = loadImageBank(11000, &roomImagesResource);
        hotelPlaceX = loadShortTable(11000, &hotelPlaceXResource);
        hotelPlaceY = loadShortTable(11001, &hotelPlaceYResource);
        lillyViewHotX = loadShortTable(11002, &lillyViewHotXResource);
        lillyViewHotY = loadShortTable(11003, &lillyViewHotYResource);
        roomViewX = loadShortTable(11004, &roomViewXResource);
        roomViewY = loadShortTable(11005, &roomViewYResource);
    }
    if (hotelLevel == 3) {
        roomImages3d = loadImageBank(12000, &roomImagesResource);
        roomColumnX = loadShortTable(9000, &roomColumnXResource);
        roomColumnY = loadShortTable(9001, &roomColumnYResource);
        layerRowX = loadShortTable(9002, &layerRowXResource);
        layerRowY = loadShortTable(9003, &layerRowYResource);
        roomViewX = loadShortTable(12004, &roomViewXResource);
        roomViewY = loadShortTable(12005, &roomViewYResource);
    }
    hotelButtonImages = loadImageBank(8000, &hotelButtonResource);
    {
        short labels = hotelLevel;

        if (labels >= 2)
            labels--;
        hotelLabelView = addView(0x108000, drawCels, runViewScript, labels + 11500, 6, 0, 0, 0);
    }
    talkerStage = 0;
    campHint((short *)(gameState + 0x3a));
    hintSound = 20081;
    switch (hotelLevel) {
    case 0:
        if ((*(short *)(gameState + 0x3a) & 0xfff) > 1)
            talkerStage = randomUpTo(2) + 1;
        break;
    case 1:
        talkerStage = 4;
        break;
    case 2:
        talkerStage = 5;
        break;
    case 3:
        talkerStage = 6;
        break;
    }
    if (!talkerStage || talkerStage == 4)
        talkerPending = 1;
    setGroupLists(hotelGroups, 1, (short)0xc000);
    addSoundRange(8900, 8901, 0);
    addSoundRange(996, 997, 0);
    addSoundRange(20000, 29999, 1);
    addSoundRange(99, 99, 0);
    addSoundRange(7000, 7999, 1);
    addSoundRange(425, 499, 0);
    addSoundRange(6004, 6006, 0);
    addSoundRange(6000, 6099, 0);
    addSoundRange(9004, 9006, 0);
    addSoundRange(9000, 9999, 0);
    addSoundRange(10000, 10999, 0);
    hotelTalkerView = addView(0x8108000, drawCels, runViewScript, talkerStage + 7000, 6, 0, 0, 0);
    loadViewSounds(hotelTalkerView, 1);
    makePartySnoids(0);
    setUpLillyPuzzle();
    view11800 = addView(0x100000, drawCels, runViewScript, 11800, 6, 0, 0, 0);
    updateViews();
    roundResetGroup = groupViews(hotelTalkerView, hotelTalkerView, 0, 0, 0, 0);
    showRect(&shownGameRect);
    fadeInViews();
    getColors(&savedPalette[10], 10, 236);
    hotelOpen = 1;
}

/* The other puzzle's clicks: 1 the leave button, 2 the other button, 3 a
   piece picked up and dropped on the board. */
/* @zoombi32 0x00429943 */
void otherClick(short action)
{
    View *other;
    ImageBank *bank;
    Point where;
    ShortRect rect;
    View *view;
    LillyActor *actor;
    short *cel;

    if (sceneDue) {
        pendingScene = sceneDue;
        sceneDue = 0;
        setCurrentMap(0);
        closeLilly();
        return;
    }
    switch (action) {
    case 1:
        queueViewSound(999, 0);
        drawLillyButton(action, 1, 1);
        waitForEventFor(0, 2, 0, 1);
        drawLillyButton(action, 0, 1);
        snoidsArrived = 1;
        sceneDue = 1;
        askKeepParty();
        break;
    case 2:
        if (lillyGoReady && !sceneDue) {
            queueViewSound(996, 0);
            drawLillyButton(action, 1, 1);
            waitForEventFor(0, 2, 0, 1);
            drawLillyButton(action, 0, 1);
            sceneDue = 12;
            snoidsOnTheirWay = 0;
            swapPending = 0;
        }
        break;
    case 3:
        getCursorPosition(&where);
        view = viewAt(where, 0x980002, 1);
        if (view && !swapPending) {
            actor = (LillyActor *)&view->body;
            if ((snoidsOnTheirWay <= 0 || (snoidsOnTheirWay > 0 && actor->unknownC0 == 2)) && !actor->unknownC2
                && actor->unknownC0 != 1) {
                dragLillyPiece(view, where);
                actor = (LillyActor *)&view->body;
                cel = (short *)&view->body;

                bank = groupBanks[view->body.scriptGroup];
                claimedRow = lillyClaimIndex();
                if (!actor->unknownC0 && claimedRow >= 0 && claimedRow <= 11) {
                    unionRgnRect(removedRgn, &actor->body.bounds);
                    actor->body.bounds.left = 0;
                    actor->body.bounds.right = 0;
                    actor->body.bounds.top = 0;
                    actor->body.bounds.bottom = 0;
                    actor->body.x = lillyBoard[claimedRow][0].rect.left;
                    actor->body.y = lillyBoard[claimedRow][0].rect.top;
                    while (*cel) {
                        cel[1] = actor->body.x - actorHotSpotsX[*cel];
                        cel[2] = actor->body.y - actorHotSpotsY[*cel];
                        unsigned short *image = (unsigned short *)(bank->offsets[*cel] + (char *)bank);

                        cel++;
                        rect.left = *cel++;
                        rect.right = swapShort(image[0]) + rect.left;
                        rect.top = *cel++;
                        rect.bottom = swapShort(image[1]) + rect.top;
                        unionRect(&view->body.bounds, &rect);
                    }
                    view->nextUpdate = 0;
                    view->changed = 1;
                    releaseLillyClaim();
                    lillyBoard[claimedRow][0].attributes[0] = 1;
                    actor->unknownC2 = 1;
                    actor->column = 0;
                    actor->row = claimedRow;
                    startPlan(actor);
                    moveView(view->id, 0, rowAnchorViews[actor->row]);
                    other = findView(snoidPadViews[padsPlaced]);
                    if (other) {
                        cel = (short *)&other->body;
                        cel[14] = view->id;
                        cel[15] = claimedRow;
                        other->flags = 0x180000;
                        setViewScript(other, cel[12] + 10109, 1);
                        other->placed = placeByHotSpot;
                        other->notify = lillyActorNotify;
                        actor->unknownE1 = snoidPadViews[padsPlaced];
                        padsPlaced++;
                        actor->unknownE3 = *(char *)&cel[10];
                    }
                    if (padsPlaced == lillyPartySize) {
                        if (swapToolStage == 6)
                            checkLillyArrivals();
                        snoidsOnTheirWay = 1;
                    }
                } else if (!actor->unknownC0) {
                    *(Point *)&actor->body.x = jumpPlaces[cel[20]];
                    setViewScript(view, cel[20] + 10043, 1);
                    view->nextUpdate = 0;
                    view->changed = 1;
                    releaseLillyClaim();
                }
            }
        }
        break;
    }
}

/*
 * Drags a piece of the other puzzle: a new piece (unknownC0 0) onto a row
 * of the board's left edge, lighting up the marker there; the swapping
 * tool (else) to two squares in turn, whose contents it then swaps.
 */
/* body and bank are set and never read; the original keeps both stores,
   which only `volatile` reproduces. */
/* @zoombi32 0x0042d9c5 */
void dragLillyPiece(View *piece, Point where0)
{
    short id;
    short lastRow;
    Point start;
    Point where;
    LillyActor *actor;
    View *view;
    View *marker;
    long unused1;
    ShortRect cell;
    ShortRect unused2;
    ShortRect rect;
    unsigned long interval;
    short found;
    short unused3;
    ShortRect unused4;
    ViewBody *volatile body;
    long unused5;
    long unused6;
    ImageBank *volatile bank;
    short hitColumn;
    short hitRow;
    short hit;
    short cancel;
    short row;
    short column;

    lillyClaim = 0;
    id = piece->id;
    if ((marker = findView(cursorSquareView)) != 0 && (view = removeView(id, 0)) != 0) {
        view->id = -3;
        insertViewAtEnd(view);
        view->flags = 0x980002;
        interval = view->interval;
        view->nextUpdate = 0;
        view->interval = 3;
        actor = (LillyActor *)&view->body;
        if (!actor->unknownC0) {
            cancel = 0;
            lillyDragState = 1;
            actor->body.unknownAa = (actor->body.bounds.right - actor->body.bounds.left) / 2 + view->body.cels[0].x;
            actor->body.unknownAc = (actor->body.bounds.bottom - actor->body.bounds.top) / 2 + view->body.cels[0].y;
        } else {
            cancel = 0;
            lillyDragState = 4;
            swapStep = 4;
            start.x = 38;
            start.y = 415;
        }
        cursorColumn = 0;
        cursorRow = -1;
        lastRow = -1;
        if (hideDragCursor)
            hideCursor();
        body = &view->body;
        bank = groupBanks[view->body.scriptGroup];
        while (lillyDragState) {
            if (!actor->unknownC0)
                lillyDragState = keepDragging();
            getCursorPosition(&where);
            short value = where.x;

            if (value >= gameRect.left && value <= gameRect.right)
                actor->body.x = value;
            value = where.y;
            if (value >= gameRect.top && value <= gameRect.bottom)
                actor->body.y = value;
            view->nextUpdate = 0;
            view->body.running = 1;
            view->body.frame = 0;
            view->body.frameOffset = 1;
            view->changed = 1;
            if (!actor->unknownC0) {
                found = 0;
                for (column = 0; column < placedViewCount; column++)
                    if (ptInRect(&rowEntryRects[column], where) && !lillyBoard[column][0].attributes[0]) {
                        switch (actor->unknownDe) {
                        case 1:
                            if (lillyBoard[column][0].attributes[1] == actor->unknownDf)
                                found = 1;
                            break;
                        case 2:
                            if (lillyBoard[column][0].attributes[2] == actor->unknownDf)
                                found = 1;
                            break;
                        case 3:
                            if (lillyBoard[column][0].attributes[3] == actor->unknownDf)
                                found = 1;
                            break;
                        }
                        if (found) {
                            cursorRow = column;
                            marker->body.running = 1;
                            column = placedViewCount;
                        }
                    }
            } else {
                short down = isButtonStillDown(2);

                if (!down)
                    down = isButtonStillDown(buttonDown);
                if (down) {
                    if (lillyDragState != 4) {
                        short d;

                        if (!ptInRect(&lillyArea, where)) {
                            cancel = 1;
                        } else if (lillyDragState == 2) {
                            if (ptInRect(&lillyArea1, where)) {
                                d = MAGNITUDE(123 - where.x);
                                if (d * 539 / 100 > where.y - 62)
                                    cancel = 1;
                                else
                                    lillyDragState = 3;
                            } else if (ptInRect(&lillyArea2, where)) {
                                d = where.x - 509;
                                if (d * 539 / 100 > MAGNITUDE(where.y - 430))
                                    cancel = 1;
                                else
                                    lillyDragState = 3;
                            } else {
                                lillyDragState = 3;
                            }
                        } else if (ptInRect(&lillyArea1, where)) {
                            d = MAGNITUDE(123 - where.x);
                            if (d * 539 / 100 > where.y - 62)
                                cancel = 1;
                        } else if (ptInRect(&lillyArea2, where)) {
                            d = where.x - 509;
                            if (d * 539 / 100 > MAGNITUDE(where.y - 430))
                                cancel = 1;
                        }
                    }
                } else if (lillyDragState == 1 || lillyDragState == 4) {
                    lillyDragState = 2;
                }
                if (lillyDragState == 3 && !cancel && swapToolStage < 6) {
                    lillyDragState = 1;
                    if (swapStep == 4 || swapStep == 5) {
                        where.x += 27;
                        where.y += 22;
                        for (row = 0, hit = 0; row < 12; row++)
                            for (column = 0; column < 12; column++) {
                                cell = lillyBoard[row][column].rect;
                                cell.top += 4;
                                cell.left += 4;
                                cell.right -= 4;
                                cell.bottom -= 4;
                                if (ptInRect(&cell, where)) {
                                    hit = 1;
                                    hitColumn = column;
                                    hitRow = row;
                                    row = 12;
                                    column = 12;
                                }
                            }
                        if (hit && !lillyBoard[hitRow][hitColumn].attributes[0]) {
                            switch (swapStep) {
                            case 4:
                                flashColumn = hitColumn;
                                flashRow = hitRow;
                                queueViewSound(swapSound++ + 12000, 0);
                                showViewOnSquare(swapFirstMarker, flashRow, flashColumn);
                                swapStep = 5;
                                swapPending = 1;
                                break;
                            case 5:
                                swapColumn = hitColumn;
                                swapRow = hitRow;
                                queueViewSound(swapSound++ + 12000, 0);
                                showViewOnSquare(swapSecondMarker, swapRow, swapColumn);
                                swapStep = 6;
                                if (swapColumn != flashColumn || swapRow != flashRow)
                                    if (++swapsThisStage >= swapsPerStage && swapToolStage < 6) {
                                        swapToolStage++;
                                        swapsThisStage = 0;
                                        setViewScript(view, swapToolStage + 10078, 1);
                                        if (swapToolStage == 6 && padsPlaced == lillyPartySize)
                                            checkLillyArrivals();
                                    }
                                break;
                            }
                            if (swapSound > 3)
                                swapSound = 0;
                        }
                    }
                }
                if (cancel) {
                    actor->body.x = start.x;
                    actor->body.y = start.y;
                    if (swapStep == 5)
                        placeViewOnSquare(swapFirstMarker, flashRow, flashColumn);
                    swapPending = 0;
                    lillyDragState = 0;
                }
            }
            if (!found) {
                if (marker->body.running && lastRow >= 0 && lastRow <= 11) {
                    marker->body.running = 0;
                    rect.left = lillyBoard[lastRow][cursorColumn].rect.left - 17;
                    rect.top = lillyBoard[lastRow][cursorColumn].rect.top - 14;
                    rect.right = lillyBoard[lastRow][cursorColumn].rect.right - 17;
                    rect.bottom = lillyBoard[lastRow][cursorColumn].rect.bottom - 14;
                    unionRgnRect(removedRgn, &rect);
                }
            } else {
                if (lastRow != cursorRow && lastRow >= 0 && lastRow <= 11) {
                    rect.left = lillyBoard[lastRow][cursorColumn].rect.left - 17;
                    rect.top = lillyBoard[lastRow][cursorColumn].rect.top - 14;
                    rect.right = lillyBoard[lastRow][cursorColumn].rect.right - 17;
                    rect.bottom = lillyBoard[lastRow][cursorColumn].rect.bottom - 14;
                    unionRgnRect(removedRgn, &rect);
                }
                lastRow = cursorRow;
            }
            if (!cancel)
                mainLoopEvents();
            resetViewClock();
        }
        if (hideDragCursor)
            showCursor();
        if (found) {
            lillyClaim = cursorRow;
            *(Point *)&actor->targetX = placedViewPoints[cursorRow];
            if (marker->body.running) {
                marker->body.running = 0;
                rect.left = lillyBoard[cursorRow][cursorColumn].rect.left - 17;
                rect.top = lillyBoard[cursorRow][cursorColumn].rect.top - 14;
                rect.right = lillyBoard[cursorRow][cursorColumn].rect.right - 17;
                rect.bottom = lillyBoard[cursorRow][cursorColumn].rect.bottom - 14;
                unionRgnRect(removedRgn, &rect);
                mainLoopEvents();
            }
        } else {
            cursorRow = -1;
            lillyClaim = -1;
        }
        view->id = id;
        view->interval = interval;
        discardEvents(3);
    }
}

/*
 * Sets up the other puzzle's board: turns and mirrors the level's grids,
 * leaves out some pieces, deals the squares and fills them in, noting
 * starting squares on the first row (lillyStarts) at levels 3 and 4.
 */
/* @zoombi32 0x0042cc75 */
void setUpBoard()
{
    short k;
    short unusedA[1];
    short next;
    short limit;
    short unusedB[2];
    short had1;
    short had2;
    short had3;
    short count;
    short total;
    short unusedD[1];
    short row;
    short column;
    short value;

    fillMemory(valueUses, 0, 24);
    for (row = 0; row < 13; row++) {
        rowValueUsed[row] = row;
        columnValueUsed[row] = row;
    }
    rowValueUsed[13] = 0;
    columnValueUsed[13] = 0;
    for (row = 0; row < 12; row++)
        for (column = 0; column < 13; column++)
            lillyBoard[row][column].attributes[0] = 0;
    if (lillyLevel == 1 || lillyLevel == 2) {
        limit = 0;
        layersToSearchCount = 0;
        if (lillyLevel == 1)
            column = 12 - levelLeftOut[lillyPartySize];
        else
            column = 12;
    } else {
        column = 12;
        if (lillyLevel == 3) {
            limit = 2;
            if (levelLeftOut[lillyPartySize] < 8) {
                switch (randomBetween(3, 5)) {
                case 3:
                    layersToSearchCount = 3;
                    turnGrid(grid1, 0);
                    break;
                case 4:
                    layersToSearchCount = 4;
                    turnGrid(grid2, 0);
                    break;
                case 5:
                    layersToSearchCount = 5;
                    turnGrid(grid3, 0);
                    break;
                }
            } else {
                layersToSearchCount = 4;
                turnGrid(grid2, 0);
            }
        } else if (lillyLevel == 4) {
            limit = 3;
            if (levelLeftOut[lillyPartySize] < 8) {
                if (randomBetween(4, 5) == 4) {
                    layersToSearchCount = 4;
                    turnGrid(grid2, 0);
                } else {
                    layersToSearchCount = 5;
                    turnGrid(grid3, 0);
                }
            } else {
                layersToSearchCount = 4;
                turnGrid(grid2, 0);
            }
        }
    }
    switch (randomBetween(0, 2)) {
    case 0:
        turnGrid(grid1, 1);
        turnGrid(grid2, 1);
        turnGrid(grid3, 1);
        break;
    case 1:
        mirrorGrid(grid1, randomBetween(0, 1));
        mirrorGrid(grid2, randomBetween(0, 1));
        mirrorGrid(grid3, randomBetween(0, 1));
        break;
    case 2:
        break;
    }
    row = 12;
    for (k = 0; k < column; k++) {
        short j = randomBetween(1, row);

        rowValueUsed[columnValueUsed[j]] = 0;
        for (; j < row + 1; j++)
            columnValueUsed[j] = columnValueUsed[j + 1];
        row--;
    }
    dealSquares();
    count = 0;
    total = 0;
    for (row = 0; row < 12; row++)
        for (column = 0; column < 12; column++) {
            squareClaims[row][column] = -1;
            lillyBoard[row][column].attributes[0] = 0;
            lillyBoard[row][column].attributes[1] = 0;
            lillyBoard[row][column].attributes[2] = 0;
            had1 = 0;
            had2 = 0;
            had3 = 0;
            for (k = 0; k < 3; k++) {
                switch (k) {
                case 0:
                    value = grid1[row][column];
                    if (value) {
                        if (layersToSearchCount == 3)
                            next = value;
                        else
                            next = value + 1;
                        if (next > 3)
                            next = 1;
                    }
                    break;
                case 1:
                    value = grid2[row][column];
                    if (value) {
                        if (layersToSearchCount == 4)
                            next = value;
                        else
                            next = value + 1;
                        if (next > 7)
                            next = 4;
                    }
                    break;
                case 2:
                    value = grid3[row][column];
                    if (value) {
                        if (layersToSearchCount == 5)
                            next = value;
                        else
                            next = value + 1;
                        if (next > 12)
                            next = 8;
                    }
                    break;
                }
                if (value && rowColumnAllowed[row] && rowColumnAllowed[column] && !rowValueUsed[value] && valueUses[value] < 2)
                    if (randomBetween(0, 100) > 75 || (row == 11 && !valueUses[value])) {
                        valueUses[value]++;
                        value = next;
                        total++;
                    }
                if (value > 0 && value < 13) {
                    switch (squareDeals[value].a) {
                    case 1:
                        had1 = 1;
                        lillyBoard[row][column].attributes[squareDeals[value].a] = squareDeals[value].b;
                        break;
                    case 2:
                        had2 = 1;
                        lillyBoard[row][column].attributes[squareDeals[value].a] = squareDeals[value].b;
                        break;
                    case 3:
                        had3 = 1;
                        lillyBoard[row][column].attributes[squareDeals[value].a] = squareDeals[value].b;
                        break;
                    }
                    if ((lillyLevel == 3 || lillyLevel == 4) && count < limit && !row)
                        switch (layersToSearchCount) {
                        case 3:
                            if (value >= 1 && value <= 3) {
                                lillyStarts[count].column = column;
                                lillyStarts[count].attribute = squareDeals[value].a;
                                lillyStarts[count].layer = squareDeals[value].b;
                                lillyStarts[count].c = squareDeals[value].c;
                                count++;
                            }
                            break;
                        case 4:
                            if (value >= 4 && value <= 7) {
                                lillyStarts[count].column = column;
                                lillyStarts[count].attribute = squareDeals[value].a;
                                lillyStarts[count].layer = squareDeals[value].b;
                                lillyStarts[count].c = squareDeals[value].c;
                                count++;
                            }
                            break;
                        case 5:
                            if (value >= 8 && value <= 12) {
                                lillyStarts[count].column = column;
                                lillyStarts[count].attribute = squareDeals[value].a;
                                lillyStarts[count].layer = squareDeals[value].b;
                                lillyStarts[count].c = squareDeals[value].c;
                                count++;
                            }
                            break;
                        }
                }
            }
            if (!had1)
                lillyBoard[row][column].attributes[1] = randomBetween(0, 2);
            if (!had2)
                lillyBoard[row][column].attributes[2] = randomBetween(0, 3);
            if (!had3)
                lillyBoard[row][column].attributes[3] = randomBetween(0, 4);
            lillyBoard[row][column].attributes[4] =
                overlayImageBase[lillyBoard[row][column].attributes[3]] + lillyBoard[row][column].attributes[1];
            lillyBoard[row][column].rect.left = rowLeft[row + 1] + column * 35;
            lillyBoard[row][column].rect.top = rowTop[row + 1] + columnDy[column];
            lillyBoard[row][column].rect.right = lillyBoard[row][column].rect.left + 36;
            lillyBoard[row][column].rect.bottom = lillyBoard[row][column].rect.top + 30;
        }
    if (lillyLevel > 1) {
        flashColumn = presetSwapColumns[0].x;
        flashRow = presetSwapRows[0].x;
        swapColumn = presetSwapColumns[1].x;
        swapRow = presetSwapRows[1].x;
        swapSquares();
        flashColumn = presetSwapColumns[2].x;
        flashRow = presetSwapRows[2].x;
        swapColumn = presetSwapColumns[3].x;
        swapRow = presetSwapRows[3].x;
        swapSquares();
    }
    total += 5;
    swapsPerStage = total / 6;
    swapsPerStage += swapsPerStage * 6 < total;
}

/* Opens the other puzzle (Lilly.MHK) at the level reached. */
/* @zoombi32 0x004281b0 */
void openLilly()
{
    Snoid *snoid;
    Point place;
    LillyActor actor;
    short i;
    View *view;
    View *other;

    swapToolStage = 0;
    swapsPerStage = 0;
    swapsThisStage = 0;
    unusedLilly1 = 0;
    lillyGoReady = 0;
    sceneDue = 0;
    unusedLilly2 = 0;
    lillyOpen = 0;
    claimedRow = 0;
    firstArrivals = 0;
    unusedLilly3 = 0;
    actorCount = 12;
    event4Pad = 0;
    layersToSearchCount = 0;
    unusedLilly4 = 0;
    presetSwapIndex = 0;
    nextActorTime = clockTime() + 600;
    unusedLilly5 = 0;
    padsPlaced = 0;
    unusedLilly6 = 0;
    unusedLilly7 = 0;
    unusedLilly8 = 0;
    swapPending = 0;
    lillyStage = 0;
    flashCount = 0;
    unusedLilly9 = 1;
    hintSound = 0;
    fillMemory(snoidPadViews, 0, 42);
    fillMemory(actorViews, 0, 28);
    fillMemory(unusedLillyTable1, 0, 288);
    fillMemory(lillyLayerViews, 0, 20);
    fillMemory(rowAnchorViews, 0, 24);
    fillMemory(unusedLillyTable2, 0, 40);
    fillMemory(planQueue, 0, 40);
    fillMemory(planWaiting, 0, 40);
    fillMemory(event60Views, 0, 40);
    fillMemory(unusedLillyTable3, 0, 40);
    fillMemory(event3Views, 0, 40);
    fillMemory(event2Views, 0, 40);
    fillMemory(landerQueue, 0, 40);
    fillMemory(event44Views, 0, 40);
    fillMemory(hopperQueue, 0, 288);
    fillMemory(unusedLillyTable4, 0, 288);
    fillMemory(hopperWaiting, 0, 288);
    fillMemory(actorsOut, 0, 288);
    fillMemory(event80Views, 0, 288);
    fillMemory(replanQueue, 0, 288);
    replanQueueCount = 0;
    event80Count = 0;
    unusedLilly10 = 0;
    nextStart = 0;
    actorsOutCount = 0;
    padsArrived = 0;
    unusedLilly11 = 0;
    hopperQueueCount = 0;
    hopperWaitingCount = 0;
    unusedLilly12 = 0;
    event44Count = 0;
    landerBusy = 0;
    landerQueueCount = 0;
    jumper2Busy = 0;
    landedJumper = 0;
    jumperBusy = 0;
    finishedLander = 0;
    event2Count = 0;
    event3Count = 0;
    unusedLilly13 = 0;
    planQueueCount = 0;
    planWaitingCount = 0;
    jumperQueueCount = 0;
    event60Count = 0;
    unusedLilly14 = 0;
    openGameFile(&lillyFile, "Lilly.MHK");
    setCurrentMap(lillyFile);
    lillyButtonImages = loadImageBank(7000, &lillyButtonResource);
    drawBackdrop(5000);
    loadFeatureGroup(11000, 0, 0);
    loadFeatureGroup(14000, 1, 0);
    loadFeatureGroup(10000, 2, 0);
    loadScripts(11000, 1);
    addScripts(14000, 5, 0);
    addScripts(10000, 167, 0);
    copyPaletteRange(10, 236);
    addView(0x1000, drawLillyButtons, updateLillyButtons, 0, 0, 0, 0, 0);
    lillyImages = loadImageBank(13000, &lillyImagesResource);
    loadTablePair(rowPlaceResources, 100, &rowLeft, &rowTop);
    loadTablePair(actorHotSpotResources, 10000, &actorHotSpotsX, &actorHotSpotsY);
    loadTablePair(squareHotSpotResources, 200, &squareHotSpotsX, &squareHotSpotsY);
    makePartySnoids(0);
    lillyPartySize = listChosenSnoids()->count;
    setLillyLevel(lillyLevel = sceneLevel() + 1);
    loadLockedTable(&grid1Resource, &grid1Handle, 15000, (short **)&grid1);
    loadLockedTable(&grid2Resource, &grid2Handle, 15001, (short **)&grid2);
    loadLockedTable(&grid3Resource, &grid3Handle, 15002, (short **)&grid3);
    setUpBoard();
    boardView = addView(0x4008000, drawBoard, updateLillyBackdrop, 0, 0, 0, 0, 0);
    for (i = 0; i < 12; i++) {
        place.x = rowLeft[i + 1] + 18;
        place.y = rowTop[i + 1] + 15;
        placedViewPoints[i] = place;
        placeClaims[i] = 0;
    }
    placedViewCount = 12;
    placesClaimable = 1;
    cursorSquareView = addView(0x4008000, drawCursorSquare, updateSquareHighlight, 0, 5, 0, 0, 0);
    swapFirstMarker = addView(0x4008000, flashSquare, updateMarkerView, 0, 4, 0, 0, 0);
    swapSecondMarker = addView(0x4008000, flashSwap, updateMarkerView, 0, 4, 0, 0, 0);
    short n = 0;

    for (view = viewListEnd(1); view; view = view->next)
        if (view->flags == 1) {
            view->body.running = 0;
            view->body.x = 680;
            view->body.y = 220;
            snoid = viewSnoid(view);
            snoidPadViews[n] = addView(0x4180000, drawCels, runViewScript, n + 10109, 4, &padPlaces[n], 0, 0);
            other = findView(snoidPadViews[n]);
            if (other) {
                other->placed = placeByHotSpot;
                short *parts = (short *)&other->body;

                parts[10] = snoid->features[0] - 1;
                parts[13] = view->id;
                parts[12] = n;
                parts[15] = 0;
                if (n == lillyPartySize - 2 || n == lillyPartySize - 1) {
                    other->body.running = 0;
                    if (lillyLevel == 1) {
                        setViewScript(other, n + 10089, 1);
                        other->body.running = 1;
                        other->placed = placeByHotSpot;
                        other->notify = lillyActorNotify;
                    }
                    snoidsOnTheirWay = lillyPartySize - n;
                    if (snoidsOnTheirWay < 0)
                        snoidsOnTheirWay = 0;
                }
                n++;
            }
        }
    addLillyActors();
    for (i = 14000; i <= 14004; i++)
        lillyLayerViews[i - 14000] = addView(0x4000000, drawCels, runViewScript, i, 0, 0, 0, 0);
    if (lillyLevel > 1) {
        lillyView11000 = addView(0x180000, drawCels, runViewScript, 11000, 5, 0, 0, 0);
        actor.unknownC2 = 0;
        actor.unknownC0 = 2;
        swapToolView = addView(0x4180002, drawCels, runViewScript, swapToolStage + 10078, 6, &actor, 0, 0);
        swapStep = 4;
        other = findView(swapToolView);
        if (other) {
            other->flags = 0;
            other->body.running = 0;
            other->body.x = 38;
            other->body.y = 415;
        }
    } else {
        swapStep = 0;
    }
    for (i = 0; i < 12; i++)
        rowAnchorViews[i] = addView(0x4000000, lillyNoDraw, lillyNoUpdate, 14000, 0, 0, 0, 0);
    fadeOutViews();
    copyPaletteRange(10, 236);
    updateViews();
    setGroupLists(lillyGroups, 1, (short)0xc000);
    drawLillyButton(1, 0, 0);
    drawLillyButton(2, 0, 0);
    showRect(&shownGameRect);
    fadeInViews();
    if (lillyLevel == 1)
        queueViewSound(997, 0);
    chooseSnoids(0, 0);
    resetViewClock();
    lillyOpen = 1;
    addSoundRange(996, 997, 0);
    addSoundRange(20000, 29999, 1);
    addSoundRange(11000, 11001, 1);
    addSoundRange(12000, 12004, 0);
    addSoundRange(10009, 10009, 0);
    addSoundRange(10010, 10010, 0);
    addSoundRange(10011, 10011, 0);
    addSoundRange(10000, 10000, 0);
    addSoundRange(10002, 10002, 0);
    addSoundRange(10004, 10004, 0);
    addSoundRange(10003, 10003, 0);
    addSoundRange(10005, 10008, 0);
    switch (campHint((short *)(gameState + 0x34))) {
    case 2:
        hintSound = randomBetween(20076, 20077);
        break;
    default:
        if (lillyLevel > 1)
            hintSound = randomBetween(20075, 20077);
        else
            hintSound = 20075;
        break;
    }
    setLillyStage(3);
    if (lillyLevel > 1) {
        other = findView(lillyView11000);
        if (other) {
            setViewScript(other, 11000, 1);
            other->notify = lillyViewNotify3;
        }
    }
    setViewsLocked(0);
}

/*
 * The other puzzle's idle work, once per pass: ends it when asked, and
 * otherwise starts the waiting actors' next moves (the queues of views
 * whose scripts have ended), sends in new actors along the starting row
 * from time to time (levels 3 and 4), and deletes the finished ones.
 */
/* Not exact: the original caches the addresses of jumper2Busy and jumperQueueCount
   in esi and edi (this caches others), which changes the registers
   throughout. */
/* @zoombi32 0x00428d84 */
void lillyFrame()
{
    short row;
    View *view;
    LillyActor *actor;
    short script;

    if (inLillyFrame || !lillyOpen)
        return;
    inLillyFrame = 1;
    updateViews();
    if (sceneDue) {
        if (isSoundPlaying(996, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
            inLillyFrame = 0;
            return;
        }
        if (!dialogQuestion || dialogQuestion == 3) {
            if (dialogQuestion == 3)
                chooseSnoids(0, 0);
            if (viewsLocked || !snoidsOnTheirWay || snoidsArrived >= 1) {
                pendingScene = sceneDue;
                sceneDue = 0;
                setCurrentMap(0);
                closeLilly();
                inLillyFrame = 0;
                return;
            }
        } else if (dialogQuestion == 2) {
            dialogQuestion = 0;
            sceneDue = 0;
        }
    } else if (dialogFlags) {
        playAmbientSound();
        inLillyFrame = 0;
        return;
    } else {
        while (jumperQueueCount > 0 && !jumperBusy) {
            jumperQueueCount--;
            jumperBusy = jumperQueue[jumperQueueCount];
            view = findView(jumperBusy);
            if (view) {
                setViewScript(view, 10057, 1);
                view->placed = placeJumper;
                view->notify = lillyNotify44;
            }
        }
        while (event60Count > 0 && !jumper2Busy) {
            event60Count--;
            jumper2Busy = event60Views[event60Count];
            view = findView(jumper2Busy);
            if (view) {
                setViewScript(view, 10058, 1);
                view->placed = placeJumperAt;
                view->notify = lillyNotify49;
                view->flags = 0x4980002;
                moveView(view->id, 0, lillyLayerView1);
            }
        }
        if (landedJumper) {
            view = findView(landedJumper);
            landedJumper = 0;
            jumper2Busy = 0;
            if (view) {
                actor = (LillyActor *)&view->body;
                short *parts = (short *)&view->body;

                *(Point *)&actor->body.x = jumpPlaces[parts[20]];
                setViewScript(view, parts[20] + 10043, 1);
            }
        }
        while (event44Count > 0 && !jumperBusy) {
            view = findView(event44Views[--event44Count]);
            if (view) {
                actor = (LillyActor *)&view->body;
                if (actor->unknownDe == 3) {
                    setViewScript(view, randomBetween(0, 2) * 2 + 10061, 1);
                    view->placed = placeActorCelsLow;
                    view->notify = lillyNotify60;
                } else {
                    setViewScript(view, randomBetween(0, 2) * 2 + 10060, 1);
                    view->placed = placeActorCels;
                    view->notify = lillyNotify60;
                }
            }
        }
        while (landerQueueCount && !landerBusy) {
            landerQueueCount--;
            landerBusy = landerQueue[landerQueueCount];
            view = findView(landerBusy);
            if (view) {
                setViewScript(view, 10059, 1);
                view->placed = placeLander;
                view->notify = lillyNotify54;
            }
        }
        if (finishedLander) {
            deleteView(finishedLander);
            finishedLander = 0;
            landerBusy = 0;
        }
        while (event3Count) {
            view = findView(event3Views[--event3Count]);
            if (view) {
                short *parts = (short *)&view->body;

                setViewScript(view, parts[12] + 10141, 1);
                view->placed = placeByHotSpot;
                view->notify = lillyActorNotify;
                view->flags |= 0x4000000;
            }
        }
        while (event2Count) {
            view = findView(event2Views[--event2Count]);
            if (view) {
                short *parts = (short *)&view->body;

                setViewScript(view, parts[15] + 10019, 1);
                view->placed = placeByHotSpot;
                view->notify = hopperNotify;
            }
        }
        if (!swapPending) {
            if (planTick == 1) {
                planTick = 0;
                while (planWaitingCount) {
                    planWaitingCount--;
                    planQueue[planQueueCount] = planWaiting[planWaitingCount];
                    planQueueCount++;
                }
                while (planQueueCount) {
                    view = findView(planQueue[--planQueueCount]);
                    if (view) {
                        if (clockTime() >= view->nextUpdate) {
                            actor = (LillyActor *)&view->body;
                            moveView(view->id, 0, rowAnchorViews[actor->row]);
                            script = pickLeastVisited(actor);
                            if (script == 10031) {
                                setViewScript(view, script += actor->row, 1);
                                view->placed = placeActorCels3;
                                view->notify = lillyNotify30;
                            } else if (script) {
                                setViewScript(view, script, 1);
                                view->placed = placeActorCels3;
                                view->notify = hopNotify;
                            } else {
                                planWaiting[planWaitingCount] = planQueue[planQueueCount];
                                planWaitingCount++;
                            }
                        } else {
                            planWaiting[planWaitingCount] = planQueue[planQueueCount];
                            planWaitingCount++;
                        }
                    }
                }
            } else {
                planTick = 1;
                if (lillyLevel > 2 && !swapPending) {
                    if (snoidsOnTheirWay <= 0) {
                        while (replanQueueCount) {
                            view = findView(replanQueue[--replanQueueCount]);
                            if (view) {
                                actor = (LillyActor *)&view->body;
                                startPlan(actor);
                            }
                        }
                        if (clockTime() > nextActorTime && actorsOutCount < 20) {
                            short column = lillyStarts[nextStart].column;

                            row = 0;
                            if (!lillyBoard[row][column].attributes[0]) {
                                actorsOut[actorsOutCount] = addLillyActor(nextStart);
                                view = findView(actorsOut[actorsOutCount]);
                                if (view) {
                                    actorsOutCount++;
                                    actor = (LillyActor *)&view->body;
                                    short *parts = (short *)&view->body;

                                    actor->column = column;
                                    actor->row = row;
                                    actor->unknownBe = nextStart;
                                    actor->unknownDe = lillyStarts[0].attribute;
                                    actor->unknownDf = lillyBoard[actor->row][actor->column].attributes[actor->unknownDe];
                                    actor->unknownE0 = squareSetC[attributeImageIndex[actor->unknownDe]] + actor->unknownDf;
                                    parts[25] = 0;
                                    parts[26] = 0;
                                    startPlan(actor);
                                    lillyBoard[actor->row][actor->column].attributes[0] = 1;
                                    actor->unknownC2 = 1;
                                    actor->body.running = 1;
                                    actor->grid[actor->row][actor->column] = 1;
                                    actor->body.x = rowLeft[actor->row + 1] + actor->column * 35 + 2;
                                    actor->body.y = rowTop[actor->row + 1] + columnDy[actor->column] - 17;
                                    squareClaims[actor->row][actor->column] = 1;
                                    setViewScript(view, 10067, 1);
                                    view->placed = placeActorCelsHidden;
                                    view->notify = lillyNotify70;
                                    moveView(view->id, 0, rowAnchorViews[actor->row]);
                                    nextActorTime = clockTime() + 480;
                                }
                            } else if (squareClaims[row][column] == 1) {
                                actorsOut[actorsOutCount] = addLillyActor(nextStart);
                                view = findView(actorsOut[actorsOutCount]);
                                if (view) {
                                    actorsOutCount++;
                                    actor = (LillyActor *)&view->body;
                                    short *parts = (short *)&view->body;

                                    parts[25] = 1;
                                    parts[26] = 0;
                                    actor->column = column;
                                    actor->row = row;
                                    actor->unknownC2 = 1;
                                    actor->body.running = 1;
                                    actor->grid[actor->row][actor->column] = 1;
                                    actor->body.x = rowLeft[actor->row + 1] + actor->column * 35 + 2;
                                    actor->body.y = rowTop[actor->row + 1] + columnDy[actor->column] - 17;
                                    setViewScript(view, 10067, 1);
                                    view->placed = placeActorCelsHidden;
                                    view->notify = lillyNotify70;
                                    moveView(view->id, 0, rowAnchorViews[actor->row]);
                                    nextActorTime = clockTime() + 480;
                                }
                            }
                            if (++nextStart >= startCount)
                                nextStart = 0;
                        }
                    }
                    while (hopperWaitingCount) {
                        hopperWaitingCount--;
                        hopperQueue[hopperQueueCount] = hopperWaiting[hopperWaitingCount];
                        hopperQueueCount++;
                    }
                    while (hopperQueueCount) {
                        view = findView(hopperQueue[--hopperQueueCount]);
                        if (view) {
                            if (clockTime() >= view->nextUpdate) {
                                actor = (LillyActor *)&view->body;
                                short *parts = (short *)&view->body;

                                if (!parts[25])
                                    script = pickNextSquare(view);
                                else
                                    script = moveActorDown(view);
                                if (script == 10069) {
                                    moveView(view->id, 0, rowAnchorViews[actor->row]);
                                    setViewScript(view, script, 1);
                                    view->placed = placeActorCelsHidden;
                                    view->notify = lillyNotify70;
                                    actor->unknownC2 = 0;
                                } else if (script) {
                                    switch (actor->unknownD5) {
                                    case 0:
                                        moveView(view->id, 0, rowAnchorViews[actor->row]);
                                        break;
                                    case 1:
                                        moveView(view->id, 0, rowAnchorViews[actor->row]);
                                        break;
                                    case 2:
                                        moveView(view->id, 0, rowAnchorViews[actor->row + 1]);
                                        break;
                                    case 3:
                                        moveView(view->id, 0, rowAnchorViews[actor->row]);
                                        break;
                                    }
                                    setViewScript(view, script, 1);
                                    view->placed = placeHopper;
                                    view->notify = lillyNotify70;
                                } else {
                                    hopperWaiting[hopperWaitingCount] = hopperQueue[hopperQueueCount];
                                    hopperWaitingCount++;
                                }
                            } else {
                                hopperWaiting[hopperWaitingCount] = hopperQueue[hopperQueueCount];
                                hopperWaitingCount++;
                            }
                        }
                    }
                }
            }
        }
        while (event80Count)
            deleteView(event80Views[--event80Count]);
        if (event4Pad) {
            deleteView(event4Pad);
            event4Pad = 0;
        }
    }
    playAmbientSound();
    inLillyFrame = 0;
}
