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
#include "module_4623b8.h"
#include "platform.h"
#include "snoids.h"
#include "sound.h"
#include "town.h"
#include "view.h"

/* Resets the Zoombini being made (g_4b1484): no features, a new name. */
/* @zoombi32 0x0043e620 */
void resetZoombiniMade()
{
    short i;

    g_4b755e = 60;
    g_4b15a6 = g_4b15aa = g_4b15ac = 0;
    g_4b0d52 = 0;
    g_4b15b0 = g_4b15b2 = g_4b15b6 = 0;
    for (i = 0; i < 4; i++)
        g_4b1484.features[i] = 0;
    g_4b1484.name[0] = 0;
    makeName(g_4b1484.name, 10);
    for (i = 0; i < 16; i++)
        sortedIds[i] = 0;
    g_4b15b8 = 0;
    g_4b755c = g_4b755a = 0;
}

/*
 * Opens Zoombini Isle: loads Picker.MHK's backdrop, images, scripts and
 * sounds, adds the scene's views and the queue's places, brings back the
 * party waiting here, places each spot by the queue (the nearest one no
 * earlier place has taken), and sets up the Zoombini being made. Offers to
 * load a saved game first if asked (g_4a7410). Then a hint, unless coming
 * from the camp, where a remark (20043/20044) may say how many Zoombinis
 * are left to make.
 */
/* @zoombi32 0x0043e6b6 */
void openIsle()
{
    Point entry = g_4a3364;
    Point exit;
    short i;
    short skip;
    short spot;
    short j;

    g_4b15a4 = 0;
    resetZoombiniMade();
    addSoundRange(20000, 29999, 1);
    addSoundRange(1000, 1007, 0);
    fn_45aaff(0);
    g_4b15ae = 16;
    g_4b15aa = zoombiniMadeAllowed();
    setPenWidth(1);
    openGameFile(&g_4b1588, "Picker.MHK");
    setCurrentMap(g_4b1588);
    loadPaths(1000);
    drawBackdrop(4000);
    g_4b1598 = loadImageBank(4400, &g_4b158c);
    g_4b15a0 = loadImageBank(4200, &g_4b1594);
    g_4b159c = loadImageBank(4300, &g_4b1590);
    loadFeatureGroup(4100, 0, 0);
    loadScripts(4100, 11);
    fn_4148da(10, 236);
    loadSoundByKey(1000, RESOURCE_TYPE(0, 'S', 'N', 'D'));
    loadSoundByKey(1004, RESOURCE_TYPE(0, 'S', 'N', 'D'));
    loadSoundByKey(1005, RESOURCE_TYPE(0, 'S', 'N', 'D'));
    loadSoundByKey(1006, RESOURCE_TYPE(0, 'S', 'N', 'D'));
    for (i = 4101; i <= 4103; i++)
        addView(0x8000, drawCels, runViewScript, i, i - 4091, 0, 0, 0);
    if (!*(short *)(g_4a4ba0 + 0x20))
        addIsleSettingViews();
    g_4b15b4 = addView(0x64000000, drawCels, runViewScript, 4100, 0, 0, 0, 0);
    addView(0x108a000, drawCels, runViewScript, 4110, 6, &entry, 0, 0);
    for (i = 4106; i <= 4109; i++)
        addView(0, drawCels, runViewScript, i, 0, 0, 0, 0);
    addView(0x4001000, drawIsleButtonsView, updateIsleButtons, 0, 0, 0, 0, 0);
    setViewPlaces(16, g_4a3324, 1);
    *party() = *waitingParties();
    waitingParties()->count = 0;
    makePartySnoids(0);
    exit = g_4a3368;
    spotTaken(&exit, 0, 500);
    for (i = 0; i < 16; i++) {
        skip = 0;
        spot = spotNear(&g_4a3324[i], 500, skip);
        for (j = 0; spot && j < i; j++)
            if (spot == sortedIds[j]) {
                skip++;
                spot = spotNear(&g_4a3324[i], 500, skip);
                j = 0;
            }
        sortedIds[i] = spot;
    }
    chooseSnoids(1, 0);
    updateViews();
    if (*(short *)(g_4a4ba0 + 0x20))
        *(short *)(g_4a4ba0 + 0x26) = 1;
    showIsleSetting(1);
    checkEnoughChosen();
    setGroupLists(g_4a330c, 2, (short)0xc000);
    highlightItemAt(1, 1);
    visitAllItems();
    g_4b1484.body.x = g_4a31cc[2].rect.left + 39;
    g_4b1484.body.y = g_4a31cc[2].rect.top + 31;
    g_4b1484.unknownF7 = 1;
    drawIsleButtons(0, 0, 0);
    drawFeatureButtons(0, 0, 0);
    if (g_4a7410) {
        if (savedGames)
            askLoadGame();
        updateViews();
    }
    showRect(&gameRect);
    fadeInViews();
    g_4b15a4 = 1;
    queueViewSound(30001, 0);
    if (g_4b0d56 != 1) {
        campHint((short *)(g_4a4ba0 + 0x28));
        if (*(short *)(g_4a4ba0 + 0x48) < 625 && g_4a7410 == 1)
            g_4b15b6 = 20042;
    } else {
        short made = countSnoidViews();
        short left = 625 - (*(short *)(g_4a4ba0 + 0x4a) + *(short *)(g_4a4ba0 + 0x4c) + *(short *)(g_4a4ba0 + 0x4e))
            - made;

        if (left > 0 && made < 625) {
            switch (randomBetween(1, 20)) {
            case 1:
                g_4b15b6 = 20043;
                break;
            case 10:
                g_4b15b6 = 20044;
                break;
            }
        }
    }
    if (g_4b15b6)
        queueViewSound(g_4b15b6, 1);
    g_4a7410 = 0;
    g_4b7562 = 0;
}

/* Closes the scene, leaving the party waiting (or, when it's leaving,
   taking it on). */
/* @zoombi32 0x0043eb13 */
void closeIsle()
{
    if (g_4b15a4) {
        g_4b15a4 = 0;
        short saved = setFreeAtOnce(1);

        clearViews();
        if (!viewsLocked) {
            if (g_4a48e6 || g_4b0d50 == 1) {
                party()->unknown2 = 0;
                party()->unknown4 = 0;
                *waitingParties() = *party();
                party()->count = 0;
            } else {
                waitingParties()->count = 0;
            }
        }
        unloadSounds();
        freeResource(&g_4b1590);
        freeResource(&g_4b158c);
        freeResource(&g_4b1594);
        setFreeAtOnce(saved);
        closeGameFile(&g_4b1588);
        fadeOutViews();
        fn_4624fc();
    }
}

/* Scene 3's frame: leaving once asked to (g_4b0d52) and sound 996 is
   done; g_4b15b8 is cleared while g_4b755a isn't set. */
/* @zoombi32 0x0043ebf4 */
void isleFrame()
{
    if (!isleBusy && g_4b15a4) {
        isleBusy = 1;
        updateViews();
        if (g_4b0d52) {
            if (isSoundPlaying(996, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
                isleBusy = 0;
                return;
            }
            if (viewsLocked || !g_4b755a || g_4b755c >= 1)
                leaveIsleIfAsked();
        }
        if (!g_4b755a && g_4b15b8)
            g_4b15b8 = 0;
        isleBusy = 0;
    }
}

/* Scene 3's keys: 23 brings back the two views g_4b15b0 and g_4b15b2
   (adding them first if the game state's +0x20 is set). Returns whether it
   handled the key. */
/* @zoombi32 0x0043ec8c */
short isleKey(unsigned short key)
{
    short handled = 0;

    switch (key) {
    case 23:
        if (*(short *)(g_4a4ba0 + 0x20))
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
   g_4b15b6 first, and redraws the panel's third button if the Zoombini's
   completeness changes. */
/* @zoombi32 0x0043ecbb */
void featureButtonClicked(short button)
{
    ShortRect rect;
    short group;
    short chosen;

    if (leaveIsleIfAsked())
        return;
    if (g_4b15b6 && isSoundPlaying(g_4b15b6, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
        stopSounds(g_4b15b6, RESOURCE_TYPE(0, 'S', 'N', 'D'));
        g_4b15b6 = 0;
    }
    rect = g_4a2efc[button].rect;
    group = 0;
    while (button >= 6) {
        button -= 5;
        group++;
    }
    chosen = g_4b1484.features[group];
    if (chosen && button == chosen) {
        queueViewSound(1004, 0);
        chosen += group * 5;
        drawFeatureButtons(chosen, 0, &rect);
        showRect(&rect);
        g_4b1484.features[group] = 0;
    } else if (isleAllowsFeature(group, button - 1)) {
        if (chosen) {
            chosen += group * 5;
            drawFeatureButtons(chosen, 0, &rect);
            showRect(&rect);
        }
        queueViewSound(1000, 0);
        drawFeatureButtons(group * 5 + button, 1, &rect);
        showRect(&rect);
        g_4b1484.features[group] = button;
    } else {
        queueViewSound(1008, 0);
    }
    short allowed = zoombiniMadeAllowed();

    if (allowed != g_4b15aa) {
        g_4b15aa = allowed;
        drawIsleButtons(3, 1, 1);
    }
    g_4b15ac = 1;
}

/* Leaves the scene if asked to (g_4b0d52 names where to): returns whether
   it did. */
/* @zoombi32 0x0043ee2a */
short leaveIsleIfAsked()
{
    if (g_4b0d52) {
        g_4b0d50 = g_4b0d52;
        g_4b0d52 = 0;
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
 * queue back to be remade (1007). Stops sound g_4b15b6 first.
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
    if (g_4b15b6 && isSoundPlaying(g_4b15b6, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
        stopSounds(g_4b15b6, RESOURCE_TYPE(0, 'S', 'N', 'D'));
        g_4b15b6 = 0;
    }
    rect = g_4a31cc[button].rect;
    getCursorPosition(&cursor);
    switch (button) {
    case 1:
        if (!g_4b15a8 && g_4b15aa && *(short *)(g_4a4ba0 + 0x48) < 625) {
            if (g_4b8803 && addModifierKeys(0) == 0x800 && !countSnoidViews()) {
                *(short *)(g_4a4ba0 + 0x48) = 624;
                *(short *)(g_4a4ba0 + 0x4e) = 624;
            }
            g_4afb32 = 1;
            g_4b15b8 = 1;
            g_4b755a++;
            queueViewSound(1005, 0);
            drawIsleButtons(button, 1, 1);
            waitForEventFor(0, 2, 0, 1);
            countZoombiniMade(1);
            isleQueue(&where, &slot);
            id = placeSnoid(&g_4b1484, 0, 148, 215, where.x, where.y);
            sortedIds[slot] = id;
            (*(short *)(g_4a4ba0 + 0x48))++;
            drawIsleButtons(button, 0, 1);
            sortViews();
            checkEnoughChosen();
            makeName(g_4b157d, 10);
            drawIsleButtons(3, 1, 1);
        }
        g_4b15aa = zoombiniMadeAllowed();
        break;
    case 2:
        queueViewSound(snoidSound(&g_4b1484, randomBetween(0, 12)), 0);
        break;
    case 3:
        if (g_4b15aa) {
            queueViewSound(1000, 0);
            makeName(g_4b157d, 10);
            drawIsleButtons(button, 1, 1);
        }
        break;
    case 4:
        if (*(short *)(g_4a4ba0 + 0x48) < 625) {
            queueViewSound(1006, 0);
            drawIsleButtons(button, 1, 1);
            if (!g_4b15a8 && addModifierKeys(0) == 0x800) {
                pickZoombiniMade(1);
                drawFeatureButtons(0, 0, &rect);
                showRect(&rect);
                isleQueue(&where, &slot);
                when = clockTime();
                while (!g_4b15a8) {
                    g_4afb32 = 1;
                    countZoombiniMade(1);
                    id = placeSnoid(&g_4b1484, when, 148, 215, where.x, where.y);
                    g_4b15b8 = 1;
                    g_4b755a++;
                    sortedIds[slot] = id;
                    (*(short *)(g_4a4ba0 + 0x48))++;
                    if (!*(short *)(g_4a4ba0 + 0x20))
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
                pickZoombiniMade(g_4b15aa);
                drawFeatureButtons(0, 0, &rect);
                showRect(&rect);
                drawIsleButtons(3, 1, 1);
            }
            g_4b15aa = zoombiniMadeAllowed();
        } else {
            queueViewSound(1008, 0);
        }
        break;
    case 5:
        queueViewSound(999, 0);
        drawIsleButtons(button, 1, 1);
        waitForEventFor(0, 2, 0, 1);
        drawIsleButtons(button, 0, 1);
        g_4b0d50 = 1;
        closeIsle();
        break;
    case 6:
        if (g_4b15a8) {
            short order[4] = {11, 12, 6, 7};

            queueViewSound(996, 0);
            drawIsleButtons(button, 1, 1);
            waitForEventFor(0, 2, 0, 1);
            drawIsleButtons(button, 0, 1);
            g_4b755c = g_4b755a = 0;
            where.x = where.y = 0;
            for (slot = 0; g_4b755a < 1 && slot < 4; slot++) {
                id = sortedIds[order[slot]];
                if (id) {
                    snoid = findSnoid(id, 1);
                    if (snoid && !snoid->unknownF4) {
                        view = findView(id);
                        view->nextUpdate = clockTime() + g_4b755a * 60;
                        snoid->unknownEa = -1;
                        snoid->targetX = 544;
                        snoid->targetY = 264;
                        setSnoidAction(snoid, 7, 0);
                        g_4b755a++;
                    }
                }
            }
            g_4b0d52 = 7;
        } else {
            short made = countSnoidViews();
            short left = 625 - (*(short *)(g_4a4ba0 + 0x4a) + *(short *)(g_4a4ba0 + 0x4c) + *(short *)(g_4a4ba0 + 0x4e))
                - made;

            if (left > 0 && made < 625) {
                switch (randomBetween(1, 2)) {
                case 1:
                    g_4b15b6 = 20043;
                    break;
                case 2:
                    g_4b15b6 = 20044;
                    break;
                }
                if (g_4b15b6)
                    queueViewSound(g_4b15b6, 1);
            }
        }
        break;
    case 7:
        if (!g_4b15b8) {
            short grab;

            view = viewAt(cursor, 1, 1);
            grab = 0;
            if (view) {
                grab = ((Snoid *)&view->body)->unknownF4;
                if (!grab || grab == 6 || grab == 4)
                    grab = 1;
                else
                    grab = 0;
            }
            if (grab) {
                other = findView(g_4b15b4);
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
                        g_4afb32 = 1;
                        claimPlacedView(1, 0);
                        queueViewSound(1007, 0);
                        if (*(short *)(g_4a4ba0 + 0x48) > 0)
                            (*(short *)(g_4a4ba0 + 0x48))--;
                        for (slot = 0; slot < 4; slot++) {
                            g_4b1484.features[slot] = ((Snoid *)&view->body)->features[slot];
                            ((Snoid *)&view->body)->features[slot] = 0;
                        }
                        for (slot = 0; slot < 10; slot++)
                            g_4b157d[slot] = ((Snoid *)&view->body)->name[slot];
                        countZoombiniMade(0);
                        drawFeatureButtons(0, 0, &rect);
                        showRect(&rect);
                        drawIsleButtons(3, 1, 1);
                        checkEnoughChosen();
                        g_4b15aa = zoombiniMadeAllowed();
                        g_4b15ac = 1;
                        isleQueue(0, 0);
                        if (*(short *)(g_4a4ba0 + 0x48) == 624)
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

/* Draws the panel's buttons (1-7, from the bank g_4b15a0; some lit, some
   greyed by the scene's state), or just button `which`, lit or not; the
   second shows the Zoombini being made, the third its name (g_4b157d, if
   g_4b15aa). Shows the area drawn if `show`. */
/* @zoombi32 0x0043f5ea */
void drawIsleButtons(short which, short lit, short show)
{
    short y;
    short count;
    ShortRect rect;
    ShortRect bounds = g_4a3376;
    Color saved;
    short i;
    short x;
    short image;

    if (!which) {
        i = 0;
        count = 7;
        bounds = g_4a31cc[1].rect;
        unionRect(&bounds, &g_4a31cc[7].rect);
    } else {
        i = which - 1;
        count = i + 1;
        bounds = g_4a31cc[which].rect;
    }
    for (; i < count; i++) {
        x = g_4a31cc[i + 1].rect.left;
        y = g_4a31cc[i + 1].rect.top;
        image = 0;
        switch (i) {
        case 0:
            image = 2;
            if (*(short *)(g_4a4ba0 + 0x48) >= 625 || g_4b15a8 || !g_4b15aa) {
                lit = 0;
                image = 1;
            }
            break;
        case 1:
            drawZoombiniParts(&g_4b1484);
            break;
        case 2:
            if (g_4b9684 & 0x10)
                return;
            drawImageData((unsigned short *)(g_4b15a0->offsets[13] + (char *)g_4b15a0), x, y, 8);
            rect = g_4a31cc[i + 1].rect;
            saved = setForeColor(Color(45));
            if (g_4b15aa) {
                rect.top++;
                rect.left += 4;
                drawText(rect, 0x22, g_4b157d, 0xffff);
                rect.top--;
                rect.left -= 4;
            }
            setForeColor(saved);
            break;
        case 3:
            image = 4;
            if (!g_4b15a8 && g_4b15a6)
                image = 6;
            break;
        case 4:
            image = 11;
            break;
        case 5:
            image = 9;
            if (!g_4b15a8) {
                lit = 0;
                image = 8;
            }
            break;
        }
        if (image) {
            if (lit)
                image++;
            drawImageData((unsigned short *)(g_4b15a0->offsets[image] + (char *)g_4b15a0), x, y, 8);
        }
    }
    if (show)
        showRect(&bounds);
}

/* Draws the feature buttons (1-20, from the bank g_4b1598; each lit if it's
   the feature chosen in its group of five for the Zoombini being made), or
   just button `which`, lit or not; stores the area drawn in *bounds. */
/* @zoombi32 0x0043f856 */
void drawFeatureButtons(short which, short lit, ShortRect *bounds)
{
    ShortRect rect = g_4a337e;
    short i;
    short image;

    if (!which) {
        unionRect(&rect, &g_4a2efc[1].rect);
        unionRect(&rect, &g_4a2efc[20].rect);
        for (i = 0; i < 20; i++) {
            image = i + i + 1;
            if (i % 5 + 1 == g_4b1484.features[i / 5])
                image++;
            drawImageData((unsigned short *)(g_4b1598->offsets[image] + (char *)g_4b1598), g_4a2efc[i + 1].rect.left,
                          g_4a2efc[i + 1].rect.top, 8);
        }
    } else {
        rect = g_4a2efc[which].rect;
        i = which - 1;
        image = i + i + 1;
        if (lit)
            image++;
        drawImageData((unsigned short *)(g_4b1598->offsets[image] + (char *)g_4b1598), g_4a2efc[i + 1].rect.left,
                      g_4a2efc[i + 1].rect.top, 8);
    }
    if (bounds)
        *bounds = rect;
}

/* Draws image `which` of the bank g_4b159c at (x, y) by its hot spot
   (g_4a3386, g_4a339c). */
/* @zoombi32 0x0043f985 */
void drawIsleImage(short which, short x, short y)
{
    if (which)
        drawImageData((unsigned short *)(g_4b159c->offsets[which] + (char *)g_4b159c), x - g_4a3386[which],
                      y - g_4a339c[which], 8);
}

/* Draws a Zoombini from its parts in the bank g_4b159c where it stands:
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
        if (g_4b1484.features[i] < 0 || g_4b1484.features[i] > 5)
            g_4b1484.features[i] = 0;
        if (!g_4b1484.features[i])
            return 0;
    }
    hair = g_4b1484.features[0] - 1;
    eyes = g_4b1484.features[1] - 1;
    nose = g_4b1484.features[2] - 1;
    feet = g_4b1484.features[3] - 1;
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

    g_4b15aa = 0;
    while (!g_4b15aa && tries < 64) {
        tries++;
        if (tries >= 64) {
            found = 0;
            rename = 1;
            for (hair = 0; !found && hair < 5; hair++)
                for (eyes = 0; !found && eyes < 5; eyes++)
                    for (nose = 0; !found && nose < 5; nose++)
                        for (feet = 0; !found && feet < 5; feet++)
                            if (zoombiniCounts()[hair][eyes][nose][feet] < 2) {
                                g_4b1484.features[0] = hair + 1;
                                g_4b1484.features[1] = eyes + 1;
                                g_4b1484.features[2] = nose + 1;
                                g_4b1484.features[3] = feet + 1;
                            }
        } else {
            for (i = 0; i < 4; i++)
                if (!g_4b1484.features[i] || rename)
                    g_4b1484.features[i] = randomBetween(1, 5);
        }
        g_4b15aa = zoombiniMadeAllowed();
        if (!g_4b15aa)
            rename = 1;
    }
    if (rename)
        makeName(g_4b157d, 10);
    g_4b15ac = 1;
}

/* The view drawing the isle's buttons and the feature buttons. */
/* @zoombi32 0x0043fc7d */
void drawIsleButtonsView(View *)
{
    drawIsleButtons(0, 0, 0);
    drawFeatureButtons(0, 0, 0);
}

/* The scene's update: adds to the region to redraw the buttons whose state
   changed (24 with the cheat modifier, 26 and 21 with g_4b15a8, 21 with
   g_4b15aa, 22 when g_4b15ac asks). */
/* @zoombi32 0x0043fc9a */
void updateIsleButtons(View *, short region)
{
    if (!g_4b9684 && *(short *)(g_4a4ba0 + 0x48) < 625 && addModifierKeys(0) == 0x800) {
        if (!g_4b15a6) {
            g_4b15a6 = 1;
            unionRgnRect(region, &g_4a2efc[24].rect);
        }
    } else if (g_4b15a6) {
        g_4b15a6 = 0;
        unionRgnRect(region, &g_4a2efc[24].rect);
    }
    if (g_4b15a8) {
        if (!g_4a33b2) {
            g_4a33b2 = 1;
            unionRgnRect(region, &g_4a2efc[26].rect);
            unionRgnRect(region, &g_4a2efc[21].rect);
        }
    } else if (g_4a33b2) {
        g_4a33b2 = 0;
        unionRgnRect(region, &g_4a2efc[26].rect);
        unionRgnRect(region, &g_4a2efc[21].rect);
    }
    if (!g_4b15aa) {
        if (g_4a33b4) {
            g_4a33b4 = 0;
            unionRgnRect(region, &g_4a2efc[21].rect);
        }
    } else if (!g_4a33b4) {
        g_4a33b4 = 1;
        unionRgnRect(region, &g_4a2efc[21].rect);
    }
    if (g_4b15ac) {
        g_4b15ac = 0;
        unionRgnRect(region, &g_4a2efc[22].rect);
    }
}

/* Counts the Zoombini being made (g_4b1484) in or out of the numbers of
   each kind (at most two), noting in g_4b15aa whether it's complete (and,
   when adding, not one too many). */
/* @zoombi32 0x0043fdcc */
void countZoombiniMade(short add)
{
    Snoid *made = &g_4b1484;
    short i;
    short hair;
    short eyes;
    short nose;
    short feet;

    if (add) {
        g_4b15aa = zoombiniMadeAllowed();
    } else {
        g_4b15aa = 1;
        for (i = 0; i < 4; i++)
            if (!made->features[i])
                g_4b15aa = 0;
    }
    if (g_4b15aa) {
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

/* Shows the two views g_4b15b0 and g_4b15b2 by the setting at game state
   +0x26 (0-3), stepping it on unless `keep`. */
/* @zoombi32 0x0043ff1d */
void showIsleSetting(short keep)
{
    View *first;
    View *second;
    short a;
    short b;

    first = findView(g_4b15b0);
    second = findView(g_4b15b2);
    if (first && second) {
        if (!keep)
            (*(short *)(g_4a4ba0 + 0x26))++;
        if (*(short *)(g_4a4ba0 + 0x26) > 3)
            *(short *)(g_4a4ba0 + 0x26) = 0;
        switch (*(short *)(g_4a4ba0 + 0x26)) {
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
 * The queue of places (sortedIds, at g_4a3324): with `where`, gives the
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
                *where = g_4a3324[i];
                *slot = i;
                return;
            }
        return;
    }
    g_4b7564 = 1;
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
                        snoid->unknownEa = -1;
                        if (i + step < 17) {
                            *(Point *)&snoid->targetX = g_4a3324[i];
                            setSnoidAction(snoid, 7, 0);
                        } else {
                            snoid->targetX = 326;
                            snoid->targetY = 390;
                            setSnoidAction(snoid, 7, 0);
                            updateSnoidView(snoidView(snoid), removedRgn);
                            *(Point *)&snoid->targetX = g_4a3324[i];
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
    g_4b7564 = 0;
}

/* Adds the views g_4b15b0 (script 4104) and g_4b15b2 (4105) if they
   aren't there. */
/* @zoombi32 0x00440218 */
void addIsleSettingViews()
{
    if (!g_4b15b0)
        g_4b15b0 = addView(0x800c000, drawCels, runViewScript, 4104, 7, 0, 0, 0);
    if (!g_4b15b2)
        g_4b15b2 = addView(0x8008000, drawCels, runViewScript, 4105, 9, 0, 0, 0);
}

/* @zoombi32 0x0044027b */
short isleAllowsFeature(short, short)
{
    return 1;
}

/* Sets g_4b15a8 if there are Zoombinis chosen and either at least
   g_4b15ae of them or 625 counted in the game (the whole population). */
/* @zoombi32 0x00440286 */
void checkEnoughChosen()
{
    g_4b15a8 = 0;
    short count = countChosenSnoids();

    if (count)
        g_4b15a8 = count >= g_4b15ae || *(short *)(g_4a4ba0 + 0x48) >= 625;
}
