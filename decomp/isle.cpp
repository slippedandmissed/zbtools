/*
 * isle (0x43e620-0x4402c0): Zoombini Isle (scene 3), where Zoombinis are
 * made: 'Picker.MHK'
 */

#include "zoombinis.h"
#include "basecamp.h"
#include "debug.h"
#include "e2memory.h"
#include "features.h"
#include "focus.h"
#include "graphics.h"
#include "isle.h"
#include "mainloop.h"
#include "platform.h"
#include "snoids.h"
#include "sound.h"
#include "town.h"
#include "view.h"

SceneButton isleSceneButtons[8] = {
    {{159, 436, 198, 478}}, {{205, 304, 260, 342}}, {{205, 347, 273, 420}}, {{201, 419, 287, 437}},
    {{205, 440, 260, 478}}, {{600, 403, 639, 440}}, {{600, 441, 639, 478}}, {{0, 0, 640, 480}},
};
Point isleQueuePlaces[16] = {
    {542, 446}, {505, 447}, {466, 451}, {425, 448}, {380, 450}, {342, 451}, {522, 402}, {488, 408},
    {444, 416}, {403, 413}, {364, 413}, {498, 360}, {463, 367}, {426, 370}, {389, 373}, {352, 374},
};
Point isleEntry = {172, 226};
Point isleExit = {0};
short isleBusy = 0;
ShortRect isleButtonsRect = {0};
ShortRect featureButtonsRect = {0};
char isleImageHotX[22] = {
    0, 22, 25, 28, 19, 10, 29, 15, 7, 17, 24, 24, 7, 7, 7, 7, 7, 23, 24, 13, 15, 23,
};
char isleImageHotY[22] = {
    0, 23, 30, 29, 27, 31, 30, 11, 11, 11, 9, 6, -2, -2, -2, -2, -2, -22, -22, -20, -21, -23,
};
short isleEnoughDrawn = 0;
short isleMakeAllowedDrawn = 0;

char madeName[11];
long isleFile;
long featureButtonResource;
long isleImagesResource;
long isleButtonResource;
ImageBank *featureButtonImages;
ImageBank *isleImages;
ImageBank *isleButtonImages;
short isleOpen;
short isleCheatButtonLit;
short enoughToLeaveChosen;
short zoombiniMakeAllowed;
short isleButton22Due;
short enoughToLeave;
short settingView1;
short settingView2;
short view4100;
short isleRemark;
short isleSendingOff;

/* Resets the Zoombini being made (snoidBeingMade): no features, a new name. */
/* @zoombi32 0x0043e620 */
void resetZoombiniMade()
{
    short i;

    placeSnapRadius = 60;
    isleCheatButtonLit = zoombiniMakeAllowed = isleButton22Due = 0;
    sceneDue = 0;
    settingView1 = settingView2 = isleRemark = 0;
    for (i = 0; i < 4; i++)
        snoidBeingMade.features[i] = 0;
    snoidBeingMade.name[0] = 0;
    makeName(snoidBeingMade.name, 10);
    for (i = 0; i < 16; i++)
        sortedIds[i] = 0;
    isleSendingOff = 0;
    snoidsArrived = snoidsOnTheirWay = 0;
}

/*
 * Opens Zoombini Isle: loads Picker.MHK's backdrop, images, scripts and
 * sounds, adds the scene's views and the queue's places, brings back the
 * party waiting here, places each spot by the queue (the nearest one no
 * earlier place has taken), and sets up the Zoombini being made. Offers to
 * load a saved game first if asked (introClickState). Then a hint, unless coming
 * from the camp, where a remark (20043/20044) may say how many Zoombinis
 * are left to make.
 */
/* @zoombi32 0x0043e6b6 */
void openIsle()
{
    Point entry = isleEntry;
    Point exit;
    short i;
    short skip;
    short spot;
    short j;

    isleOpen = 0;
    resetZoombiniMade();
    addSoundRange(20000, 29999, 1);
    addSoundRange(1000, 1007, 0);
    setSnoidMode(0);
    enoughToLeave = 16;
    zoombiniMakeAllowed = zoombiniMadeAllowed();
    setPenWidth(1);
    openGameFile(&isleFile, "Picker.MHK");
    setCurrentMap(isleFile);
    loadPaths(1000);
    drawBackdrop(4000);
    featureButtonImages = loadImageBank(4400, &featureButtonResource);
    isleButtonImages = loadImageBank(4200, &isleButtonResource);
    isleImages = loadImageBank(4300, &isleImagesResource);
    loadFeatureGroup(4100, 0, 0);
    loadScripts(4100, 11);
    copyPaletteRange(10, 236);
    loadSoundByKey(1000, RESOURCE_TYPE(0, 'S', 'N', 'D'));
    loadSoundByKey(1004, RESOURCE_TYPE(0, 'S', 'N', 'D'));
    loadSoundByKey(1005, RESOURCE_TYPE(0, 'S', 'N', 'D'));
    loadSoundByKey(1006, RESOURCE_TYPE(0, 'S', 'N', 'D'));
    for (i = 4101; i <= 4103; i++)
        addView(0x8000, drawCels, runViewScript, i, i - 4091, 0, 0, 0);
    if (!*(short *)(gameState + 0x20))
        addIsleSettingViews();
    view4100 = addView(0x64000000, drawCels, runViewScript, 4100, 0, 0, 0, 0);
    addView(0x108a000, drawCels, runViewScript, 4110, 6, &entry, 0, 0);
    for (i = 4106; i <= 4109; i++)
        addView(0, drawCels, runViewScript, i, 0, 0, 0, 0);
    addView(0x4001000, drawIsleButtonsView, updateIsleButtons, 0, 0, 0, 0, 0);
    setViewPlaces(16, isleQueuePlaces, 1);
    *party() = *waitingParties();
    waitingParties()->count = 0;
    makePartySnoids(0);
    exit = isleExit;
    spotTaken(&exit, 0, 500);
    for (i = 0; i < 16; i++) {
        skip = 0;
        spot = spotNear(&isleQueuePlaces[i], 500, skip);
        for (j = 0; spot && j < i; j++)
            if (spot == sortedIds[j]) {
                skip++;
                spot = spotNear(&isleQueuePlaces[i], 500, skip);
                j = 0;
            }
        sortedIds[i] = spot;
    }
    chooseSnoids(1, 0);
    updateViews();
    if (*(short *)(gameState + 0x20))
        *(short *)(gameState + 0x26) = 1;
    showIsleSetting(1);
    checkEnoughChosen();
    setGroupLists(isleGroups, 2, (short)0xc000);
    highlightItemAt(1, 1);
    visitAllItems();
    snoidBeingMade.body.x = isleSceneButtons[2].rect.left + 39;
    snoidBeingMade.body.y = isleSceneButtons[2].rect.top + 31;
    snoidBeingMade.chosen = 1;
    drawIsleButtons(0, 0, 0);
    drawFeatureButtons(0, 0, 0);
    if (introClickState) {
        if (savedGames)
            askLoadGame();
        updateViews();
    }
    showRect(&gameRect);
    fadeInViews();
    isleOpen = 1;
    queueViewSound(30001, 0);
    if (journeyFrom != 1) {
        campHint((short *)(gameState + 0x28));
        if (*(short *)(gameState + 0x48) < 625 && introClickState == 1)
            isleRemark = 20042;
    } else {
        short made = countSnoidViews();
        short left = 625 - (*(short *)(gameState + 0x4a) + *(short *)(gameState + 0x4c) + *(short *)(gameState + 0x4e))
            - made;

        if (left > 0 && made < 625) {
            switch (randomBetween(1, 20)) {
            case 1:
                isleRemark = 20043;
                break;
            case 10:
                isleRemark = 20044;
                break;
            }
        }
    }
    if (isleRemark)
        queueViewSound(isleRemark, 1);
    introClickState = 0;
    skipJourneyMap = 0;
}

/* Closes the scene, leaving the party waiting (or, when it's leaving,
   taking it on). */
/* @zoombi32 0x0043eb13 */
void closeIsle()
{
    if (isleOpen) {
        isleOpen = 0;
        short saved = setFreeAtOnce(1);

        clearViews();
        if (!viewsLocked) {
            if (leavingGame || pendingScene == 1) {
                party()->unknown2 = 0;
                party()->unknown4 = 0;
                *waitingParties() = *party();
                party()->count = 0;
            } else {
                waitingParties()->count = 0;
            }
        }
        unloadSounds();
        freeResource(&isleImagesResource);
        freeResource(&featureButtonResource);
        freeResource(&isleButtonResource);
        setFreeAtOnce(saved);
        closeGameFile(&isleFile);
        fadeOutViews();
        showBusyCursor();
    }
}

/* Scene 3's frame: leaving once asked to (sceneDue) and sound 996 is
   done; isleSendingOff is cleared while snoidsOnTheirWay isn't set. */
/* @zoombi32 0x0043ebf4 */
void isleFrame()
{
    if (!isleBusy && isleOpen) {
        isleBusy = 1;
        updateViews();
        if (sceneDue) {
            if (isSoundPlaying(996, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
                isleBusy = 0;
                return;
            }
            if (viewsLocked || !snoidsOnTheirWay || snoidsArrived >= 1)
                leaveIsleIfAsked();
        }
        if (!snoidsOnTheirWay && isleSendingOff)
            isleSendingOff = 0;
        isleBusy = 0;
    }
}

/* Scene 3's keys: 23 brings back the two views settingView1 and settingView2
   (adding them first if the game state's +0x20 is set). Returns whether it
   handled the key. */
/* @zoombi32 0x0043ec8c */
short isleKey(unsigned short key)
{
    short handled = 0;

    switch (key) {
    case 23:
        if (*(short *)(gameState + 0x20))
            addIsleSettingViews();
        showIsleSetting(0);
        handled = 1;
        break;
    }
    return handled;
}

/* A feature button clicked (1-20, in four groups of five): picks that feature
   for the Zoombini being made (sound 1000), or drops it if already picked
   (1004), or refuses (1008, never: isleAllowsFeature allows every feature). Stops sound
   isleRemark first, and redraws the panel's third button if the Zoombini's
   completeness changes. */
/* @zoombi32 0x0043ecbb */
void featureButtonClicked(short button)
{
    ShortRect rect;
    short group;
    short chosen;

    if (leaveIsleIfAsked())
        return;
    if (isleRemark && isSoundPlaying(isleRemark, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
        stopSounds(isleRemark, RESOURCE_TYPE(0, 'S', 'N', 'D'));
        isleRemark = 0;
    }
    rect = isleButtons[button].rect;
    group = 0;
    while (button >= 6) {
        button -= 5;
        group++;
    }
    chosen = snoidBeingMade.features[group];
    if (chosen && button == chosen) {
        queueViewSound(1004, 0);
        chosen += group * 5;
        drawFeatureButtons(chosen, 0, &rect);
        showRect(&rect);
        snoidBeingMade.features[group] = 0;
    } else if (isleAllowsFeature(group, button - 1)) {
        if (chosen) {
            chosen += group * 5;
            drawFeatureButtons(chosen, 0, &rect);
            showRect(&rect);
        }
        queueViewSound(1000, 0);
        drawFeatureButtons(group * 5 + button, 1, &rect);
        showRect(&rect);
        snoidBeingMade.features[group] = button;
    } else {
        queueViewSound(1008, 0);
    }
    short allowed = zoombiniMadeAllowed();

    if (allowed != zoombiniMakeAllowed) {
        zoombiniMakeAllowed = allowed;
        drawIsleButtons(3, 1, 1);
    }
    isleButton22Due = 1;
}

/* Leaves the scene if asked to (sceneDue names where to): returns whether
   it did. */
/* @zoombi32 0x0043ee2a */
short leaveIsleIfAsked()
{
    if (sceneDue) {
        pendingScene = sceneDue;
        sceneDue = 0;
        setCurrentMap(0);
        closeIsle();
        return 1;
    }
    return 0;
}

/*
 * A panel button clicked (1-7): 1 makes the Zoombini chosen (sound 1005)
 * and sends it to the queue's first free place, while fewer than 625 have
 * been made (with Ctrl, debugging, and no Zoombinis about, first makes it
 * 624); 2 has it say something; 3 renames it (1000); 4 picks another at
 * random (1006; with Ctrl, fills the queue with random ones); 5 goes to
 * the map (999); 6 with the party complete sends up to one of places 11,
 * 12, 6 and 7 off (996) and asks to leave for scene 7, else may remark
 * (20043 or 20044) on Zoombinis left to make; 7 takes a Zoombini from the
 * queue back to be remade (1007). Stops sound isleRemark first.
 */
/* @zoombi32 0x0043ee5d */
void isleButtonClicked(short button)
{
    Point cursor;
    Point where;
    short slot;
    ShortRect rect;
    short id;
    unsigned long when;
    Snoid *snoid;
    View *view;
    View *other;

    if (leaveIsleIfAsked())
        return;
    if (isleRemark && isSoundPlaying(isleRemark, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
        stopSounds(isleRemark, RESOURCE_TYPE(0, 'S', 'N', 'D'));
        isleRemark = 0;
    }
    rect = isleSceneButtons[button].rect;
    getCursorPosition(&cursor);
    switch (button) {
    case 1:
        if (!enoughToLeaveChosen && zoombiniMakeAllowed && *(short *)(gameState + 0x48) < 625) {
            if (debugMessagesOn && addModifierKeys(0) == 0x800 && !countSnoidViews()) {
                *(short *)(gameState + 0x48) = 624;
                *(short *)(gameState + 0x4e) = 624;
            }
            rosterChanged = 1;
            isleSendingOff = 1;
            snoidsOnTheirWay++;
            queueViewSound(1005, 0);
            drawIsleButtons(button, 1, 1);
            waitForEventFor(0, 2, 0, 1);
            countZoombiniMade(1);
            isleQueue(&where, &slot);
            id = placeSnoid(&snoidBeingMade, 0, 148, 215, where.x, where.y);
            sortedIds[slot] = id;
            (*(short *)(gameState + 0x48))++;
            drawIsleButtons(button, 0, 1);
            sortViews();
            checkEnoughChosen();
            makeName(madeName, 10);
            drawIsleButtons(3, 1, 1);
        }
        zoombiniMakeAllowed = zoombiniMadeAllowed();
        break;
    case 2:
        queueViewSound(snoidSound(&snoidBeingMade, randomBetween(0, 12)), 0);
        break;
    case 3:
        if (zoombiniMakeAllowed) {
            queueViewSound(1000, 0);
            makeName(madeName, 10);
            drawIsleButtons(button, 1, 1);
        }
        break;
    case 4:
        if (*(short *)(gameState + 0x48) < 625) {
            queueViewSound(1006, 0);
            drawIsleButtons(button, 1, 1);
            if (!enoughToLeaveChosen && addModifierKeys(0) == 0x800) {
                pickZoombiniMade(1);
                drawFeatureButtons(0, 0, &rect);
                showRect(&rect);
                isleQueue(&where, &slot);
                when = clockTime();
                while (!enoughToLeaveChosen) {
                    rosterChanged = 1;
                    countZoombiniMade(1);
                    id = placeSnoid(&snoidBeingMade, when, 148, 215, where.x, where.y);
                    isleSendingOff = 1;
                    snoidsOnTheirWay++;
                    sortedIds[slot] = id;
                    (*(short *)(gameState + 0x48))++;
                    if (!*(short *)(gameState + 0x20))
                        when += randomBetween(60, 120);
                    else
                        when += randomBetween(120, 180);
                    checkEnoughChosen();
                    pickZoombiniMade(1);
                    isleQueue(&where, &slot);
                }
                drawIsleButtons(3, 1, 1);
                drawFeatureButtons(0, 0, &rect);
                showRect(&rect);
                drawIsleButtons(button, 0, 1);
            } else {
                waitForEventFor(0, 2, 0, 1);
                drawIsleButtons(button, 0, 1);
                pickZoombiniMade(zoombiniMakeAllowed);
                drawFeatureButtons(0, 0, &rect);
                showRect(&rect);
                drawIsleButtons(3, 1, 1);
            }
            zoombiniMakeAllowed = zoombiniMadeAllowed();
        } else {
            queueViewSound(1008, 0);
        }
        break;
    case 5:
        queueViewSound(999, 0);
        drawIsleButtons(button, 1, 1);
        waitForEventFor(0, 2, 0, 1);
        drawIsleButtons(button, 0, 1);
        pendingScene = 1;
        closeIsle();
        break;
    case 6:
        if (enoughToLeaveChosen) {
            short order[4] = {11, 12, 6, 7};

            queueViewSound(996, 0);
            drawIsleButtons(button, 1, 1);
            waitForEventFor(0, 2, 0, 1);
            drawIsleButtons(button, 0, 1);
            snoidsArrived = snoidsOnTheirWay = 0;
            where.x = where.y = 0;
            for (slot = 0; snoidsOnTheirWay < 1 && slot < 4; slot++) {
                id = sortedIds[order[slot]];
                if (id) {
                    snoid = findSnoid(id, 1);
                    if (snoid && !snoid->action) {
                        view = findView(id);
                        view->nextUpdate = clockTime() + snoidsOnTheirWay * 60;
                        snoid->pathIndex = -1;
                        snoid->targetX = 544;
                        snoid->targetY = 264;
                        setSnoidAction(snoid, 7, 0);
                        snoidsOnTheirWay++;
                    }
                }
            }
            sceneDue = 7;
        } else {
            short made = countSnoidViews();
            short left = 625 - (*(short *)(gameState + 0x4a) + *(short *)(gameState + 0x4c) + *(short *)(gameState + 0x4e))
                - made;

            if (left > 0 && made < 625) {
                switch (randomBetween(1, 2)) {
                case 1:
                    isleRemark = 20043;
                    break;
                case 2:
                    isleRemark = 20044;
                    break;
                }
                if (isleRemark)
                    queueViewSound(isleRemark, 1);
            }
        }
        break;
    case 7:
        if (!isleSendingOff) {
            short grab;

            view = viewAt(cursor, 1, 1);
            grab = 0;
            if (view) {
                grab = ((Snoid *)&view->body)->action;
                if (!grab || grab == 6 || grab == 4)
                    grab = 1;
                else
                    grab = 0;
            }
            if (grab) {
                other = findView(view4100);
                if (other)
                    other->flags |= 0x8000;
                dragSnoid(view, cursor, 0, 0);
                rect = view->body.bounds;
                if (heldPlaceNumber()) {
                    if ((view = removeView(view->id, 0)) != 0) {
                        for (slot = 0; slot < 16; slot++)
                            if (sortedIds[slot] == view->id) {
                                sortedIds[slot] = 0;
                                slot = 16;
                            }
                        rosterChanged = 1;
                        claimPlacedView(1, 0);
                        queueViewSound(1007, 0);
                        if (*(short *)(gameState + 0x48) > 0)
                            (*(short *)(gameState + 0x48))--;
                        for (slot = 0; slot < 4; slot++) {
                            snoidBeingMade.features[slot] = ((Snoid *)&view->body)->features[slot];
                            ((Snoid *)&view->body)->features[slot] = 0;
                        }
                        for (slot = 0; slot < 10; slot++)
                            madeName[slot] = ((Snoid *)&view->body)->name[slot];
                        countZoombiniMade(0);
                        drawFeatureButtons(0, 0, &rect);
                        showRect(&rect);
                        drawIsleButtons(3, 1, 1);
                        checkEnoughChosen();
                        zoombiniMakeAllowed = zoombiniMadeAllowed();
                        isleButton22Due = 1;
                        isleQueue(0, 0);
                        if (*(short *)(gameState + 0x48) == 624)
                            drawIsleButtons(button, 0, 1);
                    }
                }
                if (other)
                    other->flags &= ~0x8000;
            }
        }
        break;
    }
}

/* Draws the panel's buttons (1-7, from the bank isleButtonImages; some lit, some
   greyed by the scene's state), or just button `which`, lit or not; the
   second shows the Zoombini being made, the third its name (madeName, if
   zoombiniMakeAllowed). Shows the area drawn if `show`. */
/* @zoombi32 0x0043f5ea */
void drawIsleButtons(short which, short lit, short show)
{
    short y;
    short count;
    ShortRect rect;
    ShortRect bounds = isleButtonsRect;
    Color saved;
    short i;
    short x;
    short image;

    if (!which) {
        i = 0;
        count = 7;
        bounds = isleSceneButtons[1].rect;
        unionRect(&bounds, &isleSceneButtons[7].rect);
    } else {
        i = which - 1;
        count = i + 1;
        bounds = isleSceneButtons[which].rect;
    }
    for (; i < count; i++) {
        x = isleSceneButtons[i + 1].rect.left;
        y = isleSceneButtons[i + 1].rect.top;
        image = 0;
        switch (i) {
        case 0:
            image = 2;
            if (*(short *)(gameState + 0x48) >= 625 || enoughToLeaveChosen || !zoombiniMakeAllowed) {
                lit = 0;
                image = 1;
            }
            break;
        case 1:
            drawZoombiniParts(&snoidBeingMade);
            break;
        case 2:
            if (dialogFlags & 0x10)
                return;
            drawImageData((unsigned short *)(isleButtonImages->offsets[13] + (char *)isleButtonImages), x, y, 8);
            rect = isleSceneButtons[i + 1].rect;
            saved = setForeColor(Color(45));
            if (zoombiniMakeAllowed) {
                rect.top++;
                rect.left += 4;
                drawText(rect, 0x22, madeName, 0xffff);
                rect.top--;
                rect.left -= 4;
            }
            setForeColor(saved);
            break;
        case 3:
            image = 4;
            if (!enoughToLeaveChosen && isleCheatButtonLit)
                image = 6;
            break;
        case 4:
            image = 11;
            break;
        case 5:
            image = 9;
            if (!enoughToLeaveChosen) {
                lit = 0;
                image = 8;
            }
            break;
        }
        if (image) {
            if (lit)
                image++;
            drawImageData((unsigned short *)(isleButtonImages->offsets[image] + (char *)isleButtonImages), x, y, 8);
        }
    }
    if (show)
        showRect(&bounds);
}

/* Draws the feature buttons (1-20, from the bank featureButtonImages; each lit if it's
   the feature chosen in its group of five for the Zoombini being made), or
   just button `which`, lit or not; stores the area drawn in *bounds. */
/* @zoombi32 0x0043f856 */
void drawFeatureButtons(short which, short lit, ShortRect *bounds)
{
    ShortRect rect = featureButtonsRect;
    short i;
    short image;

    if (!which) {
        unionRect(&rect, &isleButtons[1].rect);
        unionRect(&rect, &isleButtons[20].rect);
        for (i = 0; i < 20; i++) {
            image = i + i + 1;
            if (i % 5 + 1 == snoidBeingMade.features[i / 5])
                image++;
            drawImageData((unsigned short *)(featureButtonImages->offsets[image] + (char *)featureButtonImages), isleButtons[i + 1].rect.left,
                          isleButtons[i + 1].rect.top, 8);
        }
    } else {
        rect = isleButtons[which].rect;
        i = which - 1;
        image = i + i + 1;
        if (lit)
            image++;
        drawImageData((unsigned short *)(featureButtonImages->offsets[image] + (char *)featureButtonImages), isleButtons[i + 1].rect.left,
                      isleButtons[i + 1].rect.top, 8);
    }
    if (bounds)
        *bounds = rect;
}

/* Draws image `which` of the bank isleImages at (x, y) by its hot spot
   (isleImageHotX, isleImageHotY). */
/* @zoombi32 0x0043f985 */
void drawIsleImage(short which, short x, short y)
{
    if (which)
        drawImageData((unsigned short *)(isleImages->offsets[which] + (char *)isleImages), x - isleImageHotX[which],
                      y - isleImageHotY[which], 8);
}

/* Draws a Zoombini from its parts in the bank isleImages where it stands:
   feet, body, eyes, nose, then hair. */
/* @zoombi32 0x0043f9d3 */
void drawZoombiniParts(Snoid *snoid)
{
    short x = snoid->body.x;
    short y = snoid->body.y;

    if (snoid->features[3])
        drawIsleImage(snoid->features[3] + 16, x, y);
    drawIsleImage(1, x, y);
    if (snoid->features[1])
        drawIsleImage(snoid->features[1] + 6, x, y);
    if (snoid->features[2])
        drawIsleImage(snoid->features[2] + 11, x, y);
    if (snoid->features[0])
        drawIsleImage(snoid->features[0] + 1, x, y);
}

/* Whether the Zoombini being made is complete (its features valid, any
   out of range cleared) and fewer than two of its kind exist yet. */
/* @zoombi32 0x0043fa67 */
short zoombiniMadeAllowed()
{
    short i;
    short hair;
    short eyes;
    short nose;
    short feet;

    for (i = 0; i < 4; i++) {
        if (snoidBeingMade.features[i] < 0 || snoidBeingMade.features[i] > 5)
            snoidBeingMade.features[i] = 0;
        if (!snoidBeingMade.features[i])
            return 0;
    }
    hair = snoidBeingMade.features[0] - 1;
    eyes = snoidBeingMade.features[1] - 1;
    nose = snoidBeingMade.features[2] - 1;
    feet = snoidBeingMade.features[3] - 1;
    if (zoombiniCounts()[hair][eyes][nose][feet] >= 2)
        return 0;
    return 1;
}

/*
 * Picks features for the Zoombini being made: random ones (all of them if
 * `rename`, else those not chosen) until it's a kind with fewer than two,
 * and after 64 tries the last such kind in order (the search never stops
 * early: its flag is never set). Names it if `rename` or the first pick
 * failed.
 */
/* @zoombi32 0x0043fb0f */
void pickZoombiniMade(short rename)
{
    short tries = 0;
    short found;
    short hair;
    short eyes;
    short nose;
    short feet;
    short i;

    zoombiniMakeAllowed = 0;
    while (!zoombiniMakeAllowed && tries < 64) {
        tries++;
        if (tries >= 64) {
            found = 0;
            rename = 1;
            for (hair = 0; !found && hair < 5; hair++)
                for (eyes = 0; !found && eyes < 5; eyes++)
                    for (nose = 0; !found && nose < 5; nose++)
                        for (feet = 0; !found && feet < 5; feet++)
                            if (zoombiniCounts()[hair][eyes][nose][feet] < 2) {
                                snoidBeingMade.features[0] = hair + 1;
                                snoidBeingMade.features[1] = eyes + 1;
                                snoidBeingMade.features[2] = nose + 1;
                                snoidBeingMade.features[3] = feet + 1;
                            }
        } else {
            for (i = 0; i < 4; i++)
                if (!snoidBeingMade.features[i] || rename)
                    snoidBeingMade.features[i] = randomBetween(1, 5);
        }
        zoombiniMakeAllowed = zoombiniMadeAllowed();
        if (!zoombiniMakeAllowed)
            rename = 1;
    }
    if (rename)
        makeName(madeName, 10);
    isleButton22Due = 1;
}

/* The view drawing the isle's buttons and the feature buttons. */
/* @zoombi32 0x0043fc7d */
void drawIsleButtonsView(View *)
{
    drawIsleButtons(0, 0, 0);
    drawFeatureButtons(0, 0, 0);
}

/* The scene's update: adds to the region to redraw the buttons whose state
   changed (24 with the cheat modifier, 26 and 21 with enoughToLeaveChosen, 21 with
   zoombiniMakeAllowed, 22 when isleButton22Due asks). */
/* @zoombi32 0x0043fc9a */
void updateIsleButtons(View *, short region)
{
    if (!dialogFlags && *(short *)(gameState + 0x48) < 625 && addModifierKeys(0) == 0x800) {
        if (!isleCheatButtonLit) {
            isleCheatButtonLit = 1;
            unionRgnRect(region, &isleButtons[24].rect);
        }
    } else if (isleCheatButtonLit) {
        isleCheatButtonLit = 0;
        unionRgnRect(region, &isleButtons[24].rect);
    }
    if (enoughToLeaveChosen) {
        if (!isleEnoughDrawn) {
            isleEnoughDrawn = 1;
            unionRgnRect(region, &isleButtons[26].rect);
            unionRgnRect(region, &isleButtons[21].rect);
        }
    } else if (isleEnoughDrawn) {
        isleEnoughDrawn = 0;
        unionRgnRect(region, &isleButtons[26].rect);
        unionRgnRect(region, &isleButtons[21].rect);
    }
    if (!zoombiniMakeAllowed) {
        if (isleMakeAllowedDrawn) {
            isleMakeAllowedDrawn = 0;
            unionRgnRect(region, &isleButtons[21].rect);
        }
    } else if (!isleMakeAllowedDrawn) {
        isleMakeAllowedDrawn = 1;
        unionRgnRect(region, &isleButtons[21].rect);
    }
    if (isleButton22Due) {
        isleButton22Due = 0;
        unionRgnRect(region, &isleButtons[22].rect);
    }
}

/* Counts the Zoombini being made (snoidBeingMade) in or out of the numbers of
   each kind (at most two), noting in zoombiniMakeAllowed whether it's complete (and,
   when adding, not one too many). */
/* @zoombi32 0x0043fdcc */
void countZoombiniMade(short add)
{
    Snoid *made = &snoidBeingMade;
    short i;
    short hair;
    short eyes;
    short nose;
    short feet;

    if (add) {
        zoombiniMakeAllowed = zoombiniMadeAllowed();
    } else {
        zoombiniMakeAllowed = 1;
        for (i = 0; i < 4; i++)
            if (!made->features[i])
                zoombiniMakeAllowed = 0;
    }
    if (zoombiniMakeAllowed) {
        hair = made->features[0] - 1;
        eyes = made->features[1] - 1;
        nose = made->features[2] - 1;
        feet = made->features[3] - 1;
        if (add) {
            if (zoombiniCounts()[hair][eyes][nose][feet] < 2)
                zoombiniCounts()[hair][eyes][nose][feet]++;
        } else if (zoombiniCounts()[hair][eyes][nose][feet] > 0) {
            zoombiniCounts()[hair][eyes][nose][feet]--;
        }
    }
}

/* Shows the two views settingView1 and settingView2 by the setting at game state
   +0x26 (0-3), stepping it on unless `keep`. */
/* @zoombi32 0x0043ff1d */
void showIsleSetting(short keep)
{
    View *first;
    View *second;
    short a;
    short b;

    first = findView(settingView1);
    second = findView(settingView2);
    if (first && second) {
        if (!keep)
            (*(short *)(gameState + 0x26))++;
        if (*(short *)(gameState + 0x26) > 3)
            *(short *)(gameState + 0x26) = 0;
        switch (*(short *)(gameState + 0x26)) {
        case 0:
            a = 1;
            b = 1;
            break;
        case 1:
            a = 0;
            b = 0;
            break;
        case 2:
            a = 1;
            b = 0;
            break;
        case 3:
            a = 0;
            b = 1;
            break;
        }
        first->body.running = a;
        first->body.group = 0;
        second->body.running = b;
        second->body.group = 0;
    }
}

/*
 * The queue of places (sortedIds, at isleQueuePlaces): with `where`, gives the
 * first free place and its number; otherwise moves the Zoombinis up into
 * each free place from up to five places behind (by where the place is
 * in its row of five), one at a time.
 */
/* @zoombi32 0x0043ffd5 */
void isleQueue(Point *where, short *slot)
{
    short d1;
    short d2;
    short d3;
    short d4;
    short waiting;
    short i;
    short step;
    Snoid *snoid;

    if (where) {
        for (i = 0; i < 16; i++)
            if (!sortedIds[i]) {
                *where = isleQueuePlaces[i];
                *slot = i;
                return;
            }
        return;
    }
    noPaths = 1;
    for (i = 0; i < 15; i++)
        if (!sortedIds[i]) {
            step = d1 = d2 = d3 = d4 = 0;
            switch (i) {
            case 0:
                step = 1;
                d1 = 2;
                d2 = 3;
                d3 = 4;
                d4 = 5;
                break;
            case 1:
            case 6:
            case 11:
                step = 1;
                d1 = 2;
                d2 = 3;
                d3 = 4;
                break;
            case 2:
            case 7:
            case 12:
                step = 1;
                d1 = 2;
                d2 = 3;
                break;
            case 3:
            case 8:
            case 13:
                step = 1;
                d1 = 2;
                break;
            case 4:
            case 9:
            case 14:
                step = 1;
                break;
            }
            waiting = 1;
            while (step && waiting) {
                if (sortedIds[i + step]) {
                    snoid = findSnoid(sortedIds[i + step], 1);
                    if (snoid) {
                        snoid->pathIndex = -1;
                        if (i + step < 17) {
                            *(Point *)&snoid->targetX = isleQueuePlaces[i];
                            setSnoidAction(snoid, 7, 0);
                        } else {
                            snoid->targetX = 326;
                            snoid->targetY = 390;
                            setSnoidAction(snoid, 7, 0);
                            updateSnoidView(snoidView(snoid), removedRgn);
                            *(Point *)&snoid->targetX = isleQueuePlaces[i];
                        }
                        sortedIds[i] = sortedIds[i + step];
                        sortedIds[i + step] = 0;
                        waiting = 0;
                    }
                }
                step = d1;
                d1 = d2;
                d2 = d3;
                d3 = d4;
                d4 = 0;
            }
        }
    updateViews();
    noPaths = 0;
}

/* Adds the views settingView1 (script 4104) and settingView2 (4105) if they
   aren't there. */
/* @zoombi32 0x00440218 */
void addIsleSettingViews()
{
    if (!settingView1)
        settingView1 = addView(0x800c000, drawCels, runViewScript, 4104, 7, 0, 0, 0);
    if (!settingView2)
        settingView2 = addView(0x8008000, drawCels, runViewScript, 4105, 9, 0, 0, 0);
}

/* @zoombi32 0x0044027b */
short isleAllowsFeature(short, short)
{
    return 1;
}

/* Sets enoughToLeaveChosen if there are Zoombinis chosen and either at least
   enoughToLeave of them or 625 counted in the game (the whole population). */
/* @zoombi32 0x00440286 */
void checkEnoughChosen()
{
    enoughToLeaveChosen = 0;
    short count = countChosenSnoids();

    if (count)
        enoughToLeaveChosen = count >= enoughToLeave || *(short *)(gameState + 0x48) >= 625;
}
