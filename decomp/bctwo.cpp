/*
 * bctwo (0x418698-0x41a404): 'bctwo.mhk'
 */

#include "zoombinis.h"
#include "basecamp.h"
#include "bctwo.h"
#include "debug.h"
#include "e2memory.h"
#include "features.h"
#include "focus.h"
#include "graphics.h"
#include "module_4623b8.h"
#include "net.h"
#include "platform.h"
#include "snoids.h"
#include "sound.h"
#include "town.h"
#include "view.h"

/* Resets scene 5's state. */
/* @zoombi32 0x00418698 */
void resetCamp2()
{
    bookView = camp2HoverButton = sceneDue = 0;
    camp2ClicksOff = view6000Running = camp2Dragging = 0;
    chosenCamp2Button = 0;
    view6002Next = 1;
    populationFull = 0;
}

/* Opens scene 5, the camp: the book of the Zoombinis waiting there (kept
   at +0x3688 in the game's state), the Zoombinis back at the camp and the
   party (which joins the book when it doesn't carry on), and a line by
   the camp's hint. */
/* @zoombi32 0x004186dc */
void openCamp2()
{
    short choices;
    short saved;
    short highest;
    Point places[16] = {{490, 372}, {458, 359}, {450, 384}, {412, 376}, {393, 398}, {365, 386},
                        {348, 405}, {321, 389}, {304, 410}, {278, 397}, {264, 417}, {234, 400},
                        {218, 420}, {197, 398}, {177, 418}, {152, 403}};
    short n; /* the loop, then the Zoombinis chosen, then the hint */
    short m; /* whether the party fitted, then the line to say */

    camp2Open = 0;
    resetCamp2();
    saved = soundOn;
    soundOn = 0;
    rosterChanged = 1;
    addSoundRange(20000, 29999, 1);
    addSoundRange(2000, 2099, 0);
    addSoundRange(6000, 6099, 1);
    bookEntries = (CampEntries *)(gameState + 0x3688);
    bookRow = bookEntries->row;
    bookCount = bookEntries->count;
    bookHighest = lastBookEntry();
    countBookEntry(-1);
    openGameFile(&camp2File, "bctwo.mhk");
    setCurrentMap(camp2File);
    loadPaths(1000);
    loadDragCursors(10000);
    loadTerrain(100);
    drawBackdrop(5000);
    loadFeatureGroup(6000, 0, 0);
    loadFeatureGroup(7000, 1, 0);
    loadScripts(6000, 14);
    addScripts(7000, 16, 0);
    loadShapeList(&bookImages, 8000, 0, 1);
    loadShapeList(&camp2Images, 9000, 0, 1);
    bookView = addView(0xc000, drawBook, scrollBook, 0, 6, 0, 0, 0);
    addView(0x9000, drawCamp2Buttons2, 0, 0, 0, 0, 0, 0);
    addView(0x1000, drawCamp2Buttons1, updateCamp2Button0, 0, 0, 0, 0, 0);
    for (n = 0; n < 16; n++)
        placedViews[n] = addView(0x108a000, drawCels, runViewScript, n + 7000, 7, &places[n], 0, 0);
    campThingViews[9] = addView(0x5188000, drawCels, runViewScript, 6000, 6, 0, 0, 0);
    campThingViews[0] = addView(0x188000, drawCels, runViewScript, 6005, 6, 0, 0, 0);
    campThingViews[1] = addView(0x188000, drawCels, runViewScript, 6011, 6, 0, 0, 0);
    campThingViews[2] = addView(0x1188000, drawCels, runViewScript, 6010, 6, 0, 0, 0);
    campThingViews[3] = addView(0x4188000, drawCels, runViewScript, 6002, 6, 0, 0, 0);
    campThingViews[4] = addView(0x4180000, drawCels, runViewScript, 6004, 6, 0, 0, 0);
    campThingViews[5] = addView(0x1188000, drawCels, runViewScript, 6009, 6, 0, 0, 0);
    campThingViews[6] = addView(0x5188000, drawCels, runViewScript, 6006, 6, 0, 0, 0);
    campThingViews[7] = addView(0x5188000, drawCels, runViewScript, 6007, 6, 0, 0, 0);
    campThingViews[8] = addView(0x5188000, drawCels, runViewScript, 6008, 6, 0, 0, 0);
    copyPaletteRange(10, 236);
    setViewPlaces(16, places, 1);
    if (party()->count)
        makePartySnoids(0);
    n = countChosenSnoids();
    *(short *)(gameState + 0x4c) += n;
    *party() = waitingParties()[2];
    waitingParties()[2].count = 0;
    waitingParties()[2].unknown2 = 1;
    waitingParties()[2].unknown4 = 1;
    if (n) {
        if (!party()->unknown2 && countPresentTravellers()) {
            highest = bookHighest;
            m = addPartyToBook();
            bookCount += countPresentTravellers();
            bookHighest = lastBookEntry();
            countBookEntry(-1);
            if (m) {
                bookRow = (highest + 1) / 5 % bookRows;
                countBookEntry(-1);
            }
            party()->unknown2 = 1;
        }
    } else
        g_4b7562 = 1;
    makePartySnoids(1);
    enterSnoids(-20);
    updateViews();
    if (n)
        staggerSnoids(45, 30);
    populationFull = *(short *)(gameState + 0x48) >= 625
               && *(short *)(gameState + 0x4a) + *(short *)(gameState + 0x4c)
                          + waitingParties()[0].count
                      < 16;
    if (populationFull) {
        short count = countChosenSnoids();

        enoughChosen = count
                   && *(short *)(gameState + 0x4a) + *(short *)(gameState + 0x4c)
                              + waitingParties()[0].count
                          <= count;
        camp2EnoughDrawn = enoughChosen;
    } else
        camp2EnoughDrawn = enoughChosen = countChosenSnoids() >= 16;
    setGroupLists(campGroups, 2, (short)0xc000);
    highlightItemAt(1, 1);
    drawCamp2Button(0, 0, 0, 0);
    showRect(&shownGameRect);
    fadeInViews();
    camp2Open = 1;
    m = 0;
    n = -1;
    if (puzzleLeft) {
        n = campHint((short *)(gameState + 0x3e));
        puzzleLeft = 0;
    }
    if (n == 2 && !*(short *)(gameState + 0x40) && *(short *)(gameState + 0x4c) <= 16) {
        n = 1;
        *(short *)(gameState + 0x3e) &= 0xcfff;
    }
    choices = 3;
    if (*(short *)(gameState + 0x3e) & 0x3000)
        choices = 4;
    switch (n) {
    case 0:
        switch (randomBetween(1, choices)) {
        case 1:
            m = 20084;
            break;
        case 2:
            m = 20085;
            break;
        case 3:
            m = 20082;
            break;
        case 4:
            m = 20083;
            break;
        }
        break;
    case 1:
        m = 20082;
        break;
    case 2:
    case 12:
        m = 20083;
        break;
    case 5:
        m = 20082;
        break;
    }
    resetViewClock();
    soundOn = saved;
    if (m)
        queueViewSound(m, 0);
}

/* Closes scene 5: the party stays at the camp (all of it when leaving
   for the map or scene 1, else those present, counted off the town's
   population), and the book is tidied. */
/* @zoombi32 0x00418d40 */
void closeCamp2()
{
    short saved;

    if (camp2Open) {
        camp2Open = 0;
        saved = setFreeAtOnce(1);
        clearViews();
        if (!viewsLocked) {
            if (leavingGame || pendingScene == 1) {
                party()->unknown2 = 0;
                party()->unknown4 = 0;
                waitingParties()[2] = *party();
                party()->count = 0;
            } else {
                party()->unknown2 = 1;
                party()->unknown4 = 0;
                waitingParties()[2] = *party();
                party()->unknown2 = 0;
                party()->unknown4 = 1;
                *(short *)(gameState + 0x4c) -= countPresentTravellers();
            }
            dropEmptyBookRows();
            countBookEntry(-1);
        }
        unloadSounds();
        freeShapeList(&camp2Images);
        freeShapeList(&bookImages);
        setFreeAtOnce(saved);
        closeGameFile(&camp2File);
        fadeOutViews();
        showBusyCursor();
    }
}

/* Scene 5's frame: leaves for the scene due (once sound 996 is done);
   else shows the drag cursor for the button (4-7) under the cursor, and
   keeps view view6000's script 6001 going while view6000Running. */
/* @zoombi32 0x00418e62 */
void camp2Frame()
{
    Point where;
    short button;
    short i;
    View *view;

    if (inCamp2Frame || !camp2Open)
        return;
    inCamp2Frame = 1;
    updateViews();
    if (sceneDue) {
        if (isSoundPlaying(996, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
            inCamp2Frame = 0;
            return;
        }
        if (viewsLocked || !snoidsOnTheirWay || snoidsArrived >= 1) {
            pendingScene = sceneDue;
            sceneDue = 0;
            setCurrentMap(0);
            closeCamp2();
        }
    } else {
        button = 0;
        if (!camp2Dragging && !dialogFlags) {
            getCursorPosition(&where);
            for (i = 3; !button && i < 7; i++)
                if (ptInRect(&camp2Buttons[i].rect, where))
                    button = i - 2;
        }
        setDragCursor(button);
        if (view6000Running) {
            view = findView(view6000);
            if (!view->body.running) {
                setViewScript(view, 6001, 1);
                view->flags = 0x88000;
            }
        }
    }
    playAmbientSound();
    inCamp2Frame = 0;
}

/* Scene 5's clicks: once a scene is due, leaves for it; 1 leaves for
   the map (scene 16) when enoughChosen, else says a line; 3 leaves for scene
   1; 4-7 scroll the book while held. */
/* @zoombi32 0x00418fa7 */
void camp2Clicked(short which)
{
    Point where;

    if (sceneDue) {
        pendingScene = sceneDue;
        sceneDue = 0;
        setCurrentMap(0);
        closeCamp2();
        return;
    }
    getCursorPosition(&where);
    switch (which) {
    case 1:
        if (enoughChosen) {
            queueViewSound(996, 0);
            drawCamp2Button(which, 1, 0, 1);
            waitForEventFor(0, 2, 0, 1);
            drawCamp2Button(which, 0, 0, 1);
            markPlacedSnoids();
            sendSnoids(680, 316, 45);
            sceneDue = 16;
        } else
            switch (randomBetween(1, 3)) {
            case 1:
                queueViewSound(20084, 0);
                break;
            case 2:
                queueViewSound(20085, 0);
                break;
            case 3:
                queueViewSound(20082, 0);
                break;
            }
        break;
    case 3:
        queueViewSound(999, 0);
        drawCamp2Button(which, 1, 0, 1);
        waitForEventFor(0, 2, 0, 1);
        drawCamp2Button(which, 0, 0, 1);
        pendingScene = 1;
        closeCamp2();
        break;
    case 4:
    case 5:
    case 6:
    case 7:
        chosenCamp2Button = which;
        drawCamp2Button(which, 1, 0, 1);
        do {
            scrollPressed = which - 3;
            lightScrollButton(0, 0);
            mainLoopEvents();
        } while (isButtonStillDown(1));
        lightScrollButton(1, 0);
        chosenCamp2Button = 0;
        drawCamp2Button(which, 0, 0, 1);
        break;
    }
}

/* The camp's drags (event 1 pressed, 2 dragging): a Zoombini picked out
   of the book (taken from its entry) or one on the ground is dragged;
   dropped on an empty cell of the book it goes in it, and a Zoombini
   picked out of the book goes back if dropped nowhere useful (or with over
   32 about). enoughChosen then says whether enough are chosen to leave.
   Clicking elsewhere starts the camp's thing there (campThingRects). */
/* @zoombi32 0x0041914d */
void campDragged(short event)
{
    Point where;
    ShortRect rect;
    ShortRect unused; /* only takes stack space */
    short slot;
    short added;
    short picked;
    short dropped;
    short place;
    Snoid *snoid;
    short moved;
    View *view;
    short entry;
    short i;

    if (sceneDue) {
        pendingScene = sceneDue;
        sceneDue = 0;
        setCurrentMap(0);
        closeCamp2();
        return;
    }
    if (camp2ClicksOff && event != 2)
        return;
    getCursorPosition(&where);
    picked = 0;
    view = 0;
    if (event == 1 && snoidsOnTheirWay <= 0) {
        view = viewAt(where, 1, 1);
        if (!view) {
            rect.left = rect.right = where.x;
            rect.top = rect.bottom = where.y;
            entry = bookEntryAt(bookRow, rect, 1);
            if (entry >= 0) {
                if (bookCount > 0)
                    bookCount--;
                initSnoid(&bookSnoid);
                bookSnoid.zoombini = bookEntries->entries[entry].zoombini;
                for (i = 0; i < 10; i++)
                    bookSnoid.name[i] = bookEntries->entries[entry].name[i];
                bookSnoid.body.x = where.x;
                bookSnoid.body.y = where.y;
                bookEntries->entries[entry].zoombini = 0;
                refreshBook();
                added = addSnoidView(&bookSnoid, 0);
                if (added) {
                    view = findView(added);
                    scrollPressed = -1;
                    picked = 1;
                    event = 2;
                }
            }
        } else
            event = 2;
    }
    switch (event) {
    case 1:
        break;
    case 2:
        if (!view && snoidsOnTheirWay <= 0)
            view = viewAt(where, 1, 1);
        if (view) {
            i = 0;
            unusedCamp2Long = 0;
            camp2Dragging = 1;
            dropped = dragSnoid(view, where, 0, 0);
            camp2Dragging = 0;
            place = heldPlaceNumber();
            snoid = viewSnoid(view);
            moved = !(snoid->targetX == snoid->body.x && snoid->targetY == snoid->body.y);
            snoid->unknownF7 = place > 0;
            if (snoid->unknownF7)
                snoid->unknownF8 = 1;
            rect = view->body.bounds;
            if (sectRect(&rect, &bookArea)) {
                rect = view->body.bounds;
                slot = bookEntryAt(bookRow, rect, 0);
                if (slot >= 0) {
                    countBookEntry(slot);
                    bookEntries->entries[slot].zoombini = viewSnoid(view)->zoombini;
                    for (i = 0; i < 10; i++)
                        bookEntries->entries[slot].name[i] = viewSnoid(view)->name[i];
                    deleteView(view->id);
                    refreshBook();
                    scrollPressed = -1;
                    picked = 0;
                    i = 1;
                }
            }
            if (picked) {
                short back = countSnoidViews() > 32;

                if (!back && !place && moved)
                    back = 1;
                if (back) {
                    bookEntries->entries[entry].zoombini = bookSnoid.zoombini;
                    for (i = 0; i < 10; i++)
                        bookEntries->entries[entry].name[i] = bookSnoid.name[i];
                    removeView(added, 1);
                }
                scrollPressed = -1;
            } else if (dropped && !place && moved && !i) {
                claimPlacedView(dropped, view->id);
                snoid->unknownF7 = 1;
                snoid->unknownF8 = 1;
            }
            if (populationFull) {
                short chosen = countChosenSnoids();

                enoughChosen = chosen
                           && *(short *)(gameState + 0x4a) + *(short *)(gameState + 0x4c)
                                      + waitingParties()[0].count
                                  <= chosen;
            } else
                enoughChosen = countChosenSnoids() >= 16;
        } else
            for (i = 0; i < 10; i++)
                if (ptInRect(&campThingRects[i], where)) {
                    view = findView(campThingViews[i]);
                    if (view && !view->body.running) {
                        switch (i) {
                        case 3: {
                            short script;

                            if (view6002Next) {
                                view6002Next = 0;
                                script = 6002;
                            } else {
                                view6002Next = 1;
                                script = 6003;
                            }
                            setViewScript(view, script, 1);
                            break;
                        }
                        case 1:
                            if (view->kind == 6013)
                                setViewScript(view, 6011, 1);
                            else
                                setViewScript(view, view->kind + 1, 1);
                            break;
                        case 9:
                            if (!view6000Running) {
                                setViewScript(view, 0, 1);
                                view6000Running = 1;
                            }
                            break;
                        default:
                            setViewScript(view, 0, 1);
                            break;
                        }
                        i = 10;
                        loadViewSounds(view->id, 1);
                    }
                }
        break;
    }
}

/* @zoombi32 0x004196a8 */
long camp2Key(long)
{
    return 0;
}

/* Draws the camp's button `button` (1-7), lit or not, or with 0 a group
   of them (`group` 1: 1-3, 2: 4-7, else all); button 1 is out while
   enoughChosen is clear, 4-7 lit for the one chosen (chosenCamp2Button). With `show`
   shows them (redrawing the dragged view over 4-7). */
/* @zoombi32 0x004196b1 */
void drawCamp2Button(short button, short lit, short group, short show)
{
    short y;
    ShortRect unused; /* unused, like `color`: they only take stack space */
    ShortRect rect = camp2ButtonsRect;
    Color color;
    short dragging = 0;
    short first;
    short last;
    short image;
    short x;

    if (!button) {
        switch (group) {
        case 1:
            first = 0;
            last = 3;
            break;
        case 2:
            first = 3;
            last = 7;
            break;
        default:
            first = 0;
            last = 7;
            break;
        }
        rect = camp2Buttons[first].rect;
        unionRect(&rect, &camp2Buttons[last - 1].rect);
    } else {
        first = button - 1;
        last = first + 1;
        rect = camp2Buttons[button - 1].rect;
    }
    for (; first < last; first++) {
        x = camp2Buttons[first].rect.left;
        y = camp2Buttons[first].rect.top;
        image = 0;
        switch (first) {
        case 0:
            image = 1;
            if (!enoughChosen) {
                lit = 0;
                image = 15;
            }
            break;
        case 2:
            image = 5;
            break;
        case 3:
        case 4:
        case 5:
        case 6:
            dragging = 1;
            image = (first - 3) * 2 + 7;
            lit = 0;
            if (chosenCamp2Button - 1 == first)
                lit = 1;
            break;
        }
        if (image) {
            if (lit)
                image++;
            drawImage(camp2Images, image, x, y, 8, 17);
        }
    }
    if (show) {
        if (dragging)
            drawDragCursor(viewListEnd(0));
        showRect(&rect);
    }
}

/* The entry shown from row `row` on (25 of them, in 5 lines of 5) that
   `rect`'s corner is on, if `taken`; else the empty one `rect` covers
   most (by over 625 pixels); -1 if none. */
/* Not exact: the original keeps `column` in ebx (with `row`) and `n` in
   esi; here they are the other way round. */
/* @zoombi32 0x004198be */
short bookEntryAt(short row, ShortRect rect, short taken)
{
    short found = -1;
    short i;
    short index;
    short count;
    short line;
    ShortRect cell;
    short best;
    Point where;
    short column;
    short n;
    short zoombini;
    short area;

    row %= bookRows;
    where.x = rect.left;
    where.y = rect.top;
    count = 25;
    index = row * 5;
    column = line = 0;
    best = 0;
    for (i = 0; i < count; i++, index++) {
        n = index % bookSlots;
        zoombini = (short)bookEntries->entries[n].zoombini;
        if (taken && zoombini || !taken && !zoombini) {
            if (zoombini) {
                cell = bookEntries->entries[n].rect;
                if (ptInRect(&cell, where))
                    return n;
            } else {
                cell.left = cellX[line * 2 + 1] - 30;
                cell.top = cellY[line * 2 + 1][column] - 30;
                cell.right = cell.left + 60;
                cell.bottom = cell.top + 60;
                if (sectRect(&cell, &rect)) {
                    area = (cell.right - cell.left) * (cell.bottom - cell.top);
                    if (area > 625 && area > best) {
                        best = area;
                        found = n;
                    }
                }
            }
        }
        if (++column >= 5) {
            column = 0;
            line++;
        }
    }
    return found;
}

/* View draw callbacks: draw buttons 1-3, and 4-7. */
/* @zoombi32 0x0041983f */
void drawCamp2Buttons1(View *)
{
    drawCamp2Button(0, 0, 1, 0);
}

/* @zoombi32 0x00419853 */
void drawCamp2Buttons2(View *)
{
    drawCamp2Button(0, 0, 2, 0);
}

/* A view update: redraws button 0 whenever enoughChosen changes. */
/* @zoombi32 0x00419867 */
void updateCamp2Button0(View *, short region)
{
    if (enoughChosen) {
        if (!camp2EnoughDrawn) {
            camp2EnoughDrawn = 1;
            unionRgnRect(region, &camp2Buttons[0].rect);
        }
    } else if (camp2EnoughDrawn) {
        camp2EnoughDrawn = 0;
        unionRgnRect(region, &camp2Buttons[0].rect);
    }
}

/* Counts entry `n` (0-624) in: bookCount of them, bookHighest the highest;
   bookSlots is then that rounded up past a multiple of 5 (50-625), and
   bookRows a fifth of it; bookRow stays within 5 of the end. The first
   entry keeps bookRow and the count. */
/* @zoombi32 0x00419e49 */
void countBookEntry(short n)
{
    if (n >= 0 && n < 625 && bookCount < 625) {
        bookCount++;
        if (n > bookHighest)
            bookHighest = n;
    }
    bookSlots = (bookHighest + 10) / 5 * 5;
    if (bookSlots > 625)
        bookSlots = 625;
    if (bookSlots < 50)
        bookSlots = 50;
    bookRows = bookSlots / 5;
    if (bookRow > bookRows - 5)
        bookRow = bookRows - 5;
    bookEntries->row = bookRow;
    bookEntries->count = bookCount;
}

/* Draws the camp's book of waiting Zoombinis (a view draw callback): 25
   of them from row bookRow on, in 5 lines of 5 (half a line lower, and
   one line more, while bookHalfLine), noting where each is drawn. */
/* Not exact: the original keeps `column` in ebx, `n` in esi and `line` in
   edi; here they get edi, ebx and esi (declaration order and `register`
   don't change that; bookEntryAt has the same swap). */
/* @zoombi32 0x00419c3a */
void drawBook(View *)
{
    short image;
    short count;
    short y;
    short i;
    short index;
    short dx;
    short dy;
    Snoid snoid;
    short n;
    short column;
    short line;
    short x;

    initSnoid(&snoid);
    bookRow %= bookRows;
    count = 25;
    index = bookRow * 5;
    column = line = 0;
    if (bookHalfLine) {
        image = 1;
        count += 5;
        dy = -3;
        dx = -1;
    } else {
        image = 3;
        dy = 0;
        dx = 0;
    }
    drawImage(bookImages, image, 140, 23, 0, 17);
    for (i = 0; i < count; i++, index++) {
        n = index % bookSlots;
        if (bookEntries->entries[n].zoombini) {
            if (bookHalfLine) {
                x = cellX[line * 2];
                y = cellY[line * 2][column];
            } else {
                x = cellX[line * 2 + 1];
                y = cellY[line * 2 + 1][column];
            }
            snoid.body.clipped = 0;
            snoid.unknownC0 = -1;
            snoid.body.frame = 0;
            snoid.body.frameOffset = 2;
            snoid.zoombini = bookEntries->entries[n].zoombini;
            snoid.body.x = x;
            snoid.body.y = y;
            setSnoidFacing(&snoid, 0);
            layOutSnoid(&snoid, 0);
            bookEntries->entries[n].rect = snoid.body.bounds;
            drawSnoid(&snoid);
        }
        if (++column >= 5) {
            column = 0;
            line++;
        }
    }
    drawImage(bookImages, image + 1, dx + 141, dy + 28, 8, 17);
    drawImage(bookImages, 5, 101, 0, 8, 17);
}

/* The book's view update: when due, scrolls the book the way pressed
   (scrollPressed: 1 up a page, 2 up, 3 down, 4 down a page), half a line at a
   time (bookHalfLine), making room at the start when at the top. */
/* @zoombi32 0x00419a59 */
void scrollBook(View *view, short)
{
    short steps;

    if (clockTime() >= view->nextUpdate) {
        view->nextUpdate = clockTime() + view->interval;
        if (view->reset) {
            view->reset = 0;
            view->body.bounds = bookArea;
        } else if (scrollPressed) {
            view->changed = 1;
            steps = 1;
            switch (scrollPressed) {
            case 1:
                steps += 4;
                if (!bookHalfLine && bookRow - steps < 0)
                    steps = 0;
                /* falls through: 5 steps up */
            case 2:
                if (steps && !bookRow)
                    makeBookRoom();
                if (steps)
                    do {
                        if (!bookHalfLine && bookRow > 0) {
                            bookRow--;
                            if (bookRow < 0) {
                                bookRow = 0;
                                steps = 1;
                            }
                            bookHalfLine = 1;
                        } else
                            bookHalfLine = 0;
                    } while (--steps);
                break;
            case 4:
                steps += 4;
                if (!bookHalfLine && bookRow + steps > bookRows - 5)
                    steps = 0;
                /* falls through: 5 steps down */
            case 3:
                if (steps)
                    do {
                        if (!bookHalfLine) {
                            if (bookRow + 1 <= 120 && bookRow < bookRows - 5)
                                bookHalfLine = 1;
                        } else {
                            bookHalfLine = 0;
                            bookRow++;
                            if (bookRow >= bookRows - 5) {
                                bookRow = bookRows - 5;
                                if (bookRow > 120)
                                    bookRow = 120;
                                steps = 1;
                            }
                        }
                    } while (--steps);
                break;
            }
            if (!bookHalfLine)
                scrollPressed = 0;
        }
    }
}

/* Makes room for a row at the start, when one is taken, there are under
   125 rows and the last row is free: moves every entry down a row. */
/* @zoombi32 0x00419f3a */
void makeBookRoom()
{
    short taken = 0;
    short free = 1;
    short i;

    for (i = 0; !taken && i < 5; i++)
        if (bookEntries->entries[i].zoombini)
            taken = 1;
    if (taken && bookRows < 125) {
        for (i = 620; free && i < 625; i++)
            if (bookEntries->entries[i].zoombini)
                free = 0;
        if (free) {
            for (i = 619; i >= 0; i--) {
                bookEntries->entries[i + 5] = bookEntries->entries[i];
                bookEntries->entries[i].zoombini = 0;
            }
            bookHighest += 5;
            countBookEntry(-1);
            bookRow++;
            bookEntries->row = bookRow;
            lightScrollButton(0, 1);
        }
    }
}

/* The index of the last of 625 entries with a value, or 0. */
/* @zoombi32 0x00419f1a */
short lastBookEntry()
{
    for (short i = 0x270; i >= 0; i--)
        if (bookEntries->entries[i].zoombini)
            return i;
    return 0;
}

/* Has view bookView update at once. */
/* @zoombi32 0x0041a225 */
void refreshBook()
{
    View *view = findView(bookView);

    if (view)
        view->nextUpdate = 0;
}

/* Drops the whole rows of empty entries at the start. */
/* @zoombi32 0x0041a024 */
void dropEmptyBookRows()
{
    short searching = 1;
    short empty = -5;
    short i;

    for (i = 0; searching && i < 625; i++)
        if (!bookEntries->entries[i].zoombini)
            empty++;
        else
            searching = 0;
    if (empty >= 5) {
        empty = empty / 5 * 5;
        if (empty) {
            for (i = empty; i < 625; i++) {
                bookEntries->entries[i - empty] = bookEntries->entries[i];
                bookEntries->entries[i].zoombini = 0;
            }
            bookHighest -= empty;
            bookRow -= empty / 5;
            if (bookHighest < 0)
                bookHighest = 0;
            if (bookRow < 0)
                bookRow = 0;
            bookEntries->row = bookRow;
        }
    }
}

/* Lights the scroll button pressed (scrollPressed) if it can scroll that way,
   with a sound when that changes; `quiet` puts it out. */
/* @zoombi32 0x0041a11b */
void lightScrollButton(short quiet, short)
{
    short sound = 0;
    short lit;

    if (scrollPressed >= 0) {
        lit = 0;
        switch (scrollPressed) {
        case 1:
            if (bookRow > 4)
                lit = 1;
            break;
        case 2:
            if (bookRow > 0)
                lit = 1;
            break;
        case 3:
            if (bookRow < bookRows - 5 && bookRow + 1 <= 120)
                lit = 1;
            break;
        case 4:
            if (bookRow < bookRows - 9 && bookRow + 5 <= 120)
                lit = 1;
            break;
        }
        if (lit != camp2HoverButton) {
            camp2HoverButton = lit;
            switch (camp2HoverButton) {
            case 0:
                sound = 2001;
                break;
            case 1:
                sound = 2000;
                break;
            }
        }
        if (quiet) {
            if (camp2HoverButton)
                sound = 2001;
            camp2HoverButton = 0;
        }
        if (sound) {
            if (sound == 2001)
                stopSounds(2000, RESOURCE_TYPE(0, 'S', 'N', 'D'));
            queueViewSound(sound, 0);
        }
    }
}

/* Adds the party's Zoombinis after the last taken entry; if they don't
   fit there, into the free ones (taking the name and features of the
   party's Zoombini with the entry's number, as the original does).
   Returns whether they fitted after the last. */
/* @zoombi32 0x0041a23b */
short addPartyToBook()
{
    short added;
    short count;
    ShortRect empty = camp2EmptyRect;
    short found;
    short last;
    short i;
    short j;

    count = countPresentTravellers();
    added = found = 0;
    for (i = 624; !found && i >= 0; i--)
        if (bookEntries->entries[i].zoombini) {
            found = 1;
            last = i + 1;
        }
    if (!found)
        last = 0;
    if (last + count <= 624) {
        for (i = 0; i < count; i++) {
            bookEntries->entries[last + i].zoombini = party()->travellers[i].zoombini;
            bookEntries->entries[last + i].rect = empty;
            for (j = 0; j < 10; j++)
                bookEntries->entries[last + i].name[j] = party()->travellers[i].name[j];
        }
        added = 1;
    } else
        for (last = 0, i = 0; last < count && i < 625; i++)
            if (!bookEntries->entries[i].zoombini) {
                bookEntries->entries[i].zoombini = party()->travellers[i].zoombini;
                bookEntries->entries[i].rect = empty;
                for (j = 0; j < 10; j++)
                    bookEntries->entries[i].name[j] = party()->travellers[i].name[j];
                last++;
            }
    return added;
}
