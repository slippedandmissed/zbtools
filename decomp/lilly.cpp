/*
 * lilly (0x4281b0-0x42f920): the lily pad puzzle (scene 11), 'Lilly.MHK'
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
#include "loading.h"
#include "mainloop.h"
#include "net.h"
#include "platform.h"
#include "snoids.h"
#include "sound.h"
#include "view.h"


/* A lilly actor's view body (flag 2, a large body). Partly known. */
struct LillyActor
{
    ViewBody body;
    short unusedBc;
    short startIndex;
    short kind;
    char onBoard;
    char column; /* +0xc3 */
    char row; /* +0xc4 */
    char fromColumn;
    char fromRow;
    char planColumn;
    char planRow;
    short startX; /* +0xc9 */
    short startY;
    short targetX; /* +0xcd */
    short targetY;
    short stepX; /* +0xd1 */
    short stepY;
    char direction;
    char reach;
    short planStep;
    short nextScript; /* +0xd9 */
    char landings;
    short unusedDc;
    char attribute;
    char attributeValue;
    char imageOffset; /* +0xe0: added to its second part's image */
    char padView;
    char unusedE2;
    char thirdImageOffset; /* +0xe3: added to its third part's image */
    short unusedE4;
    char unknownE6[12];
    short grid[12][13]; /* +0xf2 */
};

short lillyLevel = 1;
short squareSetC[5] = {0, 1, 2, 3, 4};
short attributeImageIndex[4] = {0, 0, 3, 7};
short boardView = 0;
long lillyImagesResource = 0;
long unusedLillyResource = 0;
SceneButton lillyButtons[3] = {
    {{600, 403, 639, 440}}, {{600, 441, 639, 478}}, {{0, 0, 640, 480}},
};
Group g_4a1bb8[1] = {{g_4a0766, (InputItem *)lillyButtons, 3, 0x2068}};
GroupList lillyGroups[1] = {{g_4a1bb8, 1, 0, lillyClicked}};
Scene g_4a1bd4[1] = {{openLilly, closeLilly, lillyFrame, 0, lillyKey}};
long lillyButtonResource = 0;
Point padPlaces[42] = {
    {101, 27}, {100, 42}, {95, 55}, {88, 69}, {78, 80}, {88, 24}, {85, 39}, {80, 54}, {72, 67},
    {62, 81}, {74, 25}, {70, 40}, {64, 53}, {56, 67}, {46, 79}, {59, 25}, {55, 39}, {49, 53},
    {41, 65}, {44, 31}, {44, 31}, {629, 348}, {628, 363}, {623, 376}, {616, 389}, {606, 400},
    {616, 345}, {613, 360}, {609, 374}, {601, 385}, {592, 401}, {604, 343}, {601, 358}, {595, 370},
    {587, 385}, {578, 399}, {589, 346}, {584, 359}, {580, 371}, {570, 384}, {574, 349}, {574, 349},
};
Point jumpPlaces[13] = {
    {66, 118}, {60, 147}, {46, 177}, {58, 205}, {44, 232}, {52, 262}, {39, 289}, {18, 313},
    {47, 327}, {17, 345}, {43, 363}, {53, 393},
};
ShortRect rowEntryRects[12] = {
    {134, 65, 168, 93}, {131, 94, 165, 122}, {129, 122, 163, 150}, {127, 152, 161, 180},
    {124, 181, 158, 209}, {117, 210, 151, 238}, {113, 239, 147, 267}, {109, 268, 143, 296},
    {100, 297, 134, 331}, {93, 326, 127, 354}, {91, 355, 125, 383}, {88, 384, 120, 532},
};
Point presetSwapColumns[5] = {{4}, {3}, {8}, {10}};
Point presetSwapRows[5] = {{4}, {6}, {3}, {5}};
ImageBank *lillyButtonImages = 0;
short lillyButton2Lit = 0;
short lillyButton1Drawn = 0;
short columnDy[12] = {2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12};
short inLillyFrame = 0;
short hopDirections[4][4] = {
    {0x2711, 0x271a, 0x271f, 0x2718}, {0x2715, 0x2712, 0x271b, 0x2720},
    {0x271d, 0x2716, 0x2713, 0x271c}, {0x2719, 0x271e, 0x2717, 0x2714},
};
short actorDealOrder[13] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
short actorDealKinds[12] = {1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3};
short actorDealValues[12] = {0, 1, 2, 0, 1, 2, 3, 0, 1, 2, 3, 4};
ShortRect lillyArea = {83, 62, 556, 430};
ShortRect lillyArea1 = {83, 62, 130, 337};
ShortRect lillyArea2 = {509, 155, 556, 430};
short cursorImageBase[5] = {20, 22, 24, 26};
short squareImageBase[4] = {28, 30, 32, 34};
short cursorFrameOffsets[4] = {0, 0, 1, 1};
short cursorSquareFrame = 0;
ShortRect markerArea = {83, 62, 556, 460};
char flashFrame = 0;
char swapFlashFrame = 0;
short squareSetA[12] = {1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3};
short squareSetB[12] = {0, 1, 2, 0, 1, 2, 3, 0, 1, 2, 3, 4};
short overlayImageBase[10] = {5, 8, 11, 14, 17, 0, 0, 10, 11, 12};
short levelLeftOut[21] = {
    1, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10,
};
short rowColumnAllowed[12] = {0, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
short valueUses[12] = {0};
short rowValueUsed[14] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
short columnValueUsed[14] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
short swapSound = 0;

short firstArrivals;
short unusedLillyLevel;
short lillyLevelParam;
short swapToolStage;
short swapsPerStage;
short swapsThisStage;
long rowPlaceResources[2];
long squareHotSpotResources[2];
long actorHotSpotResources[2];
short *rowLeft;
short *rowTop;
short *squareHotSpotsX;
short *squareHotSpotsY;
short *actorHotSpotsX;
short *actorHotSpotsY;
short unusedLilly1;
short dealtKinds[12];
short dealtValues[12];
long grid1Resource;
short grid1Handle;
long grid2Resource;
short grid2Handle;
long grid3Resource;
short grid3Handle;
short (*grid1)[12];

short (*grid2)[12];

short (*grid3)[12];

short unusedLillyTable2[20];
short unusedLilly13;
short hopperQueue[144];
short hopperQueueCount;
short unusedLillyTable4[144];
short unusedLilly11;
short hopperWaiting[144];
short hopperWaitingCount;
short planQueue[20];
short planQueueCount;
short planWaiting[20];
short planWaitingCount;
short jumperQueue[20];
short jumperQueueCount;
short landerQueue[20];
short landerQueueCount;
short event60Views[20];
short event60Count;
short event44Views[20];
short event44Count;
short unusedLillyTable3[20];
short unusedLilly14;
short event2Views[20];
short event2Count;
short event3Views[20];
short event3Count;
short replanQueue[144];
short replanQueueCount;
short landedJumper;
short unusedLilly12;
short jumper2Busy;
short jumperBusy;
short landerBusy;
short finishedLander;
LillyCell lillyBoard[12][13];
LillySearch layerSearches[5];
short squareClaims[12][13];
LillyStart lillyStarts[3];
short lillyLayerViews[10];
short rowAnchorViews[12];
short snoidPadViews[21];
short actorViews[14];
short event80Views[144];
short actorsOut[144];
short unusedLillyTable1[144];
short event80Count;
short unusedLilly10;
short nextStart;
short actorsOutCount;
short lillyPartySize;
short padsArrived;
short padsPlaced;
short unusedLilly4;
short unusedLilly6;
short unusedLilly7;
short unusedLilly9;
short startCount;
short unusedLilly5;
unsigned long nextActorTime;
short unusedLilly3;
short actorCount;
short presetSwapIndex;
long lillyScriptResources[91];
short lillyScriptHandles[91];
short claimedRow;
short lillyView11000;
short event4Pad;
short cursorSquareView;
short swapFirstMarker;
short swapSecondMarker;
short layersToSearchCount;
short cursorColumn;
short cursorRow;
short flashColumn;
short flashRow;
short swapColumn;
short swapRow;
short lillyStage;
short flashCount;
short swapToolView;
short swapStep;
short lillyClaim;
short unusedLilly8;
short swapPending;
long lillyFile;
short lillyOpen;
short lillyGoReady;
short unusedLilly2;
ImageBank *lillyImages;
short planTick;
short actorDealLast;
ShortRect cursorRect;
short *squareSets[3];
short squareSetA3[4];
short squareSetB3[4];
short squareSetC3[4];
short squareSetA4[5];
short squareSetB4[5];
short squareSetC4[5];
short squareSetA5[6];
short squareSetB5[6];
short squareSetC5[6];
LillyDeal squareDeals[13];
short lillyDragState;
Point searchQueue[144];
short searchQueueEnd;
short searchQueueStart;

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

/* @zoombi32 0x0042c0d9 */
void updateLillyBackdrop(View *view, short region)
{
    ShortRect unused;

    if (!dialogFlags && view->reset) {
        view->reset = 0;
        unionRgnRect(region, &lillyArea);
    }
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

/* @zoombi32 0x00428c45 */
void updateLillyButtons(View *view, short region)
{
    if (lillyGoReady) {
        if (!lillyButton2Lit) {
            lillyButton2Lit = 1;
            unionRgnRect(region, &lillyButtons[1].rect);
        }
    } else if (lillyButton2Lit) {
        lillyButton2Lit = 0;
        unionRgnRect(region, &lillyButtons[1].rect);
    }
    if (!lillyButton1Drawn) {
        lillyButton1Drawn = 1;
        unionRgnRect(region, &lillyButtons[0].rect);
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
        actor->body.waypointX = body->cels[0].x;
        actor->body.waypointY = body->cels[0].y;
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
        actor->body.waypointX = body->cels[0].x;
        actor->body.waypointY = body->cels[0].y;
        actor->onBoard = 0;
        view->flags = 0x980002;
        landedJumper = view->id;
        break;
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
        drawImageData((unsigned short *)((char *)lillyButtonImages + lillyButtonImages->offsets[image]), lillyButtons[which - 1].rect.left,
                      lillyButtons[which - 1].rect.top, 8);
        if (show)
            showRect(&lillyButtons[which - 1].rect);
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

            if (actor->onBoard && actor->reach == 11) {
                count++;
                short *parts = (short *)&view->body;
                View *rider = findView(parts[13]);

                if (rider)
                    viewSnoid(rider)->chosen = 1;
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
   offset by imageOffset. */
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
        *cel += actor->imageOffset;
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

/* The same, the second part's image offset by imageOffset - 7. */
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
        *cel = actor->imageOffset + *cel - 7;
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

/* The same for three parts, the second and third offset by imageOffset and
   thirdImageOffset. */
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
        *cel += actor->imageOffset;
        image = *cel++;
        *cel++ -= actorHotSpotsX[image];
        *cel++ -= actorHotSpotsY[image];
        if (*cel > 0) {
            *cel += actor->thirdImageOffset;
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
        actor->body.waypointX = body->cels[0].x;
        actor->body.waypointY = body->cels[0].y;
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

/* @zoombi32 0x0042a6fa */
void lillyNotify44(View *view, short event)
{
    ViewBody *body = &view->body;
    LillyActor *actor = (LillyActor *)&view->body;

    switch (event) {
    case 44:
        actor->body.x = actorHotSpotsX[body->cels[0].image] + body->cels[0].x;
        actor->body.y = actorHotSpotsY[body->cels[0].image] + body->cels[0].y;
        actor->body.waypointX = actorHotSpotsX[body->cels[0].image] + body->cels[0].x;
        actor->body.waypointY = actorHotSpotsY[body->cels[0].image] + body->cels[0].y;
        lillyBoard[actor->row][actor->column + 1].attributes[0] = 0;
        event44Views[event44Count] = view->id;
        event44Count++;
        break;
    }
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
   0x110 are offset by imageOffset, and all but the first are hidden while
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
            *cel += actor->imageOffset;
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
        actor->body.waypointX = body->cels[0].x;
        actor->body.waypointY = body->cels[0].y;
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
            actor->attribute = lillyStarts[0].attribute;
            actor->attributeValue = lillyBoard[row][column].attributes[actor->attribute];
            actor->imageOffset = squareSetC[attributeImageIndex[actor->attribute]] + actor->attributeValue;
            actor->grid[row][column] = 1;
            body[26] = 1;
        } else if (squareClaims[row][column] != 1 && !squareClaims[row][column]) {
            return 0;
        }
    }
    if (blocked)
        return actor->nextScript = 10069;
    lillyBoard[row][column].attributes[0] = 1;
    return actor->nextScript = 10073;
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

    actor.thirdImageOffset = 0;
    actor.unusedE4 = 0;
    actor.unusedE2 = 0;
    actor.column = 0;
    actor.row = 0;
    actor.fromColumn = 0;
    actor.fromRow = 0;
    actor.planColumn = 0;
    actor.planRow = 0;
    actor.direction = 2;
    actor.unusedBc = 0;
    actor.startIndex = value;
    actor.kind = 1;
    actor.unusedDc = 0;
    actor.landings = 0;
    actor.onBoard = 0;
    actor.planStep = 0;
    actor.reach = 11;
    actor.body.celsEnd = 0;
    actor.body.script = 0;
    actor.body.scriptGroup = 0;
    actor.body.frameOffset = 1;
    actor.body.running = 0;
    actor.startX = 0;
    actor.startY = 0;
    actor.body.x = 100;
    actor.body.y = 25;
    actor.body.waypointX = 100;
    actor.body.waypointY = 25;
    for (short row = 0; row < 12; row++)
        for (short column = 0; column < 12; column++)
            actor.grid[row][column] = 0;
    actor.nextScript = 63;
    actor.startIndex = value;
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
        actor->body.waypointX = actorHotSpotsX[body->cels[0].image] + body->cels[0].x;
        actor->body.waypointY = actorHotSpotsY[body->cels[0].image] + body->cels[0].y;
        firstArrivals++;
        if (firstArrivals == 1)
            lillyGoReady = 1;
        actor->landings++;
        actor->reach = 0;
        if (actor->landings == 2) {
            landerQueue[landerQueueCount] = view->id;
            landerQueueCount++;
        } else {
            jumperQueue[jumperQueueCount] = view->id;
            jumperQueueCount++;
        }
        lillyBoard[actor->row][actor->column].attributes[0] = 0;
        body->cels[2].image = 0;
        moveView(actor->padView, 0, lillyLayerViews[3]);
        View *other = findView(actor->padView);

        if (other) {
            other->body.running = 1;
            setViewScript(other, actor->row + 10129, 1);
            other->placed = placeByHotSpot;
            other->notify = lillyActorNotify;
            short *parts = (short *)&other->body;
            View *rider = findView(parts[13]);

            if (rider) {
                padsArrived++;
                viewSnoid(rider)->chosen = 1;
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
        *cel += actor->imageOffset;
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
        *cel += actor->imageOffset;
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
        *cel += actor->imageOffset;
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
 * Follows the rising numbers in an actor's grid from where it is (clearing
 * them as it goes) until it reaches `limit` across (kind 0) or down
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

    if (!actor->kind) {
        actor->grid[actor->planRow][actor->planColumn] = 0;
        x = actor->planColumn;
        y = actor->planRow;
    } else {
        actor->grid[actor->row][actor->column] = 0;
        x = actor->column;
        y = actor->row;
    }
    steps = 0;
    bestX = x;
    bestY = y;
    if (!actor->kind)
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
                if (!actor->kind && c > reach)
                    reach = c;
                else if (actor->kind == 1 && r > reach)
                    reach = r;
            }
        }
        actor->grid[bestY][bestX] = 0;
        x = bestX;
        y = bestY;
        steps++;
    }
}

/* The same, jumping to its place in jumpPlaces (by startIndex). */
/* @zoombi32 0x0042ab6f */
void placeJumperAt(View *view)
{
    LillyActor *actor = (LillyActor *)&view->body;
    short *cel;
    short image;

    switch (actor->body.frame) {
    case 0:
        actor->targetX = jumpPlaces[actor->startIndex].x;
        actor->targetY = jumpPlaces[actor->startIndex].y;
        actor->stepX = (actor->targetX - actor->body.x) / 3;
        actor->stepY = (actor->targetY - actor->body.y) / 3;
        /* fall through */
    case 1:
    case 2:
        cel = (short *)&view->body;
        image = *cel++;
        *cel++ -= actorHotSpotsX[image];
        *cel++ -= actorHotSpotsY[image];
        *cel += actor->imageOffset;
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
        *cel += actor->imageOffset;
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
        *cel += actor->imageOffset;
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
        actor.unusedBc = 0;
        actor.startIndex = i;
        actor.kind = 0;
        actor.column = 0;
        actor.row = 0;
        actor.fromColumn = 0;
        actor.fromRow = 0;
        actor.planColumn = 0;
        actor.planRow = 0;
        actor.planStep = 0;
        actor.direction = 1;
        actor.reach = 11;
        actor.unusedDc = 0;
        actor.landings = 0;
        actor.onBoard = 0;
        actor.padView = 0;
        actor.unusedE2 = 0;
        actor.thirdImageOffset = 0;
        actor.unusedE4 = 0;
        actor.body.celsEnd = 0;
        actor.body.running = 1;
        actor.startX = 0;
        actor.startY = 0;
        actor.targetX = 0;
        actor.targetY = 0;
        actor.body.x = 0;
        actor.body.y = 0;
        actor.body.waypointX = 0;
        actor.body.waypointY = 0;
        for (short row = 0; row < 12; row++)
            for (short column = 0; column < 12; column++)
                actor.grid[row][column] = 0;
        short k = randomBetween(0, actorDealLast);

        actor.attribute = actorDealKinds[actorDealOrder[k]];
        actor.attributeValue = actorDealValues[actorDealOrder[k]];
        actor.imageOffset = actorDealOrder[k];
        dealtKinds[i] = actor.attribute;
        dealtValues[i] = actor.attributeValue;
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
        *cel += actor->imageOffset;
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
        *cel += actor->imageOffset;
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
            *cel += actor->imageOffset;
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
            && lillyBoard[r - 1][c].attributes[actor->attribute] == actor->attributeValue
            && layerSearches[actor->attributeValue].steps[r][c] < layerSearches[actor->attributeValue].steps[actor->row + 1][actor->column]) {
            best = direction;
            bestColumn = c;
            bestRow = r - 1;
            direction = 4;
        }
        direction++;
    }
    if (done)
        return actor->nextScript = 10069;
    switch (best) {
    case 0:
        actor->direction = 0;
        lillyBoard[bestRow][bestColumn].attributes[0] = 1;
        return actor->nextScript = 10071;
    case 1:
        actor->direction = 1;
        lillyBoard[bestRow][bestColumn].attributes[0] = 1;
        return actor->nextScript = 10077;
    case 2:
        actor->direction = 2;
        lillyBoard[bestRow][bestColumn].attributes[0] = 1;
        return actor->nextScript = 10073;
    case 3:
        actor->direction = 3;
        lillyBoard[bestRow][bestColumn].attributes[0] = 1;
        return actor->nextScript = 10075;
    }
    return 0;
}

/* A hopping lilly actor's script events: 11 sets off for the next square
   (direction: 0 up, 1 right, 2 down, 3 left), 12 puts it halfway, 13-14
   there, 10 and 15 end the hop. */
/* @zoombi32 0x00429d94 */
void hopNotify(View *view, short event)
{
    short *cel = (short *)&view->body;
    LillyActor *actor = (LillyActor *)&view->body;

    switch (event) {
    case 11:
        actor->fromColumn = actor->column;
        actor->fromRow = actor->row;
        switch (actor->direction) {
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
        switch (actor->direction) {
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
        lillyBoard[actor->fromRow][actor->fromColumn].attributes[0] = 0;
        break;
    case 13:
    case 14:
        switch (actor->direction) {
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
        actor->body.waypointX = actor->targetX;
        actor->body.waypointY = actor->targetY;
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
    direction = actor->direction;
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
                switch (actor->attribute) {
                case 1:
                    if (lillyBoard[r][c].attributes[1] != actor->attributeValue)
                        open = 0;
                    break;
                case 2:
                    if (lillyBoard[r][c].attributes[2] != actor->attributeValue)
                        open = 0;
                    break;
                case 3:
                    if (lillyBoard[r][c].attributes[3] != actor->attributeValue)
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
        actor->direction = 0;
        actor->nextScript = hopDirections[0][actor->direction];
        actor->grid[actor->row - 1][actor->column] = current + 1;
        lillyBoard[bestRow][bestColumn].attributes[0] = 1;
        return actor->nextScript;
    case 1:
        actor->direction = 1;
        actor->nextScript = hopDirections[1][actor->direction];
        actor->grid[actor->row][actor->column + 1] = current + 1;
        lillyBoard[bestRow][bestColumn].attributes[0] = 1;
        return actor->nextScript;
    case 2:
        actor->direction = 2;
        actor->nextScript = hopDirections[2][actor->direction];
        actor->grid[actor->row + 1][actor->column] = current + 1;
        lillyBoard[bestRow][bestColumn].attributes[0] = 1;
        return actor->nextScript;
    case 3:
        actor->direction = 3;
        actor->nextScript = hopDirections[3][actor->direction];
        actor->grid[actor->row][actor->column - 1] = current + 1;
        lillyBoard[bestRow][bestColumn].attributes[0] = 1;
        return actor->nextScript;
    case 4:
        actor->direction = 1;
        actor->nextScript = 10031;
        return actor->nextScript;
    default:
        return 0;
    }
    return 0;
}

/*
 * Plans a lilly actor's way across (kind 0) or down (1) to `limit`,
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

    if (!actor->kind)
        reach = actor->column;
    else
        reach = actor->row;
    if (actor->reach == 11) {
        column = actor->column;
        row = actor->row;
    } else {
        column = actor->planColumn;
        row = actor->planRow;
    }
    bestColumn = column;
    bestRow = row;
    heading = actor->direction;
    actor->grid[row][column] = actor->planStep;
    value = actor->planStep;
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
                    if (!actor->kind)
                        reach = c;
                }
                break;
            case 2:
                r++;
                if (r > 11) {
                    r--;
                    open = 0;
                    if (actor->kind == 1)
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
                switch (actor->attribute) {
                case 1:
                    if (lillyBoard[r][c].attributes[1] != actor->attributeValue)
                        open = 0;
                    break;
                case 2:
                    if (lillyBoard[r][c].attributes[2] != actor->attributeValue)
                        open = 0;
                    break;
                case 3:
                    if (lillyBoard[r][c].attributes[3] != actor->attributeValue)
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
                if (!actor->kind) {
                    if (c > reach)
                        reach = c;
                    if (c < actor->planColumn) {
                        actor->planColumn = c;
                        actor->planRow = r;
                    }
                } else {
                    if (r > reach)
                        reach = r;
                    if (r < actor->planRow) {
                        actor->planColumn = c;
                        actor->planRow = r;
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
    actor->planStep = actor->grid[row][column];
    return actor->reach = reach;
}

/* Places a hopping lilly actor's cels through its hop (by frame), its
   parts showing image 0x110 offset by imageOffset and the rest hidden while
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
        actor->fromColumn = actor->column;
        actor->fromRow = actor->row;
        switch (actor->nextScript) {
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
            lillyBoard[actor->fromRow][actor->fromColumn].attributes[0] = 0;
        break;
    case 7:
        if (cel[26])
            shown = cel[26];
        x = actor->targetX;
        y = actor->targetY;
        actor->body.x = x;
        actor->body.y = y;
        actor->body.waypointX = x;
        actor->body.waypointY = y;
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
        *cel += actor->imageOffset;
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

/* Plans a lilly actor's way afresh from where it is. */
/* @zoombi32 0x0042e760 */
void startPlan(LillyActor *actor)
{
    for (short row = 0; row < 12; row++)
        for (short column = 0; column < 12; column++)
            actor->grid[row][column] = 0;
    actor->planColumn = actor->column;
    actor->planRow = actor->row;
    actor->reach = 11;
    actor->planStep = 1;
    planWay(actor, actor->reach);
    planWay(actor, actor->reach);
    followGrid(actor, actor->reach);
    actor->grid[actor->row][actor->column] = actor->planStep;
}

/* Closes the puzzle. */
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
            if (actor->onBoard) {
                actor->grid[flashRow][flashColumn] = 0;
                actor->grid[swapRow][swapColumn] = 0;
                if (lillyBoard[flashRow][flashColumn].attributes[actor->attribute] == actor->attributeValue
                    || lillyBoard[swapRow][swapColumn].attributes[actor->attribute] == actor->attributeValue)
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
            if (actor->onBoard) {
                actor->grid[flashRow][flashColumn] = 0;
                actor->grid[swapRow][swapColumn] = 0;
                if (lillyBoard[flashRow][flashColumn].attributes[actor->attribute] == actor->attributeValue
                    || lillyBoard[swapRow][swapColumn].attributes[actor->attribute] == actor->attributeValue) {
                    startPlan(actor);
                    layers[actor->attributeValue].attribute = actor->attribute;
                    layers[actor->attributeValue].layer = actor->attributeValue;
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

/* The puzzle's clicks: 1 the leave button, 2 the other button, 3 a
   piece picked up and dropped on the board. */
/* @zoombi32 0x00429943 */
void lillyClicked(short action)
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
            if ((snoidsOnTheirWay <= 0 || (snoidsOnTheirWay > 0 && actor->kind == 2)) && !actor->onBoard
                && actor->kind != 1) {
                dragLillyPiece(view, where);
                actor = (LillyActor *)&view->body;
                cel = (short *)&view->body;

                bank = groupBanks[view->body.scriptGroup];
                claimedRow = lillyClaimIndex();
                if (!actor->kind && claimedRow >= 0 && claimedRow <= 11) {
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
                    actor->onBoard = 1;
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
                        actor->padView = snoidPadViews[padsPlaced];
                        padsPlaced++;
                        actor->thirdImageOffset = *(char *)&cel[10];
                    }
                    if (padsPlaced == lillyPartySize) {
                        if (swapToolStage == 6)
                            checkLillyArrivals();
                        snoidsOnTheirWay = 1;
                    }
                } else if (!actor->kind) {
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
 * Drags a piece of the puzzle: a new piece (kind 0) onto a row
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
        if (!actor->kind) {
            cancel = 0;
            lillyDragState = 1;
            actor->body.waypointX = (actor->body.bounds.right - actor->body.bounds.left) / 2 + view->body.cels[0].x;
            actor->body.waypointY = (actor->body.bounds.bottom - actor->body.bounds.top) / 2 + view->body.cels[0].y;
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
            if (!actor->kind)
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
            if (!actor->kind) {
                found = 0;
                for (column = 0; column < placedViewCount; column++)
                    if (ptInRect(&rowEntryRects[column], where) && !lillyBoard[column][0].attributes[0]) {
                        switch (actor->attribute) {
                        case 1:
                            if (lillyBoard[column][0].attributes[1] == actor->attributeValue)
                                found = 1;
                            break;
                        case 2:
                            if (lillyBoard[column][0].attributes[2] == actor->attributeValue)
                                found = 1;
                            break;
                        case 3:
                            if (lillyBoard[column][0].attributes[3] == actor->attributeValue)
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
 * Sets up the puzzle's board: turns and mirrors the level's grids,
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

/* Opens the puzzle (Lilly.MHK) at the level reached. */
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
        actor.onBoard = 0;
        actor.kind = 2;
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
 * The puzzle's idle work, once per pass: ends it when asked, and
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
                moveView(view->id, 0, lillyLayerViews[1]);
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
                if (actor->attribute == 3) {
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
                                    actor->startIndex = nextStart;
                                    actor->attribute = lillyStarts[0].attribute;
                                    actor->attributeValue = lillyBoard[actor->row][actor->column].attributes[actor->attribute];
                                    actor->imageOffset = squareSetC[attributeImageIndex[actor->attribute]] + actor->attributeValue;
                                    parts[25] = 0;
                                    parts[26] = 0;
                                    startPlan(actor);
                                    lillyBoard[actor->row][actor->column].attributes[0] = 1;
                                    actor->onBoard = 1;
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
                                    actor->onBoard = 1;
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
                                    actor->onBoard = 0;
                                } else if (script) {
                                    switch (actor->direction) {
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
