/*
 * town (0x45c0f4-0x45e2d8): the town (scene 0), 'Town.MHK'
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "zoombinis.h"
#include "basecamp.h"
#include "debug.h"
#include "e2memory.h"
#include "events.h"
#include "features.h"
#include "focus.h"
#include "game.h"
#include "graphics.h"
#include "module_4623b8.h"
#include "platform.h"
#include "snoids.h"
#include "sound.h"
#include "town.h"
#include "view.h"

/* Opens scene 0. */
/* @zoombi32 0x0045c12e */
void openIntro()
{
    g_4b7cf4 = g_4b7cf8 = g_4b7cf6 = 0;
    sceneDue = g_4b7cec = 0;
    g_4b7cf0 = 1;
    setGroupLists(townGroups, 1, (short)0xc000);
    g_4b7cf4 = 1;
}

/* Scene 0's clicks: moves g_4a7410 on from 1 to 2 (always, with
   g_4b7cf8); 1 or -1 goes back to the scene sceneToReturnTo picks. */
/* @zoombi32 0x0045c391 */
void introClicked(short which)
{
    if (g_4b7cf8)
        g_4a7410 = 2;
    if (which > 0 && g_4a7410 == 1)
        g_4a7410 = 2;
    if (abs(which) == 1) {
        g_4b7cf0 = g_4b7cf6 = 0;
        sceneDue = sceneToReturnTo();
    }
}

/* Every 1800 ticks, reads the time for the clock: its minute hand
   (0-11, in fives) and hour hand (0-11). */
/* @zoombi32 0x0045c4c9 */
void readClock()
{
    unsigned long now;
    short year;
    char ignored;

    now = clockTime();
    if (now > g_4b7efc + 1800) {
        g_4b7efc = now;
        getDateTime(&year, &ignored, &ignored, (char *)&clockHour, (char *)&clockMinute);
        clockMinute = clockMinute / 5;
        clockHour = clockHour % 12;
    }
}

/* Sets whether the views g_4b7ece and the first g_4b7f02 party views run
   their scripts. */
/* @zoombi32 0x0045ccca */
void setTownRunning(short running)
{
    short i;
    View *view;

    for (i = 0; i <= 19; i++) {
        view = findView(g_4b7ece[i]);
        if (view)
            view->body.running = running;
    }
    for (i = 0; i < g_4b7f02; i++) {
        view = findView(partyViews[i]);
        if (view)
            view->body.running = running;
    }
}

/* A view update: once reset, adds the button to `region`. */
/* @zoombi32 0x0045cf8b */
void updateTownButton(View *view, short region)
{
    if (view->reset) {
        view->reset = 0;
        unionRgnRect(region, &townButtons[0].rect);
    }
}

/* A script from 3000-3002 by gameState's +0x46. */
/* @zoombi32 0x0045d04c */
short townScript()
{
    short script;

    script = ((*(short *)(gameState + 0x46) - 1) & 0xfff) % 3 + 3000;
    if (script < 3000)
        script = 3000;
    if (script >= 3003)
        script = 3002;
    return script;
}

/* A view's placed callback: drops its cels whose image is past g_4b7e10. */
/* @zoombi32 0x0045daf7 */
void placeTownCels(View *view)
{
    ViewCel *cel;

    for (cel = view->body.cels; cel->image;)
        if (cel->image > g_4b7e10)
            removeFirstCel(cel);
        else
            cel++;
}

/* A notify: negates the view's entry in g_4b7ece (the first 19) and counts
   it in g_4b7f10. */
/* @zoombi32 0x0045e29e */
void townsfolkNotify(View *view, short)
{
    short id = view->id;
    short i;

    for (i = 0; i < 19; i++)
        if (id == g_4b7ece[i]) {
            g_4b7ece[i] = -g_4b7ece[i];
            g_4b7f10++;
            break;
        }
}

/* Closes scene 0, leaving its palette and clip for the next scene, and
   reloads the Zoombinis and dialogs. */
/* @zoombi32 0x0045c175 */
void closeIntro()
{
    if (g_4b7cf4) {
        g_4b7cf4 = 0;
        short saved = setFreeAtOnce(1);

        stopMovie(1);
        viewsShown = 1;
        setTakeStatic(1);
        realizePalette(getPortPalette(), 1);
        setClipRect(gameRect);
        discardEvents(3);
        setFreeAtOnce(saved);
        fadeOutViews();
        showBusyCursor();
        loadSnoids(0);
        loadDialogs();
        g_4b2aea = 1;
        showCursor();
    }
}

/* Moves every view with flag 2 a screen (320) left or right, wrapping
   round the town's 1920 pixels; a Zoombini's anchor (unknownAa) keeps its
   place relative to it. */
/* @zoombi32 0x0045ce80 */
void scrollTown(short left)
{
    View *view;
    Snoid *snoid;
    short x;
    short old;

    for (view = viewListEnd(1); view; view = view->next)
        if (view->flags & 2) {
            snoid = viewSnoid(view);
            x = snoid->body.x;
            old = x;
            if (left) {
                x -= 320;
                if (x < -320)
                    x += 1920;
            } else {
                x += 320;
                if (x > 1599)
                    x -= 1920;
            }
            snoid->body.x = x;
            if (snoid->features[0])
                snoid->body.unknownAa += old - x;
            snoid->unknownF5 = -1;
            view->nextUpdate = 0;
        }
}

/* Draws button `which` (only 1), lit or not, and shows it if asked. */
/* @zoombi32 0x0045ceff */
void drawTownButton(short which, short lit, short show)
{
    short image = 0;

    switch (which) {
    case 1:
        image = 5;
        break;
    }
    if (image) {
        if (lit)
            image++;
        drawImageData((unsigned short *)(g_4a74c8->offsets[image] + (char *)g_4a74c8), townButtons[which - 1].rect.left,
                      townButtons[which - 1].rect.top, 8);
        if (show)
            showRect(&townButtons[which - 1].rect);
    }
}

/* Closes scene 6. */
/* @zoombi32 0x0045cfae */
void closeTown()
{
    if (g_4b7e00) {
        g_4b7e00 = 0;
        short saved = setFreeAtOnce(1);

        useAltSnoids(1);
        if (!viewsLocked) {
            viewsLocked = 1;
            *(short *)(gameState + 0xa92e) = 0;
            *(short *)(gameState + 0xa930) = 1;
            *(short *)(gameState + 0xa932) = 1;
        }
        clearViews();
        unloadSounds();
        freeResource(&g_4a74c4);
        setFreeAtOnce(saved);
        closeGameFile(&g_4b7dfc);
        fadeOutViews();
        showBusyCursor();
        g_4a74dc = -1;
    }
}

/* Finds the hotspot under the cursor (into *where): the first of the
   g_4b7eb2 non-empty rectangles g_4b7e12 holding it sets g_4b7eb4, its
   number (g_4b7e92, from 1) in g_4b7eb6 and a script by it in g_4b7eb8. */
/* @zoombi32 0x0045d715 */
void findTownHotspot(Point *where)
{
    short i;

    getCursorPosition(where);
    g_4b7eb4 = 0;
    for (i = 0; !g_4b7eb4 && i < g_4b7eb2; i++)
        if (!emptyRect(&g_4b7e12[i]) && ptInRect(&g_4b7e12[i], *where)) {
            i = g_4b7e92[i];
            g_4b7eb8 = g_4a7582[i] + 1003;
            g_4b7eb6 = i + 1;
            g_4b7eb4 = 1;
        }
}

/* Scene 0's keys: Ctrl-Q quits; any other key clicks. */
/* @zoombi32 0x0045c0f4 */
short introKey(unsigned short key)
{
    switch (key) {
    case 0x1b:
    case ' ':
    default:
        introClicked(1);
        return 1;
    case 0x11:
        closeIntro();
        quitRequested = -1;
        return 1;
    }
}

/* Resets scene 6's state; the pace g_4b7f08 by g_4b2b00. */
/* @zoombi32 0x0045c3ec */
void resetTown()
{
    short i;

    g_4b7eba = 0;
    for (i = 0; i < 4; i++)
        g_4b7e08[i] = 0;
    g_4b7f00 = g_4b7f02 = g_4b7f10 = 0;
    g_4b7f04 = 0;
    g_4b7f0c = 0;
    if (g_4b2b00)
        g_4b7f08 = 600;
    else
        g_4b7f08 = 120;
    g_4b7ec8 = g_4b7eca = g_4b7ecc = 0;
    g_4b7eb6 = g_4b7eb8 = g_4b7ec4 = g_4b7ec6 = 0;
    g_4a74dc = -1;
    g_4b7ef8 = 0;
    g_4b7ec0 = 0;
    for (i = 0; i <= 19; i++)
        g_4b7ece[i] = 0;
    g_4b7efc = 0;
    readClock();
    g_4b7ef6 = 0;
    g_4b7f12 = 0;
}

/* A view draw: draws the button, unlit. */
/* @zoombi32 0x0045cf79 */
void drawTownButtons(View *)
{
    drawTownButton(1, 0, 0);
}

/* Moves the Zoombinis who arrived (the travellers, countPresentTravellers of them) into
   the town's free slots, up to 625. */
/* @zoombi32 0x0045dfb1 */
void settleTravellers()
{
    short arrived;
    short found;
    short i;
    short j;

    arrived = countPresentTravellers();
    found = 0;
    for (i = 624; !found && i >= 0; i--)
        if (townSlots->slots[i].zoombini)
            found = 1;
    if (found)
        ;
    for (j = 0, i = 0; j < arrived && i < 625 && townSlots->count < 625; i++)
        if (!townSlots->slots[i].zoombini) {
            townSlots->count++;
            townSlots->slots[i].zoombini = travellers()[j].zoombini;
            strcpy(townSlots->slots[i].name, travellers()[j].name);
            j++;
        }
}

/* The clock's view (two cels, its hands): hidden while g_4a74dc is set
   and unless g_4b7ef8 and on screen 1 or 2 (townScreen). Shows the time
   (readClock), or with g_4b7ef6 winds the hands round that many times
   (faster, from where they are). */
/* @zoombi32 0x0045cd27 */
void drawClock(View *view)
{
    ViewCel *cels = view->body.cels;

    cels[0].image = 0;
    if (g_4a74dc > 0) {
        view->changed = 0;
        return;
    }
    if (g_4b7ef8 <= 0)
        return;
    if (townScreen() != 1 && townScreen() != 2)
        return;
    if (g_4b7ef6) {
        if (g_4b7ef6 < 0) {
            g_4b7f14 = clockMinute;
            g_4b7f15 = clockHour;
            g_4b7ef6 = abs(g_4b7ef6);
            view->interval = 2;
        } else {
            clockMinute++;
            if (clockMinute > 11) {
                clockMinute = 0;
                clockHour++;
                if (clockHour > 11)
                    clockHour = 0;
            }
            if (g_4b7f14 == clockMinute && g_4b7f15 == clockHour) {
                g_4b7ef6--;
                if (!g_4b7ef6) {
                    view->interval = 6;
                    g_4b7efc = 0;
                    readClock();
                }
            }
        }
    } else {
        readClock();
    }
    cels[0].image = clockMinute + 1;
    cels[1].image = clockHour + 13;
    cels[0].y = cels[1].y = 218;
    if (townScreen() == 1)
        cels[0].x = cels[1].x = 626;
    else
        cels[0].x = cels[1].x = 307;
}

/* An image's data in a bank. */
inline unsigned short *bankImage(ImageBank *bank, short image)
{
    return (unsigned short *)((char *)bank + bank->offsets[image]);
}

/* A view's placed callback: its cels 7-22 are the groups' records (by
   recordGroups): those set become hotspots (g_4b7e12, 56 by 28 about
   g_4a74de's point for the group plus the cel's; 4 also sets g_4b7ef8),
   the others are dropped. */
/* @zoombi32 0x0045db25 */
void placeRecordHotspots(View *view)
{
    ShortRect rect;
    ImageBank *volatile bank = groupBanks[view->body.scriptGroup];
    short *cel = (short *)view->body.cels;
    short n;

    g_4b7eb2 = g_4b7ef8 = 0;
    while (*cel)
        if (*cel >= 7 && *cel <= 22) {
            n = *cel - 7;
            if (recordGroups()[n]) {
                g_4b7e92[g_4b7eb2] = n;
                if (n == 4)
                    g_4b7ef8 = 1;
                bankImage(bank, *cel); /* unused, as in the original */
                cel++;
                rect.left = g_4a74de[n].x + *cel - 28;
                cel++;
                rect.right = rect.left + 56;
                rect.top = g_4a74de[n].y + *cel - 14;
                cel++;
                rect.bottom = rect.top + 28;
                g_4b7e12[g_4b7eb2] = rect;
            } else {
                g_4b7e12[g_4b7eb2] = noRect;
                g_4b7e92[g_4b7eb2] = -1;
                removeFirstCel((ViewCel *)cel);
            }
            g_4b7eb2++;
        } else {
            cel += 3;
        }
}

/* Scene 0's frame: plays the logo movie (Data\Logo025.MOV) once, then
   clicks; leaves when asked (sceneDue). */
/* @zoombi32 0x0045c212 */
void introFrame()
{
    if (g_4a7412 || !g_4b7cf4)
        return;
    g_4a7412 = 1;
    if (movieShowing && !g_4b7cf8) {
        if (idleMovie() == 1)
            g_4b7cf0 = 1;
    } else if (g_4b7cf8 || !movieShowing && g_4b7cf6) {
        g_4b7cf6 = 0;
        sceneDue = sceneToReturnTo();
    }
    if (sceneDue) {
        pendingScene = sceneDue;
        sceneDue = 0;
        setCurrentMap(0);
        closeIntro();
    } else if (g_4b7cf0) {
        switch (g_4b7cec) {
        case 0:
            g_4b7cf0 = 0;
            g_4b7cec++;
            if (!movieShowing) {
                g_4a7410 = 1;
                logoPath[0] = 0;
                strcpy(logoPath, installDir);
                strcat(logoPath, "Data\\");
                strcat(logoPath, "Logo025.MOV");
                if (playMovie(logoPath)) {
                    introClicked(1);
                } else {
                    g_4b7cf6 = 1;
                    hideCursor();
                }
            }
            break;
        case 1:
            g_4b2ad6 = 0;
            g_4b7cec++;
            introClicked(-1);
            break;
        }
    }
    g_4a7412 = 0;
}

/* A monument's plaque (a view draw; its interval holds the record shown,
   from 1, or g_4b7eba the one picked): its cels, then five lines about the
   record: the building's dedication, the group's feat, "when traveling
   was", the level, and the date, in colours by the plaque (its kind,
   1004-1007). */
/* The original draws each cel with drawImageData(image, *cel++, *cel++, 8),
   which relies on BCC's left-to-right argument order (unspecified in C++),
   so the loop here indexes and then steps. (It also keeps `i` in edi, as
   `bank` was, not on the stack.) */
/* @zoombi32-functional 0x0045dc5b */
void drawPlaque(View *view)
{
    Font *oldFont;
    Color saved;
    ShortRect rect;
    short building;
    short level;
    short dy;
    short outline;
    short color;
    char text[256];
    short i;
    short line;

    if (!view->interval)
        return;
    oldFont = setFont(fonts[1]);
    saved = setForeColor(Color(10));
    switch (view->kind) {
    case 1004:
    default:
        dy = 20;
        outline = 199;
        color = 45;
        break;
    case 1005:
        dy = 22;
        outline = 199;
        color = 45;
        break;
    case 1006:
        dy = 14;
        outline = 212;
        color = 45;
        break;
    case 1007:
        dy = 14;
        outline = 45;
        color = 205;
        break;
    }
    {
        short *cel = (short *)view->body.cels;
        ImageBank *bank = groupBanks[view->body.scriptGroup];

        while (*cel && bank->count >= *cel) {
            drawImageData(bankImage(bank, cel[0]), cel[1], cel[2], 8);
            cel += 3;
        }
    }
    i = (view->interval - 1) & 0xf;
    building = monumentBuildings[i];
    if (g_4b7eba) {
        i = g_4b7eba - 1;
        building = monumentBuildings[g_4b7eb6 - 1];
    }
    rect.left = view->body.bounds.left;
    rect.right = view->body.bounds.right;
    level = recordLevels()[i];
    for (line = 0; line < 5; line++) {
        rect.top = view->body.bounds.top + dy + plaqueLines[line];
        rect.bottom = view->body.bounds.top + dy + plaqueLines[line + 1];
        switch (line) {
        case 0:
            drawOutlinedText(outline, color, rect, 0x22, monumentTexts[building]);
            break;
        case 1:
            setFont(fonts[2]);
            drawOutlinedText(outline, color, rect, 0x22,
                             featTexts[((recordGroups()[i] - 1) * 4 + recordLevels()[i] - 1) & 0xf]);
            break;
        case 2:
            setFont(fonts[1]);
            drawOutlinedText(outline, color, rect, 0x22, levelTexts[23]);
            break;
        case 3:
            setFont(fonts[2]);
            drawOutlinedText(outline, color, rect, 0x22, levelTexts[level + 1]);
            break;
        case 4:
            setFont(fonts[1]);
            sprintf(text, "%s %d, %d", levelTexts[(unsigned char)recordMonths()[i] + 10],
                    (short)(unsigned char)recordDays()[i], recordYears()[i]);
            drawOutlinedText(outline, color, rect, 0x22, text);
            break;
        }
    }
    view->interval = 0;
    setForeColor(saved);
    setFont(oldFont);
}

/* While g_4b7f12 allows, adds a townsperson (a Zoombini view of script
   8000-8043, half from each half) walking at a random place, the height
   by the script, into a free one of g_4b7ece's last three. */
/* @zoombi32 0x0045e06e */
void addTownsperson()
{
    Snoid snoid;
    short y;
    short id;
    short script;
    short i;
    View *view;

    if (!g_4b7f12 || dialogFlags || g_4a74dc != -1)
        return;
    initSnoid(&snoid);
    snoid.features[0] = 1;
    snoid.features[1] = 1;
    snoid.features[2] = 1;
    snoid.features[3] = 1;
    if (randomBetween(0, 100) <= 50)
        script = randomBetween(8022, 8043);
    else
        script = randomBetween(8000, 8021);
    if (script <= 8007)
        y = randomBetween(170, 280);
    else if (script <= 8009)
        y = randomBetween(40, 280);
    else if (script <= 8017)
        y = randomBetween(110, 260);
    else if (script <= 8021)
        y = randomBetween(-10, 100);
    else if (script <= 8029)
        y = randomBetween(230, 310);
    else if (script <= 8031)
        y = randomBetween(140, 290);
    else if (script <= 8039)
        y = randomBetween(190, 280);
    else if (script <= 8043)
        y = randomBetween(100, 200);
    for (i = 0; i < 3; i++)
        if (!g_4b7ece[16 + i]) {
            id = addView(1, drawCels, runViewScript, script, 6, &snoid, 0, 0);
            view = findView(id);
            if (view) {
                g_4b7ece[16 + i] = id;
                viewSnoid(view)->features[0] = 0;
                view->flags = 0x908002;
                view->body.x = randomBetween(100, 540);
                view->body.y = y;
                *(long *)&view->body.unknownAa = *(long *)&view->body.x;
                view->notify = townsfolkNotify;
                view->notifyEnd = 1;
                moveView(id, 0, g_4b7e0e);
                i = 3;
                if (g_4b7f12 > 0)
                    g_4b7f12--;
            }
        }
}

/* Shows frame `frame` of the four views g_4b7e08 at once. */
/* @zoombi32 0x0045da4e */
void setTownFrames(short frame)
{
    short i;
    View *view;
    short value;

    for (i = 0; i < 4; i++) {
        view = findView(g_4b7e08[i]);
        if (view) {
            runViewCels(view, removedRgn);
            value = frame;
            view->body.frameOffset = scriptFrameOffset(scripts[view->body.script], &value, 0);
            view->body.frame = value;
            view->nextUpdate = 0;
            view->body.running = 1;
            view->body.lastFrame++;
            view->changed = 1;
            runViewCels(view, removedRgn);
            view->body.lastFrame--;
            view->body.frame = value;
        }
    }
}

/* Scene 6's keys (with debugging on, debugMessagesOn): z and x (once the last
   group has a record) step the plaque shown by cheat (g_4b7eba, 0-16) and
   space reports it; F toggles the townspeople; . records the next group
   passed (g_4a7592 counting through the groups and levels) now; 0 clears
   the records; 0x125 and 0x127 raise and lower the highest cel shown
   (g_4b7e10, 25-81). Returns whether the key was used. */
/* @zoombi32 0x0045d7a9 */
short townKey(unsigned short key)
{
    char hour;
    char minute;
    short used = 0;
    View *view;

    if (!debugMessagesOn)
        return used;
    switch (key) {
    case 'x':
    case 'z':
        if (recordGroups()[15]) {
            if (key == 'z')
                g_4b7eba++;
            if (key == 'x')
                g_4b7eba--;
            if (g_4a74dc > 0) {
                view = findView(g_4a74dc);
                if (view) {
                    view->interval = g_4b7eb6;
                    view->changed = 1;
                }
            }
        }
    case ' ':
        if (g_4b7eba > 16)
            g_4b7eba = 16;
        if (g_4b7eba < 0)
            g_4b7eba = 0;
        if (g_4b7eba)
            debugMessage(g_4b7eba, "Cheat Text:", 0, 0, 0);
        else
            debugMessage(-1, "Cheat Text OFF", 0, 0, 0);
        break;
    case 'F':
        if (!g_4b7f12)
            g_4b7f12 = 100;
        else
            g_4b7f12 = 0;
        break;
    case '.':
        for (used = 0; used < 16; used++)
            if (!recordGroups()[used]) {
                getDateTime(&recordYears()[used], &recordMonths()[used], &recordDays()[used], &hour, &minute);
                recordGroups()[used] = ((g_4a7592 / 4) & 3) + 1;
                recordLevels()[used] = (g_4a7592 & 3) + 1;
                used = 16;
                setTownFrames(townScreen());
                g_4a7592++;
            }
        used = 1;
        break;
    case 0x125:
    case 0x127:
        switch (key) {
        case 0x125:
            g_4b7e10 += 5;
            break;
        case 0x127:
            g_4b7e10 -= 5;
            break;
        }
        if (g_4b7e10 > 80)
            g_4b7e10 = 81;
        if (g_4b7e10 < 24)
            g_4b7e10 = 25;
        setTownFrames(townScreen());
        used = 1;
        break;
    case '0':
        g_4a7592 = 0;
        for (used = 0; used < 16; used++)
            recordGroups()[used] = 0;
        setTownFrames(townScreen());
        g_4b7ef8 = 0;
        used = 1;
        break;
    }
    return used;
}

/* Scene 6's clicks: any click closes an open plaque. 1 leaves; 2 (the
   town) winds the clock when on its view (g_4b7ece[0]), drags a Zoombini
   (who stays where dropped on the ground, y 410-475), opens the plaque
   of the hotspot under the cursor, or at the sides scrolls to the next or
   previous of the six screens. */
/* @zoombi32 0x0045d468 */
void townClicked(short which)
{
    Point where;
    View *view;
    Snoid *snoid;

    if (g_4a74dc > 0) {
        deleteView(g_4a74dc);
        g_4a74dc = -1;
        setTownRunning(1);
        g_4b7eb4 = 0;
        return;
    }
    switch (which) {
    case 1:
        queueViewSound(999, 0);
        drawTownButton(which, 1, 1);
        waitForEventFor(0, 2, 0, 1);
        drawTownButton(which, 0, 1);
        pendingScene = 1;
        setCurrentMap(0);
        closeTown();
        break;
    case 2:
        getCursorPosition(&where);
        view = viewAt(where, 0x808002, 1);
        if (view && view->id == g_4b7ece[0] && !g_4b7ef6)
            g_4b7ef6 = -1;
        view = viewAt(where, 2, 1);
        if (view && view->flags == 2) {
            setDragCursor(0);
            g_4b7eca = 1;
            dragSnoid(view, where, 0, 0);
            g_4b7552 = 0;
            g_4b7eca = 0;
            snoid = viewSnoid(view);
            if (snoid->body.y >= 410 && snoid->body.y <= 475)
                *(long *)&snoid->targetX = *(long *)&snoid->body.x;
        } else if (!g_4b7eb4) {
            if (where.y > 30 && where.y < 450 && where.x > 3 && where.x < 637) {
                if (where.x > 560) {
                    queueViewSound(999, 0);
                    townScreen()++;
                    if (townScreen() > 5)
                        townScreen() = 0;
                    setTownFrames(townScreen());
                    scrollTown(1);
                    g_4b7ef8 = 0;
                } else if (where.x < 80) {
                    queueViewSound(999, 0);
                    townScreen()--;
                    if (townScreen() < 0)
                        townScreen() = 5;
                    setTownFrames(townScreen());
                    scrollTown(0);
                    g_4b7ef8 = 0;
                }
            }
        } else if (g_4a74dc == -1) {
            g_4a74dc = 0;
            setDragCursor(0);
            queueViewSound(999, 0);
            updateViews();
            updateViews();
            g_4a74dc = addView(0x5000, drawPlaque, runViewCels, g_4b7eb8, g_4b7eb6, 0, 0, 0);
            setTownRunning(0);
            waitForEventFor(0, 2, 0, 1);
        }
        break;
    }
}

/* Opens scene 6, Zoombiniville: adds the travellers to the population
   (g_4b7ecc once it reaches 625) and the town's slots, sets up the four
   town views (the highest cel shown, g_4b7e10, by the population), walkers
   for every 37 over 20 (up to 16), the last 20 Zoombinis to settle, the
   button and the clock, scrolls to the screen last shown, and picks the
   first sound (a hint, a greeting, or 3003 when the town is full) and how
   many townspeople to add by the population. */
/* Not exact: register allocation (the original computes `highest` in ebx,
   the register `i` has later; here it is in ecx, whatever the declaration
   order or scope). */
/* @zoombi32 0x0045c52e */
void openTown()
{
    unsigned long used;
    short extras;
    short slot;
    Snoid snoid;
    unsigned long highest;
    short last;
    short i;
    View *view;
    short id;

    g_4b7e00 = 0;
    resetTown();
    soundRanges = 0;
    addSoundRange(3000, 3003, 1);
    addSoundRange(20000, 29999, 1);
    addSoundRange(996, 997, 0);
    openGameFile(&g_4b7dfc, "Town.MHK");
    setCurrentMap(g_4b7dfc);
    useAltSnoids(0);
    population() += countPresentTravellers();
    if (population() >= 625)
        g_4b7ecc = 1;
    townSlots = (Camp *)(gameState + 0x6c42);
    settleTravellers();
    party()->count = 0;
    last = -1;
    for (i = 0; last < 0 && i < 625; i++)
        if (!townSlots->slots[i].zoombini)
            last = i;
    if (last < 0)
        last = 0;
    highest = population() * 56;
    highest = highest / 625 + 1;
    if (highest > 56)
        highest = 56;
    g_4b7e10 = highest + 24;
    drawBackdrop(1200);
    loadFeatureGroup(1000, 0, 0);
    loadScripts(1000, 8);
    g_4a74c8 = loadImageBank(1100, &g_4a74c4);
    loadDragCursors(2000);
    loadFeatureGroup(4000, 1, 0);
    addScripts(4000, 8, 0);
    loadSnoidScripts(4999, 1, 0);
    addSnoidScripts(5000, 5, 0);
    loadFeatureGroup(6000, 2, 1);
    addScripts(6000, 1, 0);
    loadFeatureGroup(8000, 3, 0);
    addScripts(8000, 44, 1);
    g_4b7e08[0] = addView(0x402c000, drawCelsOpaque, runViewCels, 1000, 0, 0, 0, 0);
    g_4b7e08[1] = addView(0xc02c000, drawCels, runViewCels, 1002, 0, 0, 0, 0);
    g_4b7e08[2] = addView(0xc02c000, drawCels, runViewCels, 1003, 0, 0, 0, 0);
    g_4b7e08[3] = addView(0xc02c000, drawCels, runViewCels, 1001, 0, 0, 0, 0);
    for (i = 1; i <= 3; i++) {
        view = findView(g_4b7e08[i]);
        if (view)
            switch (i) {
            case 1:
            case 2:
                view->placed = placeTownCels;
                break;
            case 3:
                view->placed = placeRecordHotspots;
                break;
            }
    }
    {
        Point places[16] = {{467, 265}, {349, 225}, {777, 291}, {828, 284}, {44, 330}, {283, 152},
                            {195, 211}, {607, 201}, {1182, 287}, {1299, 228}, {1422, 269}, {1807, 316},
                            {1048, 309}, {709, 228}, {1740, 284}, {1532, 172}};
        short walkers[16] = {4000, 4001, 4002, 4003, 4004, 4005, 4006, 4007,
                             4000, 4001, 4002, 4003, 4004, 4005, 4006, 4007};
        short n;

        used = 0;
        n = population() - 20;
        if (n < 0)
            n = 0;
        if (n > 605)
            n = 605;
        extras = n / 37;
        if (extras < 0)
            extras = 0;
        if (extras > 16)
            extras = 16;
        i = 0;
        initSnoid(&snoid);
        snoid.features[0] = 1;
        snoid.features[1] = 1;
        snoid.features[2] = 1;
        snoid.features[3] = 1;
        if (extras)
            do {
                slot = allocateSlot(&used, 16, 0);
                g_4b7ed0[i] = addView(1, drawCels, runViewScript, walkers[slot], randomBetween(4, 6), &snoid, 0, 0);
                view = findView(g_4b7ed0[i]);
                if (view) {
                    viewSnoid(view)->features[0] = 0;
                    view->flags = 0x808002;
                    *(Point *)&view->body.x = places[slot];
                    *(Point *)&view->body.unknownAa = places[slot];
                }
                i++;
                extras--;
            } while (extras);
    }
    i = townScreen();
    if (i < 0 || i > 5)
        townScreen() = i = 0;
    if (i)
        do {
            scrollTown(1);
            i--;
        } while (i);
    g_4b7f02 = 0;
    last--;
    if (last >= 0) {
        initSnoid(&snoid);
        for (i = 0; last >= 0 && i < 20 && i < population(); last--, i++) {
            snoid.zoombini = townSlots->slots[last].zoombini;
            strcpy(snoid.name, townSlots->slots[last].name);
            snoid.body.x = randomBetween(-320, 1599);
            snoid.body.y = randomBetween(410, 475);
            id = addSnoidView(&snoid, 0);
            view = findView(id);
            if (view) {
                partyViews[g_4b7f02] = id;
                g_4b7f02++;
                view->flags &= ~1;
                view->flags |= 2;
            }
        }
    }
    addView(0x1000, drawTownButtons, updateTownButton, 0, 0, 0, 0, 0);
    setTownFrames(townScreen());
    g_4b7ece[0] = addView(0x8001, drawCels, runViewCels, 6000, 6, &snoid, 0, 0);
    view = findView(g_4b7ece[0]);
    if (view) {
        view->flags &= ~1;
        view->flags |= 2;
        view->placed = drawClock;
    }
    copyPaletteRange(1, 254);
    updateViews();
    setGroupLists(townGroups6, 1, (short)0xc000);
    drawTownButton(1, 0, 0);
    showRect(&shownGameRect);
    fadeInViews();
    g_4b7e00 = 1;
    i = 0;
    if (puzzleLeft) {
        puzzleLeft = 0;
        i = campHint((short *)(gameState + 0x46));
        if (i == 2 && population() <= 16) {
            i = 1;
            *(short *)(gameState + 0x46) &= 0xcfff;
        }
    }
    switch (i) {
    case 1:
        switch (*(short *)(gameState + 0x46)) {
        case 1:
            g_4b7ec4 = 20086;
            break;
        case 2:
            g_4b7ec4 = 20087;
            break;
        case 3:
            g_4b7ec4 = 20088;
            break;
        default:
            switch (randomBetween(1, 3)) {
            case 1:
                g_4b7ec4 = 20086;
                break;
            case 2:
                g_4b7ec4 = 20087;
                break;
            case 3:
                g_4b7ec4 = 20088;
                break;
            }
            break;
        }
        break;
    case 2:
    case 12:
        switch (randomBetween(1, 2)) {
        case 1:
            g_4b7ec4 = 20087;
            break;
        case 2:
            g_4b7ec4 = 20088;
            break;
        }
        break;
    case 5:
        g_4b7ec4 = 20086;
        break;
    default:
        g_4b7ec4 = townScript();
        g_4b7ec6 = 1;
        break;
    }
    if (g_4b7ecc) {
        g_4b7ec4 = 3003;
        g_4b7ec6 = 1;
    }
    if (g_4b7ec4) {
        queueViewSound(g_4b7ec4, 0);
        g_4b7ec8 = 1;
    }
    resetViewClock();
    g_4b7ebc = 0;
    g_4b7562 = 0;
    if (g_4b7ecc) {
        g_4b7f12 = 20;
    } else {
        if (population() > 100)
            g_4b7f12++;
        if (population() > 200)
            g_4b7f12 += 2;
        if (population() > 300)
            g_4b7f12 += 3;
        if (population() > 400)
            g_4b7f12 += 4;
        if (population() > 500)
            g_4b7f12 += 5;
    }
}

/* Scene 6's frame: deletes the townspeople who have walked off, adds
   more, leaves when asked (sceneDue); every 150-300 ticks after a sound
   ends plays the next (the greetings 3000-3002 in turn, or at random one
   of g_4a74cc, not 20093 once over 600 live here); now and then has one
   of the settled Zoombinis on screen do something (g_4b7f00 times, more
   as the town grows); and sets the cursor by what it's over (a hotspot,
   the sides to scroll). */
/* Not exact: register allocation (the original keeps `n`, the loop
   index and flags, in esi and `i`, the sound and the tries, in ebx; here
   they're the other way round, whichever is declared first). */
/* @zoombi32 0x0045d07e */
void townFrame()
{
    Point where;
    short n;
    short i;
    View *view;

    if (g_4a7580 || !g_4b7e00)
        return;
    g_4a7580 = 1;
    updateViews();
    if (g_4b7f10)
        for (n = 0; n < 19; n++)
            if (g_4b7ece[n] < 0) {
                deleteView(-g_4b7ece[n]);
                g_4b7ece[n] = 0;
                if (g_4b7f10 > 0)
                    g_4b7f10--;
            }
    addTownsperson();
    if (sceneDue) {
        pendingScene = sceneDue;
        sceneDue = 0;
        setCurrentMap(0);
        closeTown();
        g_4a7580 = 0;
        return;
    }
    if (!isSoundPlaying(g_4b7ec4, RESOURCE_TYPE(0, 'S', 'N', 'D')) && !dialogFlags) {
        if (g_4b7ec8) {
            g_4b7ec0 = clockTime();
            g_4b7ebc = randomBetween(150, 300);
            g_4b7ec8 = 0;
            if (g_4b7ecc)
                g_4b7f12 = 40;
        }
        if (clockTime() - g_4b7ec0 > g_4b7ebc) {
            g_4b7ec0 = clockTime();
            if (!g_4b7ec6 || g_4b7ecc) {
                if (g_4b7ec4 < 20000) {
                    g_4b7ec4++;
                    if (g_4b7ec4 >= 3003)
                        g_4b7ec4 = 3000;
                } else {
                    g_4b7ec4 = townScript();
                }
                g_4b7ec6 = 1;
                i = g_4b7ec4;
            } else {
                g_4b7ec6 = 0;
                for (n = 1; n;) {
                    n = 0;
                    i = g_4a74cc[allocateSlot(&g_4a74d8, 5, 0)];
                    if (i == 20093 && population() > 600)
                        n = 1;
                    g_4b7ec4 = i;
                }
            }
            queueViewSound(i, 0);
            g_4b7ec8 = 1;
        }
    }
    if (g_4b7f02 && !dialogFlags && g_4a74dc == -1) {
        if (g_4b7f00 > 0) {
            if (clockTime() - g_4b7f04 > g_4b7f08) {
                n = 0;
                g_4b7f04 = clockTime();
                i = 0;
                do {
                    i++;
                    view = idleSnoidView(partyViews[allocateSlot(&g_4b7f0c, g_4b7f02, 0)]);
                    if (view && (view->flags & 2) && view->body.x > 20 && view->body.x < 620) {
                        startSnoidScript(viewSnoid(view), viewSnoid(view)->features[3] + 4999, 0, 0);
                        g_4b7f00--;
                        n = 1;
                    }
                } while (!n && i < 16);
            }
        } else if (population() == 625) {
            g_4b7f00 = 8;
        } else if (population() > 312) {
            g_4b7f00 = 6;
        } else if (population() > 156) {
            g_4b7f00 = 4;
        } else if (population() > 156) {
            g_4b7f00 = 2;
        } else if (population()) {
            g_4b7f00 = 1;
        }
    }
    if (!dialogFlags && !g_4b7eca && g_4a74dc == -1) {
        findTownHotspot(&where);
        if (!ptInRect(&townButtons[0].rect, where) && where.y > 30 && where.y < 450 && where.x > 3
            && where.x < 637) {
            if (g_4b7eb4)
                setDragCursor(3);
            else if (where.x > 560)
                setDragCursor(2);
            else if (where.x < 80)
                setDragCursor(1);
            else
                setDragCursor(0);
        } else {
            setDragCursor(0);
        }
    }
    g_4a7580 = 0;
}
