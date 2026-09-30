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
#include "mainloop.h"
#include "platform.h"
#include "snoids.h"
#include "sound.h"
#include "town.h"
#include "view.h"

char monumentBuildings[16] = {7, 1, 4, 5, 13, 3, 10, 11, 14, 15, 0, 8, 12, 9, 2, 6};
char *monumentTexts[16] = {
    "this monument was made to\rhonor the zoombinis who:",
    "this windmill was wrought\rto honor the zoombinis who:",
    "this observatory observes\rthe zoombinis who:",
    "this bowling alley\rhonors the zoombinis who:",
    "this general store was\rerected for the zoombinis who:",
    "this swimming pool\rsalutes the zoombinis who:",
    "this playground was pitched\rto honor the zoombinis who:",
    "this band shell was built to\rhonor the zoombinis who:",
    "this schoolhouse salutes\rthe zoombinis who:",
    "this library was raised to\rhonor the zoombinis who:",
    "this firehouse honors\rthe zoombinis who:",
    "this opera house sings\rpraises to the zoombinis who:",
    "this city hall celebrates\rthe zoombinis who:",
    "this clock tower was\rconstructed for the zoombinis who:",
    "this paper clip museum was made\rfor the zoombinis who:",
    "this courthouse was constructed\rfor the zoombinis who:",
};
char *featTexts[16] = {
    "ambled past allergic cliffs,\rcruised on by\rstone cold caves,\rand\rappeased arno the\ralmost omnivorous",
    "braved blustery bridges,\routsmarted onyx's\rstone faced crew,\rand\rwon over willomaen\rthe pizza eating troll",
    "bested bridge watchers,\rcrept cautiously\rpast cave guards, \rand\rsatiated shyler the\rpizza loving troll",
    "outsmarted sneezing cliffs, conquered crusty\rcave guards,\rand\rplacated picky pizza trolls\rwithout hearing\r\"Yuck!\"",
    "calmed captain cajun,\rrode tattooed toads,\rand\rknew how to network",
    "finagled the ferryboat,\rsuccessfully swapped\rlily pads,\rand\rconnected the current",
    "finessed the ferryboat,\rcrept cautiously past\rlily pad crabs,\rand\rsurmounted\rstone elevators",
    "calmed captain cajun,\rrode tattooed toads,\rand\rknew how to network",
    "flushed the finicky fleens,\rin hotel dimensia had\rpleasant dreams\rand\rcatapulted cleanly\rover Mudball Wall",
    "flustered the fleens,\rdidn't dally at\rhotel dimensia,\rand\rmastered the\rmudball making machine",
    "sent the fleens flying,\rwrangled with\rransacked rooms,\rand\rvaulted the wall\rwith hardly a fall",
    "finally foiled the fleens,\rresolved the hotel\rrooming scene,\rand\rmastered the\rmudball wall machine",
    "did not lag in lion's lair,\rsolved the secrets of\rthe mirror machine,\rand\rflew above\rbubblewonder abyss",
    "overcame their fear\rin lion's lair,\rhad things go fine\rin the mirror machine,\rand\rrode a wonderous\rbubble ship",
    "deciphered the lion's logic,\rcorrectly calculated\rthe crystals,\rand\rascended the airy abyss",
    "raised up high the lion's paw,\rlined up the right crystals\rthat they clearly saw,\rand\rmastered bubblewonder\rwithout\rfalling in its maw",
};
const char *toggleTexts[15] = {
    "*", "music on", "music off", "sound on", "sound off", "less action", "more action",
    "hide cursor", "show cursor", "sticky mouse", "non-sticky mouse", "transitions on",
    "transitions off", "auto sticky on", "auto sticky off",
};
char introClickState = 0;
short inIntroFrame = 0;
SceneButton townButtons[1] = {{{600, 403, 639, 440}}};
long townButtonResource = 0;
ImageBank *townButtonImages = 0;
short townSounds[5] = {0x4e79, 0x4e7a, 0x4e7b, 0x4e7c, 0x4e7d};
unsigned long townSoundsUsed = 0;
short townDialogView = -1;
Point recordHotspotPoints[16] = {
    {72, 70}, {118, 126}, {81, 118}, {77, 63}, {65, 227}, {144, 83}, {107, 81}, {144, 186},
    {49, 97}, {57, 148}, {86, 46}, {67, 119}, {108, 80}, {76, 97}, {47, 181}, {119, 65},
};
unsigned char clockMinute = 0;
unsigned char clockHour = 0;
short inTownFrame = 0;
char monumentScripts[16] = {2, 2, 4, 4, 2, 3, 3, 1, 4, 1, 1, 2, 4, 2, 3, 4};
char nextCheatRecord = 0;
short plaqueLines[6] = {0, 36, 196, 210, 230, 244};

short introStep;
unsigned long introStepDue;
short introOpen;
unsigned short logoFailed;
short introSkip;
char logoPath[258];
long townFile;
short townOpen;
Camp *townSlots;
short townsfolkAnchorView;
short highestTownCel;
ShortRect recordHotspots[16];
short recordHotspotNumbers[16];
short recordHotspotCount;
short onRecordHotspot;
short hotspotRecord;
short hotspotScript;
short cheatPlaque;
unsigned long townSoundPause;
unsigned long townSoundEnded;
short townSound;
short townSoundWasGreeting;
short townSoundPlaying;
short draggingInTown;
short townFull;
short walkerViews[16];
short clockWinds;
short clockShown;
unsigned long lastClockRead;
short townFidgetsLeft;
short townPartySize;
unsigned long lastTownFidgetTime;
unsigned long townFidgetInterval;
unsigned long townFidgetersUsed;
short townsfolkGone;
short townspeopleToAdd;
unsigned char windStartMinute;
unsigned char windStartHour;

/* Opens scene 0. */
/* @zoombi32 0x0045c12e */
void openIntro()
{
    introOpen = introSkip = logoFailed = 0;
    sceneDue = introStep = 0;
    introStepDue = 1;
    setGroupLists(townGroups, 1, (short)0xc000);
    introOpen = 1;
}

/* Scene 0's clicks: moves introClickState on from 1 to 2 (always, with
   introSkip); 1 or -1 goes back to the scene sceneToReturnTo picks. */
/* @zoombi32 0x0045c391 */
void introClicked(short which)
{
    if (introSkip)
        introClickState = 2;
    if (which > 0 && introClickState == 1)
        introClickState = 2;
    if (abs(which) == 1) {
        introStepDue = logoFailed = 0;
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
    if (now > lastClockRead + 1800) {
        lastClockRead = now;
        getDateTime(&year, &ignored, &ignored, (char *)&clockHour, (char *)&clockMinute);
        clockMinute = clockMinute / 5;
        clockHour = clockHour % 12;
    }
}

/* Sets whether the views townsfolkViews and the first townPartySize party views run
   their scripts. */
/* @zoombi32 0x0045ccca */
void setTownRunning(short running)
{
    short i;
    View *view;

    for (i = 0; i <= 19; i++) {
        view = findView(townsfolkViews[i]);
        if (view)
            view->body.running = running;
    }
    for (i = 0; i < townPartySize; i++) {
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

/* A view's placed callback: drops its cels whose image is past highestTownCel. */
/* @zoombi32 0x0045daf7 */
void placeTownCels(View *view)
{
    ViewCel *cel;

    for (cel = view->body.cels; cel->image;)
        if (cel->image > highestTownCel)
            removeFirstCel(cel);
        else
            cel++;
}

/* A notify: negates the view's entry in townsfolkViews (the first 19) and counts
   it in townsfolkGone. */
/* @zoombi32 0x0045e29e */
void townsfolkNotify(View *view, short)
{
    short id = view->id;
    short i;

    for (i = 0; i < 19; i++)
        if (id == townsfolkViews[i]) {
            townsfolkViews[i] = -townsfolkViews[i];
            townsfolkGone++;
            break;
        }
}

/* Closes scene 0, leaving its palette and clip for the next scene, and
   reloads the Zoombinis and dialogs. */
/* @zoombi32 0x0045c175 */
void closeIntro()
{
    if (introOpen) {
        introOpen = 0;
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
        rosterReady = 1;
        showCursor();
    }
}

/* Moves every view with flag 2 a screen (320) left or right, wrapping
   round the town's 1920 pixels; a Zoombini's anchor (waypointX) keeps its
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
                snoid->body.waypointX += old - x;
            snoid->pose = -1;
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
        drawImageData((unsigned short *)(townButtonImages->offsets[image] + (char *)townButtonImages), townButtons[which - 1].rect.left,
                      townButtons[which - 1].rect.top, 8);
        if (show)
            showRect(&townButtons[which - 1].rect);
    }
}

/* Closes scene 6. */
/* @zoombi32 0x0045cfae */
void closeTown()
{
    if (townOpen) {
        townOpen = 0;
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
        freeResource(&townButtonResource);
        setFreeAtOnce(saved);
        closeGameFile(&townFile);
        fadeOutViews();
        showBusyCursor();
        townDialogView = -1;
    }
}

/* Finds the hotspot under the cursor (into *where): the first of the
   recordHotspotCount non-empty rectangles recordHotspots holding it sets onRecordHotspot, its
   number (recordHotspotNumbers, from 1) in hotspotRecord and a script by it in hotspotScript. */
/* @zoombi32 0x0045d715 */
void findTownHotspot(Point *where)
{
    short i;

    getCursorPosition(where);
    onRecordHotspot = 0;
    for (i = 0; !onRecordHotspot && i < recordHotspotCount; i++)
        if (!emptyRect(&recordHotspots[i]) && ptInRect(&recordHotspots[i], *where)) {
            i = recordHotspotNumbers[i];
            hotspotScript = monumentScripts[i] + 1003;
            hotspotRecord = i + 1;
            onRecordHotspot = 1;
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

/* Resets scene 6's state; the pace townFidgetInterval by fidgetPaceFlag. */
/* @zoombi32 0x0045c3ec */
void resetTown()
{
    short i;

    cheatPlaque = 0;
    for (i = 0; i < 4; i++)
        townViews[i] = 0;
    townFidgetsLeft = townPartySize = townsfolkGone = 0;
    lastTownFidgetTime = 0;
    townFidgetersUsed = 0;
    if (fidgetPaceFlag)
        townFidgetInterval = 600;
    else
        townFidgetInterval = 120;
    townSoundPlaying = draggingInTown = townFull = 0;
    hotspotRecord = hotspotScript = townSound = townSoundWasGreeting = 0;
    townDialogView = -1;
    clockShown = 0;
    townSoundEnded = 0;
    for (i = 0; i <= 19; i++)
        townsfolkViews[i] = 0;
    lastClockRead = 0;
    readClock();
    clockWinds = 0;
    townspeopleToAdd = 0;
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

/* The clock's view (two cels, its hands): hidden while townDialogView is set
   and unless clockShown and on screen 1 or 2 (townScreen). Shows the time
   (readClock), or with clockWinds winds the hands round that many times
   (faster, from where they are). */
/* @zoombi32 0x0045cd27 */
void drawClock(View *view)
{
    ViewCel *cels = view->body.cels;

    cels[0].image = 0;
    if (townDialogView > 0) {
        view->changed = 0;
        return;
    }
    if (clockShown <= 0)
        return;
    if (townScreen() != 1 && townScreen() != 2)
        return;
    if (clockWinds) {
        if (clockWinds < 0) {
            windStartMinute = clockMinute;
            windStartHour = clockHour;
            clockWinds = abs(clockWinds);
            view->interval = 2;
        } else {
            clockMinute++;
            if (clockMinute > 11) {
                clockMinute = 0;
                clockHour++;
                if (clockHour > 11)
                    clockHour = 0;
            }
            if (windStartMinute == clockMinute && windStartHour == clockHour) {
                clockWinds--;
                if (!clockWinds) {
                    view->interval = 6;
                    lastClockRead = 0;
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
   recordGroups): those set become hotspots (recordHotspots, 56 by 28 about
   recordHotspotPoints's point for the group plus the cel's; 4 also sets clockShown),
   the others are dropped. */
/* @zoombi32 0x0045db25 */
void placeRecordHotspots(View *view)
{
    ShortRect rect;
    ImageBank *volatile bank = groupBanks[view->body.scriptGroup];
    short *cel = (short *)view->body.cels;
    short n;

    recordHotspotCount = clockShown = 0;
    while (*cel)
        if (*cel >= 7 && *cel <= 22) {
            n = *cel - 7;
            if (recordGroups()[n]) {
                recordHotspotNumbers[recordHotspotCount] = n;
                if (n == 4)
                    clockShown = 1;
                bankImage(bank, *cel); /* unused, as in the original */
                cel++;
                rect.left = recordHotspotPoints[n].x + *cel - 28;
                cel++;
                rect.right = rect.left + 56;
                rect.top = recordHotspotPoints[n].y + *cel - 14;
                cel++;
                rect.bottom = rect.top + 28;
                recordHotspots[recordHotspotCount] = rect;
            } else {
                recordHotspots[recordHotspotCount] = noRect;
                recordHotspotNumbers[recordHotspotCount] = -1;
                removeFirstCel((ViewCel *)cel);
            }
            recordHotspotCount++;
        } else {
            cel += 3;
        }
}

/* Scene 0's frame: plays the logo movie (Data\Logo025.MOV) once, then
   clicks; leaves when asked (sceneDue). */
/* @zoombi32 0x0045c212 */
void introFrame()
{
    if (inIntroFrame || !introOpen)
        return;
    inIntroFrame = 1;
    if (movieShowing && !introSkip) {
        if (idleMovie() == 1)
            introStepDue = 1;
    } else if (introSkip || !movieShowing && logoFailed) {
        logoFailed = 0;
        sceneDue = sceneToReturnTo();
    }
    if (sceneDue) {
        pendingScene = sceneDue;
        sceneDue = 0;
        setCurrentMap(0);
        closeIntro();
    } else if (introStepDue) {
        switch (introStep) {
        case 0:
            introStepDue = 0;
            introStep++;
            if (!movieShowing) {
                introClickState = 1;
                logoPath[0] = 0;
                strcpy(logoPath, installDir);
                strcat(logoPath, "Data\\");
                strcat(logoPath, "Logo025.MOV");
                if (playMovie(logoPath)) {
                    introClicked(1);
                } else {
                    logoFailed = 1;
                    hideCursor();
                }
            }
            break;
        case 1:
            introPending = 0;
            introStep++;
            introClicked(-1);
            break;
        }
    }
    inIntroFrame = 0;
}

/* A monument's plaque (a view draw; its interval holds the record shown,
   from 1, or cheatPlaque the one picked): its cels, then five lines about the
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
    if (cheatPlaque) {
        i = cheatPlaque - 1;
        building = monumentBuildings[hotspotRecord - 1];
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

/* While townspeopleToAdd allows, adds a townsperson (a Zoombini view of script
   8000-8043, half from each half) walking at a random place, the height
   by the script, into a free one of townsfolkViews's last three. */
/* @zoombi32 0x0045e06e */
void addTownsperson()
{
    Snoid snoid;
    short y;
    short id;
    short script;
    short i;
    View *view;

    if (!townspeopleToAdd || dialogFlags || townDialogView != -1)
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
        if (!townsfolkViews[16 + i]) {
            id = addView(1, drawCels, runViewScript, script, 6, &snoid, 0, 0);
            view = findView(id);
            if (view) {
                townsfolkViews[16 + i] = id;
                viewSnoid(view)->features[0] = 0;
                view->flags = 0x908002;
                view->body.x = randomBetween(100, 540);
                view->body.y = y;
                *(long *)&view->body.waypointX = *(long *)&view->body.x;
                view->notify = townsfolkNotify;
                view->notifyEnd = 1;
                moveView(id, 0, townsfolkAnchorView);
                i = 3;
                if (townspeopleToAdd > 0)
                    townspeopleToAdd--;
            }
        }
}

/* Shows frame `frame` of the four views townViews at once. */
/* @zoombi32 0x0045da4e */
void setTownFrames(short frame)
{
    short i;
    View *view;
    short value;

    for (i = 0; i < 4; i++) {
        view = findView(townViews[i]);
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
   group has a record) step the plaque shown by cheat (cheatPlaque, 0-16) and
   space reports it; F toggles the townspeople; . records the next group
   passed (nextCheatRecord counting through the groups and levels) now; 0 clears
   the records; 0x125 and 0x127 raise and lower the highest cel shown
   (highestTownCel, 25-81). Returns whether the key was used. */
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
                cheatPlaque++;
            if (key == 'x')
                cheatPlaque--;
            if (townDialogView > 0) {
                view = findView(townDialogView);
                if (view) {
                    view->interval = hotspotRecord;
                    view->changed = 1;
                }
            }
        }
    case ' ':
        if (cheatPlaque > 16)
            cheatPlaque = 16;
        if (cheatPlaque < 0)
            cheatPlaque = 0;
        if (cheatPlaque)
            debugMessage(cheatPlaque, "Cheat Text:", 0, 0, 0);
        else
            debugMessage(-1, "Cheat Text OFF", 0, 0, 0);
        break;
    case 'F':
        if (!townspeopleToAdd)
            townspeopleToAdd = 100;
        else
            townspeopleToAdd = 0;
        break;
    case '.':
        for (used = 0; used < 16; used++)
            if (!recordGroups()[used]) {
                getDateTime(&recordYears()[used], &recordMonths()[used], &recordDays()[used], &hour, &minute);
                recordGroups()[used] = ((nextCheatRecord / 4) & 3) + 1;
                recordLevels()[used] = (nextCheatRecord & 3) + 1;
                used = 16;
                setTownFrames(townScreen());
                nextCheatRecord++;
            }
        used = 1;
        break;
    case 0x125:
    case 0x127:
        switch (key) {
        case 0x125:
            highestTownCel += 5;
            break;
        case 0x127:
            highestTownCel -= 5;
            break;
        }
        if (highestTownCel > 80)
            highestTownCel = 81;
        if (highestTownCel < 24)
            highestTownCel = 25;
        setTownFrames(townScreen());
        used = 1;
        break;
    case '0':
        nextCheatRecord = 0;
        for (used = 0; used < 16; used++)
            recordGroups()[used] = 0;
        setTownFrames(townScreen());
        clockShown = 0;
        used = 1;
        break;
    }
    return used;
}

/* Scene 6's clicks: any click closes an open plaque. 1 leaves; 2 (the
   town) winds the clock when on its view (townsfolkViews[0]), drags a Zoombini
   (who stays where dropped on the ground, y 410-475), opens the plaque
   of the hotspot under the cursor, or at the sides scrolls to the next or
   previous of the six screens. */
/* @zoombi32 0x0045d468 */
void townClicked(short which)
{
    Point where;
    View *view;
    Snoid *snoid;

    if (townDialogView > 0) {
        deleteView(townDialogView);
        townDialogView = -1;
        setTownRunning(1);
        onRecordHotspot = 0;
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
        if (view && view->id == townsfolkViews[0] && !clockWinds)
            clockWinds = -1;
        view = viewAt(where, 2, 1);
        if (view && view->flags == 2) {
            setDragCursor(0);
            draggingInTown = 1;
            dragSnoid(view, where, 0, 0);
            keepDragPose = 0;
            draggingInTown = 0;
            snoid = viewSnoid(view);
            if (snoid->body.y >= 410 && snoid->body.y <= 475)
                *(long *)&snoid->targetX = *(long *)&snoid->body.x;
        } else if (!onRecordHotspot) {
            if (where.y > 30 && where.y < 450 && where.x > 3 && where.x < 637) {
                if (where.x > 560) {
                    queueViewSound(999, 0);
                    townScreen()++;
                    if (townScreen() > 5)
                        townScreen() = 0;
                    setTownFrames(townScreen());
                    scrollTown(1);
                    clockShown = 0;
                } else if (where.x < 80) {
                    queueViewSound(999, 0);
                    townScreen()--;
                    if (townScreen() < 0)
                        townScreen() = 5;
                    setTownFrames(townScreen());
                    scrollTown(0);
                    clockShown = 0;
                }
            }
        } else if (townDialogView == -1) {
            townDialogView = 0;
            setDragCursor(0);
            queueViewSound(999, 0);
            updateViews();
            updateViews();
            townDialogView = addView(0x5000, drawPlaque, runViewCels, hotspotScript, hotspotRecord, 0, 0, 0);
            setTownRunning(0);
            waitForEventFor(0, 2, 0, 1);
        }
        break;
    }
}

/* Opens scene 6, Zoombiniville: adds the travellers to the population
   (townFull once it reaches 625) and the town's slots, sets up the four
   town views (the highest cel shown, highestTownCel, by the population), walkers
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

    townOpen = 0;
    resetTown();
    soundRanges = 0;
    addSoundRange(3000, 3003, 1);
    addSoundRange(20000, 29999, 1);
    addSoundRange(996, 997, 0);
    openGameFile(&townFile, "Town.MHK");
    setCurrentMap(townFile);
    useAltSnoids(0);
    population() += countPresentTravellers();
    if (population() >= 625)
        townFull = 1;
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
    highestTownCel = highest + 24;
    drawBackdrop(1200);
    loadFeatureGroup(1000, 0, 0);
    loadScripts(1000, 8);
    townButtonImages = loadImageBank(1100, &townButtonResource);
    loadDragCursors(2000);
    loadFeatureGroup(4000, 1, 0);
    addScripts(4000, 8, 0);
    loadSnoidScripts(4999, 1, 0);
    addSnoidScripts(5000, 5, 0);
    loadFeatureGroup(6000, 2, 1);
    addScripts(6000, 1, 0);
    loadFeatureGroup(8000, 3, 0);
    addScripts(8000, 44, 1);
    townViews[0] = addView(0x402c000, drawCelsOpaque, runViewCels, 1000, 0, 0, 0, 0);
    townViews[1] = addView(0xc02c000, drawCels, runViewCels, 1002, 0, 0, 0, 0);
    townViews[2] = addView(0xc02c000, drawCels, runViewCels, 1003, 0, 0, 0, 0);
    townViews[3] = addView(0xc02c000, drawCels, runViewCels, 1001, 0, 0, 0, 0);
    for (i = 1; i <= 3; i++) {
        view = findView(townViews[i]);
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
                walkerViews[i] = addView(1, drawCels, runViewScript, walkers[slot], randomBetween(4, 6), &snoid, 0, 0);
                view = findView(walkerViews[i]);
                if (view) {
                    viewSnoid(view)->features[0] = 0;
                    view->flags = 0x808002;
                    *(Point *)&view->body.x = places[slot];
                    *(Point *)&view->body.waypointX = places[slot];
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
    townPartySize = 0;
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
                partyViews[townPartySize] = id;
                townPartySize++;
                view->flags &= ~1;
                view->flags |= 2;
            }
        }
    }
    addView(0x1000, drawTownButtons, updateTownButton, 0, 0, 0, 0, 0);
    setTownFrames(townScreen());
    townsfolkViews[0] = addView(0x8001, drawCels, runViewCels, 6000, 6, &snoid, 0, 0);
    view = findView(townsfolkViews[0]);
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
    townOpen = 1;
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
            townSound = 20086;
            break;
        case 2:
            townSound = 20087;
            break;
        case 3:
            townSound = 20088;
            break;
        default:
            switch (randomBetween(1, 3)) {
            case 1:
                townSound = 20086;
                break;
            case 2:
                townSound = 20087;
                break;
            case 3:
                townSound = 20088;
                break;
            }
            break;
        }
        break;
    case 2:
    case 12:
        switch (randomBetween(1, 2)) {
        case 1:
            townSound = 20087;
            break;
        case 2:
            townSound = 20088;
            break;
        }
        break;
    case 5:
        townSound = 20086;
        break;
    default:
        townSound = townScript();
        townSoundWasGreeting = 1;
        break;
    }
    if (townFull) {
        townSound = 3003;
        townSoundWasGreeting = 1;
    }
    if (townSound) {
        queueViewSound(townSound, 0);
        townSoundPlaying = 1;
    }
    resetViewClock();
    townSoundPause = 0;
    skipJourneyMap = 0;
    if (townFull) {
        townspeopleToAdd = 20;
    } else {
        if (population() > 100)
            townspeopleToAdd++;
        if (population() > 200)
            townspeopleToAdd += 2;
        if (population() > 300)
            townspeopleToAdd += 3;
        if (population() > 400)
            townspeopleToAdd += 4;
        if (population() > 500)
            townspeopleToAdd += 5;
    }
}

/* Scene 6's frame: deletes the townspeople who have walked off, adds
   more, leaves when asked (sceneDue); every 150-300 ticks after a sound
   ends plays the next (the greetings 3000-3002 in turn, or at random one
   of townSounds, not 20093 once over 600 live here); now and then has one
   of the settled Zoombinis on screen do something (townFidgetsLeft times, more
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

    if (inTownFrame || !townOpen)
        return;
    inTownFrame = 1;
    updateViews();
    if (townsfolkGone)
        for (n = 0; n < 19; n++)
            if (townsfolkViews[n] < 0) {
                deleteView(-townsfolkViews[n]);
                townsfolkViews[n] = 0;
                if (townsfolkGone > 0)
                    townsfolkGone--;
            }
    addTownsperson();
    if (sceneDue) {
        pendingScene = sceneDue;
        sceneDue = 0;
        setCurrentMap(0);
        closeTown();
        inTownFrame = 0;
        return;
    }
    if (!isSoundPlaying(townSound, RESOURCE_TYPE(0, 'S', 'N', 'D')) && !dialogFlags) {
        if (townSoundPlaying) {
            townSoundEnded = clockTime();
            townSoundPause = randomBetween(150, 300);
            townSoundPlaying = 0;
            if (townFull)
                townspeopleToAdd = 40;
        }
        if (clockTime() - townSoundEnded > townSoundPause) {
            townSoundEnded = clockTime();
            if (!townSoundWasGreeting || townFull) {
                if (townSound < 20000) {
                    townSound++;
                    if (townSound >= 3003)
                        townSound = 3000;
                } else {
                    townSound = townScript();
                }
                townSoundWasGreeting = 1;
                i = townSound;
            } else {
                townSoundWasGreeting = 0;
                for (n = 1; n;) {
                    n = 0;
                    i = townSounds[allocateSlot(&townSoundsUsed, 5, 0)];
                    if (i == 20093 && population() > 600)
                        n = 1;
                    townSound = i;
                }
            }
            queueViewSound(i, 0);
            townSoundPlaying = 1;
        }
    }
    if (townPartySize && !dialogFlags && townDialogView == -1) {
        if (townFidgetsLeft > 0) {
            if (clockTime() - lastTownFidgetTime > townFidgetInterval) {
                n = 0;
                lastTownFidgetTime = clockTime();
                i = 0;
                do {
                    i++;
                    view = idleSnoidView(partyViews[allocateSlot(&townFidgetersUsed, townPartySize, 0)]);
                    if (view && (view->flags & 2) && view->body.x > 20 && view->body.x < 620) {
                        startSnoidScript(viewSnoid(view), viewSnoid(view)->features[3] + 4999, 0, 0);
                        townFidgetsLeft--;
                        n = 1;
                    }
                } while (!n && i < 16);
            }
        } else if (population() == 625) {
            townFidgetsLeft = 8;
        } else if (population() > 312) {
            townFidgetsLeft = 6;
        } else if (population() > 156) {
            townFidgetsLeft = 4;
        } else if (population() > 156) {
            townFidgetsLeft = 2;
        } else if (population()) {
            townFidgetsLeft = 1;
        }
    }
    if (!dialogFlags && !draggingInTown && townDialogView == -1) {
        findTownHotspot(&where);
        if (!ptInRect(&townButtons[0].rect, where) && where.y > 30 && where.y < 450 && where.x > 3
            && where.x < 637) {
            if (onRecordHotspot)
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
    inTownFrame = 0;
}
