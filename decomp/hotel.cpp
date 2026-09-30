/*
 * hotel (0x424274-0x4281b0): the hotel puzzle (scene 14), 'Hotel.MHK'
 */

#include <stdio.h>
#include <stdlib.h>

#include "zoombinis.h"
#include "debug.h"
#include "e2memory.h"
#include "events.h"
#include "features.h"
#include "focus.h"
#include "graphics.h"
#include "hotel.h"
#include "mainloop.h"
#include "platform.h"
#include "random.h"
#include "snoids.h"
#include "sound.h"
#include "view.h"


Point roomPlaces[25] = {
    {135, 78}, {138, 142}, {142, 204}, {146, 263}, {149, 324}, {223, 84}, {222, 147}, {227, 210},
    {228, 267}, {234, 328}, {315, 88}, {311, 152}, {313, 213}, {314, 272}, {314, 333}, {402, 94},
    {398, 157}, {399, 220}, {397, 277}, {396, 338}, {491, 100}, {489, 164}, {489, 226}, {488, 284},
    {485, 346},
};
Point roomPlaces3d[25] = {
    {16, 40}, {39, 50}, {60, 54}, {86, 58}, {111, 60}, {19, 115}, {42, 125}, {63, 129}, {89, 133},
    {114, 135}, {21, 188}, {44, 198}, {65, 202}, {91, 206}, {116, 208}, {24, 261}, {47, 271},
    {68, 275}, {94, 279}, {119, 281}, {28, 333}, {51, 343}, {73, 347}, {99, 351}, {124, 353},
};
ShortRect hotelBlankRect1 = {138, 293, 345, 351};
ShortRect hotelBlankRect2 = {386, 309, 516, 362};
ShortRect hotelBlankRect3 = {120, 45, 526, 362};
ShortRect hotelBlankRect4 = {11, 1, 638, 396};
short roomColumnDx[5] = {0, 23, 46, 69, 94};
short roomColumnDy[5] = {0, 7, 11, 14, 17};
ImageBank *hotelButtonImages = 0;
Point hotelPlaces[16] = {
    {504, 458}, {467, 453}, {428, 453}, {384, 454}, {344, 451}, {297, 454}, {270, 441}, {244, 453},
    {217, 448}, {188, 453}, {160, 449}, {130, 455}, {103, 446}, {74, 454}, {50, 445}, {17, 453},
};
short hotelButton2Lit = 0;
short hotelButton1Drawn = 0;
short inHotelFrame = 0;
ShortRect viewIdBoxRect = {500, 1, 600, 27};

short roomOccupancy[25];
long hotelFile;
short hotelOpen;
short hotelGoReady;
short hotelAnyFits;
short roomColumnViews[125];
short roomDoorViews[125];
short hotelTalkerView;
short roomAnchorView;
short talkerStage;
short talkerPending;
short debugTalkerScript1;
short debugTalkerScript2;
short hotelLabelView;
short view11800;
short snoidRejected;
short roomGroup;
short heldRoomPlace;
short roomAnimStage;
short hotelLevel;
short firstPlacementFree;
short snoidArriving;
short rowSortFeature;
short columnSortFeature;
short layerSortFeature;
short roomsFilled;
short hotelFails;
short hotelPartySize;
short arrivingSnoid;
short hotelRoom;
short roomCount;
short guideRemarkGroup;
short roundResetGroup;
short talkerGroup;
short talkerDoneGroup;
short arrivingGroup;
short guideView;
short guideStepGroup;
short guideStep;
short guideLastStep;
short savedGuideStep;
short savedGuideLastStep;
short hotelValueCounts[4];
short roomViewScripts[20];
short hotelWalker;
short unusedHotel1;
short talkerStarted;
short skipGuide;
short hotelFacing;
unsigned long hotelIdleSince;
long hotelButtonResource;
long roomImagesResource;
long hotelPlaceXResource;
long hotelPlaceYResource;
long roomViewXResource;
long roomViewYResource;
long roomColumnXResource;
long roomColumnYResource;
long layerRowXResource;
long layerRowYResource;
long roomViewHotXResource;
long roomViewHotYResource;
ImageBank *roomImages;
ImageBank *roomImages3d;
short *hotelPlaceX;
short *hotelPlaceY;
short *roomViewX;
short *roomViewY;
short *roomColumnX;
short *roomColumnY;
short *layerRowX;
short *layerRowY;
short *roomViewHotX;
short *roomViewHotY;
short roomRowValues[25];
short roomLayerValues[25];
short roomViews[25];
short room9002Views[125];
short room9007Views[125];
short droppedSnoid;
ChosenSnoids *hotelChosen;
short standX;
short standY;
ShortRect standArea;
PALETTEENTRY savedPalette[256];

/* @zoombi32 0x00427e1a */
void roomViewNotify(View *view, short event)
{
    switch (event) {
    case 10:
        view->flags |= 0x20000L;
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

/*
 * A room view's update: when reset, lays out the first frame of its script
 * ('SCRB' by its kind), offset by where it stands if it has flag 0x800000,
 * and sets its bounds. The original stores each cel's y in the rectangle's
 * right edge rather than its top, which is kept as written.
 */
/* Not exact: register allocation (the original keeps `left` in edi and
   reads `view` from the stack each time). */
/* @zoombi32 0x00426fd3 */
void layOutRoomView(View *view, short region)
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
            view->body.waypointX = at[1];
            view->body.waypointY = at[2];
            offsetX = view->body.x - view->body.waypointX;
            offsetY = view->body.y - view->body.waypointY;
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
                    *cel++ = rect.left = offsetX + *at++ - roomViewHotX[word];
                    *cel++ = rect.right = offsetY + *at++ - roomViewHotY[word];
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
void addHotelViews()
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
                    roomViews[i] = addView(0x808000, drawRoomView, layOutRoomView, roomViewScripts[count++] + 11004, 0, &place, 0, 0);
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
                roomViews[i] = addView(0x808000, drawRoomView, layOutRoomView, roomViewScripts[count++] + 12000, 0, &place, 0, 0);
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
        freeResource(&roomViewHotXResource);
        freeResource(&roomViewHotYResource);
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
            setViewPlaces(16, hotelPlaces, 1);
            makePartySnoids(0);
            addView(0x1000, drawHotelButtons, updateHotelButtons, 0, 0, 0, 0, 0);
            addHotelViews();
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
            if (viewSnoid(view)->chosen) {
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
        snoid->facingLeft = !snoid->facingLeft;
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
 * Sets up the puzzle: picks which features its rows, columns (and layers)
 * sort by, so that the chosen Zoombinis fit, and at level 2 places some
 * pieces at random in squares no Zoombini can take.
 */
/* @zoombi32 0x00425dde */
void setUpHotelPuzzle()
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
                roomViews[i] = addView(0x808000, drawRoomView, layOutRoomView, roomViewScripts[ok++] + 11000, 0, &place, 0, 0);
            }
    }
    fillMemory(roomRowValues, 0, 50);
    fillMemory(roomLayerValues, 0, 50);
}

/* The puzzle's clicks: 1 the leave button, 2 the other button, 3 a
   Zoombini picked up and dropped on a square. */
/* @zoombi32 0x00426230 */
void hotelClicked(short action)
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
            chosen = snoid->chosen;
            if (snoid->action != 9 && snoid->action != 8 && snoid->action != 7) {
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
                    viewSnoid(view)->chosen = 1;
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
        roomViewHotX = loadShortTable(11002, &roomViewHotXResource);
        roomViewHotY = loadShortTable(11003, &roomViewHotYResource);
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
    setUpHotelPuzzle();
    view11800 = addView(0x100000, drawCels, runViewScript, 11800, 6, 0, 0, 0);
    updateViews();
    roundResetGroup = groupViews(hotelTalkerView, hotelTalkerView, 0, 0, 0, 0);
    showRect(&shownGameRect);
    fadeInViews();
    getColors(&savedPalette[10], 10, 236);
    hotelOpen = 1;
}
