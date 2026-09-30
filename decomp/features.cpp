/*
 * features (0x465bd0-0x4696f0): 'Feature Group out of range', 'Que overflow
 * id: '
 *
 * The views' ("features'") image banks by group, their drawing, and the
 * running of their scripts.
 */

#include "zoombinis.h"
#include "basecamp.h"
#include "debug.h"
#include "e2memory.h"
#include "features.h"
#include "game.h"
#include "graphics.h"
#include "loading.h"
#include "mainloop.h"
#include "picker.h"
#include "roster.h"
#include "snoids.h"
#include "sound.h"
#include "view.h"

/* Loads group `group`'s image bank (id `id`), and with `hotspots` its
   images' hotspots (ids `id` and `id` + 1). */
/* @zoombi32 0x00465bd0 */
void loadFeatureGroup(short id, short group, short hotspots)
{
    if (group >= 0 && group < 8) {
        if (!groupBankResources[group]) {
            groupBanks[group] = loadImageBank(id, &groupBankResources[group]);
            if (hotspots) {
                groupHotX[group] = loadShortTable(id, &groupHotXResources[group]);
                groupHotY[group] = loadShortTable(id + 1, &groupHotYResources[group]);
            }
        } else {
            fatalError("Feature Group already used");
        }
    } else {
        fatalError("Feature Group out of range");
    }
}

/* @zoombi32 0x00465c81 */
void freeFeatureGroups()
{
    for (short i = 0; i < 8; i++) {
        freeResource(&groupBankResources[i]);
        freeResource(&groupHotXResources[i]);
        freeResource(&groupHotYResources[i]);
        groupHotX[i] = 0;
        groupHotY[i] = 0;
        groupBanks[i] = 0;
    }
}

/* A view's drawing: its cels, transparent (clipped to its clip rectangle
   if it has one), unless it has stopped and has flag 0x1000000. */
/* Functional: the original reads each cel's words as it pushes them
   (`drawImageData(image(*cel++), *cel++, *cel++, 8)`, relying on BCC's
   left-to-right evaluation); this reads them first. */
/* @zoombi32-functional 0x00465ce7 */
void drawCels(View *view)
{
    if (view->body.running || !(view->flags & 0x1000000)) {
        if (view->body.clipped) {
            copyRgn(featureClipRgn, currentViewRgn);
            sectRgnWithRect(featureClipRgn, &view->body.clip);
            setClip(featureClipRgn);
        }
        short *cel = (short *)view->body.cels;
        ImageBank *bank = groupBanks[view->body.scriptGroup];

        while (*cel && *cel <= bank->count) {
            unsigned short *image = (unsigned short *)(bank->offsets[*cel++] + (char *)bank);
            short x = *cel++;
            short y = *cel++;

            drawImageData(image, x, y, 8);
        }
        if (view->body.clipped)
            setClip(currentViewRgn);
    }
}

/* drawCels, opaque (functional likewise). */
/* @zoombi32-functional 0x00465da0 */
void drawCelsOpaque(View *view)
{
    if (view->body.running || !(view->flags & 0x1000000)) {
        if (view->body.clipped) {
            copyRgn(featureClipRgn, currentViewRgn);
            sectRgnWithRect(featureClipRgn, &view->body.clip);
            setClip(featureClipRgn);
        }
        short *cel = (short *)view->body.cels;
        ImageBank *bank = groupBanks[view->body.scriptGroup];

        while (*cel && *cel <= bank->count) {
            unsigned short *image = (unsigned short *)(bank->offsets[*cel++] + (char *)bank);
            short x = *cel++;
            short y = *cel++;

            drawImageData(image, x, y, 0);
        }
        if (view->body.clipped)
            setClip(currentViewRgn);
    }
}

/* Asks for a sound in the next update (MIDI, from 30000, in viewSounds2;
   0 there repeats the last MIDI), `streamed` or not. */
/* @zoombi32 0x004666b7 */
void queueViewSound(short sound, char streamed)
{
    SoundChannels *channels;

    if (sound >= 30000 || (!sound && currentDialogSound >= 30000)) {
        if (!musicOn)
            return;
        if (!sound)
            sound = currentDialogSound;
        channels = &viewSounds2;
        currentDialogSound = sound;
    } else {
        if (!soundOn)
            return;
        channels = &viewSounds;
    }
    for (short i = 0; i < 32; i++)
        if (!channels->sounds[i]) {
            channels->active = 1;
            channels->sounds[i] = sound;
            channels->streamed[i] = streamed;
            channels->state[i] = 0;
            return;
        }
    if (soundTests)
        debugMessage(sound, "Que overflow id: ", 0, 0, 1);
}

/* Removes the first of a list of cels. */
/* @zoombi32 0x0046675d */
void removeFirstCel(ViewCel *cels)
{
    short *from = (short *)cels;
    short *to = from;

    from += 3;
    while (*to) {
        *to++ = *from++;
        *to++ = *from++;
        *to++ = *from++;
    }
}

/* An update that just marks a view changed every interval. */
/* @zoombi32 0x004674af */
void tickView(View *view, short)
{
    unsigned long now = resetViewClock();

    if (now >= view->nextUpdate) {
        view->changed = 1;
        view->nextUpdate = now + view->interval;
    }
}

/*
 * The usual view update: when due (with its group, if it has one, in step
 * with the group's leader), runs the view's script a frame: the frame's
 * cels (offset by how far the view has moved with flag 0x800000), the
 * sounds it asks for and the events it tells `notify` of; then places the
 * cels by their hotspots and works out the view's bounds (or region).
 * Flags: 0x40000 goes on to the next script (unknown1e) at the end,
 * 0x100000 stops at the end, 0x2000000 plays frames at random, 0x20000 and
 * 0x10000 run a frame and stop, 0x4000 leaves the old place unredrawn.
 */
/* @zoombi32 0x00465e59 */
void runViewScript(View *view, short region)
{
    short left;
    short *hotX;
    short *hotY;
    ShortRect rect;
    short dx;
    short dy;
    short *script;
    short *at;
    short *cels;
    short ended;
    short draws;
    ImageBank *bank;
    short reset;
    short *cel;
    short word;

    if (dialogFlags)
        return;
    if (!view->body.running) {
        if (view->body.group) {
            groupLeader[view->body.group] = 0;
            view->body.group = 0;
        }
        return;
    }
    {
        short due;

        if (view->body.group) {
            if (!groupLeader[view->body.group])
                groupLeader[view->body.group] = view->id;
            if (groupLeader[view->body.group] == view->id) {
                if (groupFlagsB[view->body.group]) {
                    if (!groupFlagsA[view->body.group])
                        groupFlagsA[view->body.group] = due = view->nextUpdate <= updateTime;
                    else
                        due = 0;
                } else {
                    groupFlagsA[view->body.group] = due = view->nextUpdate <= updateTime;
                }
            } else if (groupFlagsB[view->body.group]) {
                switch (groupFlagsA[view->body.group]) {
                case 1:
                    groupFlagsA[view->body.group] = 2;
                    due = 0;
                    break;
                case 2:
                    groupFlagsA[view->body.group] = 0;
                    due = 1;
                    break;
                }
            } else {
                due = groupFlagsA[view->body.group];
            }
        } else {
            due = view->nextUpdate <= updateTime;
        }
        if (!due)
            return;
    }
    ended = 0;
    reset = view->reset;
    if (reset) {
        setViewScript(view, view->kind, 1);
        view->scriptSet = 0;
        if (view->body.lastFrame < 1 || (view->flags & 0x80000) || (view->flags & 0x20000))
            view->body.running = 0;
        if (view->flags & 0x1000000) {
            view->body.running = 0;
        } else {
            view->changed = 1;
            reset = 0;
        }
        if ((view->flags & 0x8000000) && !view->region)
            if ((view->region = newRgn()) == 0)
                notEnoughNearMemory("feature->Rgn");
    } else {
        if (!(view->flags & 0x4000)) {
            if (view->body.clipped)
                unionRgnRect(region, &view->body.clip);
            else if (view->region)
                unionRgn(region, view->region);
            else
                unionRgnRect(region, &view->body.bounds);
        }
        view->changed = 1;
        if (view->body.frame >= view->body.lastFrame) {
            short more = 1;

            if (view->flags & 0x40000) {
                ended = 1;
                if (view->tag) {
                    dx = view->tag;
                    view->tag = 0;
                    if (dx < 0) {
                        dx = -dx;
                        setViewScript(view, dx, 1);
                        view->flags |= 0x2000000;
                    } else {
                        setViewScript(view, dx, 1);
                    }
                    view->tag = 0;
                    if (view->flags & 0x2000)
                        view->body.running = 0;
                }
                if (view->body.lastFrame < 2)
                    view->body.running = 0;
                view->body.frame = 0;
                view->body.frameOffset = 1;
                more = 0;
            }
            if (view->flags & 0x100000) {
                ended = 1;
                if (view->body.group) {
                    groupLeader[view->body.group] = 0;
                    groupFlagsB[view->body.group] = 0;
                    view->body.group = 0;
                }
                view->body.running = 0;
                if (more) {
                    if (view->notifyEnd && view->notify)
                        view->notify(view, -1);
                    view->notify = 0;
                    return;
                }
            } else {
                view->body.frame = 0;
                view->body.frameOffset = 1;
            }
        } else if (view->flags & 0x2000000) {
            view->body.frame = randomBetween(0, view->body.lastFrame);
            view->body.frameOffset = scriptFrameOffset(scripts[view->body.script], &view->body.frame, 0);
        } else if (!view->scriptSet) {
            view->body.frame++;
        } else {
            view->scriptSet = 0;
        }
        if (view->flags & 0x20000)
            view->body.running = 0;
        if (view->flags & 0x10000) {
            view->flags &= 0xfffeffff;
            view->body.frame = 0;
            view->body.frameOffset = 1;
            view->body.running = 0;
        }
    }
    view->nextUpdate = updateTime + view->interval;
    script = scripts[view->body.script];
    at = script + view->body.frameOffset;
    bank = groupBanks[view->body.scriptGroup];
    if (view->flags & 0x800000) {
        dx = view->body.x - view->body.waypointX;
        dy = view->body.y - view->body.waypointY;
    } else {
        dx = dy = 0;
    }
    cels = cel = (short *)view->body.cels;
    left = 24;
    draws = *at > 0;
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
            *cel++ = *at++ + dx;
            *cel++ = *at++ + dy;
        } else {
            if (word < -0x100) {
                short sound = *at;

                at++;
                if (!reset && sound)
                    queueViewSound(sound, 0);
            }
            if ((word &= 0xff) != 0 && view->notify)
                view->notify(view, --word);
            if (left) {
                if (left == 23) {
                    view->body.frameOffset = at - script;
                    if (ended) {
                        if (view->notifyEnd && view->notify)
                            view->notify(view, -1);
                        view->notify = 0;
                    }
                    return;
                }
                *cel = left = 0;
            }
        }
    } while (left);
    view->body.frameOffset = at - script;
    if (ended) {
        if (view->notifyEnd && view->notify)
            view->notify(view, -1);
        view->notify = 0;
    }
    if (draws) {
        if (view->placed)
            view->placed(view);
        if (groupHotXResources[view->body.scriptGroup]) {
            hotX = groupHotX[view->body.scriptGroup];
            hotY = groupHotY[view->body.scriptGroup];
            for (cel = cels; *cel;) {
                word = *cel++;
                *cel++ -= hotX[word];
                *cel++ -= hotY[word];
            }
        } else if (0) {
            cel = cels;
            do {
                if (*cel > bank->count) {
                    debugMessage(bank->count, " ixy[].Part > ", &view->id, "Feature id ", 1);
                    *cel = 0;
                }
                cel += 3;
            } while (*cel);
        }
        if (view->body.clipped) {
            view->body.bounds = view->body.clip;
            return;
        }
        if (view->flags & 0x8000000) {
            setEmptyRgn(view->region);
            for (cel = cels; *cel;) {
                unsigned short *image = (unsigned short *)(bank->offsets[*cel] + (char *)bank);

                cel++;
                rect.left = *cel++;
                rect.right = swapShort(image[0]) + rect.left;
                rect.top = *cel++;
                rect.bottom = swapShort(image[1]) + rect.top;
                unionRgnRect(view->region, &rect);
            }
            view->body.bounds = ((Region *)handleData(view->region))->bounds;
        } else {
            cel = cels;
            if (*cel) {
                unsigned short *image = (unsigned short *)(bank->offsets[*cel] + (char *)bank);

                cel++;
                view->body.bounds.left = *cel++;
                view->body.bounds.right = swapShort(image[0]) + view->body.bounds.left;
                view->body.bounds.top = *cel++;
                view->body.bounds.bottom = swapShort(image[1]) + view->body.bounds.top;
            }
            while (*cel) {
                unsigned short *image = (unsigned short *)(bank->offsets[*cel] + (char *)bank);

                cel++;
                rect.left = *cel++;
                rect.right = swapShort(image[0]) + rect.left;
                rect.top = *cel++;
                rect.bottom = swapShort(image[1]) + rect.top;
                unionRect(&view->body.bounds, &rect);
            }
        }
    }
}

/* A simpler view update: runs its script's frame as it stands (without
   moving on to the next frame, sounds or events). */
/* Not exact: register allocation (the original keeps the frame's word in
   eax, the position in edx and the count in ecx, and no variable in edi). */
/* @zoombi32 0x00466798 */
void runViewCels(View *view, short region)
{
    short *cel;

    if (view->body.running && !dialogFlags && view->nextUpdate <= updateTime) {
        ShortRect rect;
        short *hotY;
        short *cels;
        ImageBank *bank;

        view->nextUpdate = updateTime + view->interval;
        if (view->reset) {
            setViewScript(view, view->kind, 1);
            view->scriptSet = 0;
            view->changed = 1;
        } else if (!(view->flags & 0x4000)) {
            unionRgnRect(region, &view->body.bounds);
            view->changed = 1;
        }
        if (view->changed) {
            {
                short word;
                short *at;
                short left;

                at = scripts[view->body.script] + view->body.frameOffset;
                bank = groupBanks[view->body.scriptGroup];
                cels = cel = (short *)view->body.cels;
                left = 24;
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
                        *cel++ = *at++;
                        *cel++ = *at++;
                    } else {
                        if (word < -0x100)
                            at++;
                        if (left)
                            *cel = left = 0;
                    }
                } while (left);
            }
            if (view->placed)
                view->placed(view);
            if (groupHotXResources[view->body.scriptGroup]) {
                short *hotX = groupHotX[view->body.scriptGroup];

                hotY = groupHotY[view->body.scriptGroup];
                for (cel = cels; *cel;) {
                    short image = *cel++;

                    *cel++ -= hotX[image];
                    *cel++ -= hotY[image];
                }
            }
            cel = cels;
            if (*cel) {
                unsigned short *image = (unsigned short *)(bank->offsets[*cel] + (char *)bank);

                cel++;
                view->body.bounds.left = *cel++;
                view->body.bounds.right = swapShort(image[0]) + view->body.bounds.left;
                view->body.bounds.top = *cel++;
                view->body.bounds.bottom = swapShort(image[1]) + view->body.bounds.top;
            }
            while (*cel) {
                unsigned short *image = (unsigned short *)(bank->offsets[*cel] + (char *)bank);

                cel++;
                rect.left = *cel++;
                rect.right = swapShort(image[0]) + rect.left;
                rect.top = *cel++;
                rect.bottom = swapShort(image[1]) + rect.top;
                unionRect(&view->body.bounds, &rect);
            }
        }
    }
}

/*
 * The party's journey ends in the current scene: parties stranded in
 * scenes 3, 4 (the camp) and 5 wait there; otherwise it's emptied.
 */
/* @zoombi32 0x00466a25 */
void strandParty()
{
    recordParty(1, 1);
    switch (currentScene) {
    case 1:
    case 6:
        party()->count = 0;
        saveRoster();
        return;
    case 4:
        *savedParty() = *party();
        party()->count = 0;
        saveRoster();
        savedParty()->count = 0;
        return;
    case 5:
        waitingParties()[2] = *party();
        party()->count = 0;
        saveRoster();
        waitingParties()[2].count = 0;
        return;
    case 3:
        waitingParties()[0] = *party();
        party()->count = 0;
        saveRoster();
        waitingParties()[0].count = 0;
        return;
    case 2:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
        saveRoster();
        party()->count = 0;
        return;
    case -1:
    case 0:
    case 19:
    case 20:
    case 21:
        rosterChanged = 0;
        party()->count = 0;
        break;
    }
}

/* @zoombi32 0x00466b93 */
void replayHint()
{
    if (hintSound) {
        if (lastViewSound == hintSound) {
            stopSounds(lastViewSound, RESOURCE_TYPE(0, 'S', 'N', 'D'));
            lastViewSound = 0;
            return;
        }
        queueViewSound(hintSound, 0);
    }
}

/* Loads the dialogs' images and scripts (from the sounds' map). */
/* @zoombi32 0x00466bd6 */
void loadDialogs()
{
    long saved;

    if (!dialogResource) {
        pendingDialogPress = 0;
        dialogClosing = dialogPressed = dialogQuestion = 0;
        dialogView = dialogButton1 = dialogButton2 = creditsView = 0;
        gamesDialogView = gamesDialogPart2 = gamesDialogButtons = confirmDialogView = confirmDialogButtons = 0;
        saved = currentMapFile;
        setCurrentMap(soundsMap);
        dialogImages = loadImageBank(1, &dialogResource);
        for (short i = 0; i < 11; i++)
            dialogScripts[i] = loadSwappedResource(&dialogScriptResources[i], i + 1,
                                                   RESOURCE_TYPE('S', 'C', 'R', 'B'));
        currentMapFile = saved;
    }
}

/* @zoombi32 0x00466c95 */
void freeDialogs()
{
    dialogClosing = dialogPressed = 0;
    dialogView = dialogButton1 = dialogButton2 = 0;
    gamesDialogView = gamesDialogPart2 = gamesDialogButtons = confirmDialogView = confirmDialogButtons = 0;
    if (savedGameList) {
        disposePtr(savedGameList);
        savedGameList = 0;
    }
    if (saveFieldSave) {
        freeSave(&saveFieldSave);
        saveFieldSave = 0;
    }
    if (dialogResource) {
        freeResource(&dialogResource);
        for (short i = 0; i < 11; i++)
            freeResource(&dialogScriptResources[i]);
    }
}

/* Asks whether to keep the party, in a scene of the journey. */
/* @zoombi32 0x00466d3d */
void askKeepParty()
{
    if (!practiceLevel && currentScene >= 1 && currentScene <= 18) {
        dialogQuestion = 1;
        showDialog(4, dialogTexts[textKeepParty], dialogTexts[textLoseEm], dialogTexts[textKeepEm]);
    }
}

/* Which of the dialog's hot spots (1-17) a click hits, of those the dialog
   showing (dialogFlags) has; notes it in dialogPressed. */
/* @zoombi32 0x0046879e */
void dialogClick(Point where)
{
    short hit;
    short i;

    if (dialogFlags & 0x10) {
        dialogClosing = 5;
        return;
    }
    for (i = 0; i < 17; i++)
        buttonPressed[i] = 0;
    dialogWhere = where;
    hit = 0;
    for (i = 0; !hit && i < 17; i++)
        if (ptInRect(&dialogSpots[i], where)) {
            short spot = i + 1;

            if (dialogFlags & 8)
                hit = spot >= 15 && spot <= 16;
            else if (dialogFlags & 2)
                hit = (spot >= 11 && spot <= 14) || spot == 17;
            else if (dialogFlags & 4)
                hit = spot >= 11 && spot <= 14;
            else if (dialogFlags & 1)
                hit = spot >= 1 && spot <= 10;
            if (hit)
                dialogPressed = spot;
        }
}

/* @zoombi32 0x00469490 */
void startNewGame()
{
    short scene;

    newGameAsked = 0;
    scene = currentScene;
    fillRosterHeader(1);
    applyPlayerSettings();
    currentScene = scene;
    *(short *)(gameState + 0xca) = journeyFrom = 3;
    journeyTo = -1;
    pendingScene = -1;
    sceneDue = 1;
    if (currentScene != 1)
        sceneDue = 3;
    *(short *)(gameState + 0xcc) = sceneDue;
    viewsLocked = 1;
    skipJourneyMap = 1;
    strcpy(gameName, "New Game");
    strcpy(userFileName, "ZBUser");
    strcat(userFileName, ".txt");
}

/* The New Game button: asks first (warning if the game isn't saved). */
/* @zoombi32 0x00469556 */
void askNewGame()
{
    if (currentScene >= 1 && currentScene <= 18) {
        if (!practiceLevel) {
            if (!newGameAsked) {
                newGameAsked = 1;
                if (rosterChanged)
                    showDialog(4, dialogTexts[textNotSavedNewGame], dialogTexts[textNewGame], dialogTexts[textCancel]);
                else
                    showDialog(4, dialogTexts[textSureNewGame], dialogTexts[textNewGame], dialogTexts[textCancel]);
            } else {
                startNewGame();
            }
        } else {
            showDialog(4, dialogTexts[textPracticeNoNew], dialogTexts[textOk2], 0);
        }
    }
}

/* The Load button. */
/* @zoombi32 0x004695e5 */
void askLoadGame()
{
    if (currentScene >= 1 && currentScene <= 18) {
        if (!practiceLevel)
            showDialog(2, 0, 0, 0);
        else
            showDialog(4, dialogTexts[textPracticeNoLoad], dialogTexts[textOk2], 0);
    }
}

/* The Save button. */
/* @zoombi32 0x00469627 */
void askSaveGame()
{
    if (currentScene >= 1 && currentScene <= 18) {
        if (!practiceLevel)
            showDialog(3, 0, 0, 0);
        else
            showDialog(4, dialogTexts[textPracticeNoSave], dialogTexts[textOk2], 0);
    }
}

/* The Quit button. */
/* @zoombi32 0x00469669 */
void askQuit()
{
    if (!dialogStage && (dialogFlags & 1))
        dialogStage = 1;
    if (dialogStage) {
        showDialog(4, dialogTexts[textReallyQuit], dialogTexts[textYes], dialogTexts[textNo]);
        return;
    }
    if (!practiceLevel && currentScene >= 1 && currentScene <= 18) {
        if (!quitRequested && !newGameAsked)
            quitRequested = 1;
    } else {
        quitRequested = -1;
    }
}

/*
 * Shows a dialog: 1 (unused?), 2 load a game, 3 save one, 4 a message with
 * one or two buttons, 5 the credits(?). Not over another dialog or while
 * leaving a scene; each kind's flag goes in dialogFlags while it shows.
 */
/* @zoombi32 0x00466d7e */
void showDialog(short kind, const char *text, const char *button2, const char *button1)
{
    long saved;
    short flag;
    short y;

    if (!viewsReady || !rosterReady || townDialogView != -1 || sceneDue > 1 || pendingScene != -1)
        return;
    viewsPaused = 0;
    setDragCursor(0);
    updateViews();
    dialogText = text;
    dialogButton2Text = button2;
    dialogButton1Text = button1;
    switch (kind) {
    case 1:
        flag = 1;
        break;
    case 2:
        firstGameShown = 0;
        selectedGame = 1;
        lastGameClickTime = 0;
        flag = 2;
        y = 0x6e;
        break;
    case 3:
        askingReplace = tooManyGames = 0;
        lastCaretBlink = 0;
        firstGameShown = 0;
        strcpy(saveName, gameName);
        saveNameLength = strlen(saveName);
        flag = 4;
        y = 0x5e;
        if (saveFieldSave) {
            freeSave(&saveFieldSave);
            saveFieldSave = 0;
        }
        break;
    case 4:
        flag = 8;
        break;
    case 5:
        flag = 0x10;
        break;
    default:
        return;
    }
    if (kind == 2 || kind == 3) {
        dialogFrame.left = 0xc0;
        dialogFrame.right = 0x195;
        dialogFrame.top = y;
        dialogFrame.bottom = y + 0xa0;
    }
    if (!flag)
        return;
    if (dialogFlags & flag)
        return;
    if (lastViewSound != 999) {
        stopSounds(lastViewSound, RESOURCE_TYPE(0, 'S', 'N', 'D'));
        lastViewSound = 0;
    }
    switch (kind) {
    case 1:
        dialogView = addView(0x4001000, drawDialogPart, updateDialogPart, 1, 0, 0, 0, 0);
        moveView(dialogView, 0, -1);
        dialogButton1 = addView(0x4000000, drawDialogPart, updateDialogPart, 2, 1, 0, 0, 0);
        dialogButton2 = addView(0x4000000, drawDialogPart, updateDialogPart, 3, 9, 0, 0, 0);
        {
            View *view = findView(dialogButton1);

            if (view)
                view->placed = placeDialogButton;
        }
        {
            View *view = findView(dialogButton2);

            if (view)
                view->placed = placeDialogButton;
        }
        break;
    case 2:
        if (!savedGames) {
            busyCount++;
            showDialog(4, dialogTexts[textNoSavedGames], dialogTexts[textOk2], 0);
            kind = 0;
            break;
        }
    case 3:
        {
            short script;

            busyCount++;
            if (kind == 2)
                script = 4;
            else
                script = 7;
            savedGameList = (SavedGameList *)newPtr(0x646);
            if (!savedGameList)
                reportRosterError("Out of Memory.");
            readWriteSavedGames(savedGameList, 2);
            gamesDialogView = addView(0x4001000, drawDialogPart, updateDialogPart, script, 0, 0, 0, 0);
            gamesDialogPart2 = addView(0x4000000, drawDialogPart, updateDialogPart, script + 1, 11, 0, 0, 0);
            gamesDialogButtons = addView(0x4000000, drawDialogPart, updateDialogPart, script + 2, 13, 0, 0, 0);
            {
                View *view = findView(gamesDialogPart2);

                if (view)
                    view->placed = placeDialogList;
            }
            {
                View *view = findView(gamesDialogButtons);

                if (view)
                    view->placed = placeDialogList;
            }
        }
        break;
    case 4:
        busyCount++;
        confirmDialogView = addView(0x4001000, drawDialogPart, updateDialogPart, 10, 0, 0, 0, 0);
        confirmDialogButtons = addView(0x4001000, drawDialogPart, updateDialogPart, 11, 15, 0, 0, 0);
        {
            View *view = findView(confirmDialogButtons);

            if (view)
                view->placed = placeDialogList;
        }
        dialogQuestion = 1;
        break;
    case 5:
        {
            long interval;

            busyCount++;
            saved = currentMapFile;
            setCurrentMap(soundsMap);
            creditsImages = loadImageBank(20, &creditsImagesResource);
            creditsBackdrop = loadSwappedResource(&creditsBackdropResource, 20, RESOURCE_TYPE('S', 'C', 'R', 'B'));
            interval = 1;
            creditsView = addView(0x4001000, drawCredits, tickView, 0, interval, 0, 0, 0);
            currentMapFile = saved;
            queueViewSound(20104, 0);
        }
        break;
    }
    if (kind)
        dialogFlags |= flag;
}

/* Closes a dialog (by kind, as showDialog), bringing the one below back
   (the menu, or a save dialog under a message). */
/* @zoombi32 0x004674cf */
void closeDialog(short kind)
{
    short closed = 1;
    unsigned short flag;

    switch (kind) {
    case 1:
        deleteView(dialogView);
        deleteView(dialogButton1);
        deleteView(dialogButton2);
        dialogView = dialogButton1 = dialogButton2 = 0;
        flag = 1;
        break;
    case 2:
        loadCancelAlt = 0;
    case 3:
        if (savedGameList) {
            disposePtr(savedGameList);
            savedGameList = 0;
        }
        if (kind == 2)
            flag = 2;
        else
            flag = 4;
        deleteView(gamesDialogView);
        deleteView(gamesDialogPart2);
        deleteView(gamesDialogButtons);
        gamesDialogView = gamesDialogPart2 = gamesDialogButtons = 0;
        if (saveFieldSave) {
            freeSave(&saveFieldSave);
            saveFieldSave = 0;
        }
        if (busyCount)
            busyCount--;
        break;
    case 4:
        flag = 8;
        deleteView(confirmDialogView);
        deleteView(confirmDialogButtons);
        confirmDialogView = confirmDialogButtons = 0;
        if (dialogFlags & 4) {
            View *view = findView(gamesDialogView);

            if (view)
                view->changed = 1;
            view = findView(gamesDialogButtons);
            if (view)
                view->changed = 1;
        }
        if (busyCount)
            busyCount--;
        break;
    case 5:
        flag = 0x10;
        if (lastViewSound) {
            stopSounds(lastViewSound, RESOURCE_TYPE(0, 'S', 'N', 'D'));
            lastViewSound = 0;
        }
        deleteView(creditsView);
        creditsView = 0;
        freeResource(&creditsImagesResource);
        freeResource(&creditsBackdropResource);
        unionRgnRect(removedRgn, &gameRect);
        if (busyCount)
            busyCount--;
        break;
    default:
        dialogClosing &= 0x1f;
        closed = 0;
        break;
    }
    if (closed) {
        dialogFlags &= ~flag;
        if (dialogFlags == 1) {
            View *view = findView(dialogButton1);

            if (view)
                view->changed = 1;
            view = findView(dialogButton2);
            if (view)
                view->changed = 1;
        }
    }
    resetViewClock();
    if (pendingDialogPress) {
        dialogPressed = pendingDialogPress;
        pendingDialogPress = 0;
    }
}

/*
 * The credits view's drawing: first a backdrop of images, then every 15
 * frames scrolls the credits up and adds a line (headings in another
 * colour after a blank; '*' starts over).
 */
/* Functional: the original reads each image's words as it pushes them
   (relying on BCC's left-to-right evaluation), as drawCels does. */
/* @zoombi32-functional 0x00467227 */
void drawCredits(View *view)
{
    Color saved;

    if (creditsShowing && view->changed) {
        switch (view->kind) {
        case 0:
            view->kind++;
            break;
        case 1:
            view->kind++;
            setClipRect(gameRect);
            fillPortRect(gameRect, Color(0x2d), 0);
            {
                for (short *cel = creditsBackdrop + 1; *cel > 0;) {
                    unsigned short *image =
                        (unsigned short *)(creditsImages->offsets[*cel++] + (char *)creditsImages);
                    short x = *cel++;
                    short y = *cel++;

                    drawImageData(image, x, y, 8);
                }
            }
            creditTick = creditLine = 0;
            creditHeading = 1;
            showRect(&gameRect);
            break;
        default: {
            setClipRect(creditsClip);
            copyPortBits(workPort, workPort, creditsScrollTo, creditsScrollFrom, 0);
            creditTick++;
            if (creditTick == 15) {
                const char *line;

                creditTick = 0;
                line = dialogTexts[textCreditLines + creditLine];
                if (*line == '*') {
                    creditLine = 0;
                    line = dialogTexts[textCreditLines + creditLine];
                }
                if (!*line) {
                    creditHeading = 1;
                } else {
                    if (creditHeading) {
                        creditHeading = 0;
                        saved = setForeColor(Color(0x26));
                        drawText(creditsLineRect, 0x22, line, 0xffff);
                    } else {
                        saved = setForeColor(Color(0x23));
                        drawText(creditsLineRect, 0x22, line, 0xffff);
                    }
                    setForeColor(saved);
                }
                creditLine++;
            }
            showRect(&creditsClip);
            break;
        }
        }
    }
}

/*
 * A dialog part's update: lays out its script's first frame (from the
 * dialog scripts, by its kind) like runViewCels, and the first time notes
 * where its cels are as dialog hot spots, from spot `interval`.
 */
/* Not exact: register allocation (the original keeps the image bank in a
   frame slot and the count in ecx; BCC here gives the bank edi). */
/* @zoombi32 0x00468033 */
void updateDialogPart(View *view, short region)
{
    ShortRect rect;
    short *cels;
    ImageBank *bank;
    short *cel;
    short *at;
    short word;
    short left;

    if (view->body.running) {
        if (view->reset)
            view->changed = 1;
        else if (view->placed)
            view->placed(view);
        if (view->changed) {
            unionRgnRect(region, &view->body.bounds);
            at = dialogScripts[view->kind - 1] + 1;
            bank = dialogImages;
            cels = cel = (short *)view->body.cels;
            left = 24;
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
                    *cel++ = *at++;
                    *cel++ = *at++;
                } else {
                    if (word < -0x100)
                        at++;
                    if (left)
                        *cel = left = 0;
                }
            } while (left);
            if (view->placed)
                view->placed(view);
            cel = cels;
            if (*cel) {
                unsigned short *image = (unsigned short *)(bank->offsets[*cel] + (char *)bank);

                cel++;
                view->body.bounds.left = *cel++;
                view->body.bounds.right = swapShort(image[0]) + view->body.bounds.left;
                view->body.bounds.top = *cel++;
                view->body.bounds.bottom = swapShort(image[1]) + view->body.bounds.top;
            }
            while (*cel) {
                unsigned short *image = (unsigned short *)(bank->offsets[*cel] + (char *)bank);

                cel++;
                rect.left = *cel++;
                rect.right = swapShort(image[0]) + rect.left;
                rect.top = *cel++;
                rect.bottom = swapShort(image[1]) + rect.top;
                unionRect(&view->body.bounds, &rect);
            }
            if (view->reset) {
                view->reset = 0;
                {
                    unsigned long spot = view->interval;

                    if (spot) {
                        spot--;
                        for (cel = cels; *cel; spot++) {
                            unsigned short *image =
                                (unsigned short *)(bank->offsets[*cel] + (char *)bank);

                            cel++;
                            dialogSpots[spot].left = *cel++;
                            dialogSpots[spot].right = swapShort(image[0]) + dialogSpots[spot].left;
                            dialogSpots[spot].top = *cel++;
                            dialogSpots[spot].bottom = swapShort(image[1]) + dialogSpots[spot].top;
                        }
                    }
                }
            }
        }
    }
}

/*
 * The menu's buttons (dialog 1): shows the toggles (sound, music, ... ) and
 * pressed buttons, and a moment after a press does what it asks.
 */
/* @zoombi32 0x004688a5 */
void placeDialogButton(View *view)
{
    short *cel = (short *)view->body.cels;
    short first = view->id == dialogButton1;
    short pressedFirst = first && dialogPressed >= 1 && dialogPressed <= 8;
    short pressedSecond = !first && (dialogPressed == 9 || dialogPressed == 10);

    if (view->changed) {
        if (first) {
            short i;
            short button;

            for (i = 0, button = 0; i <= 21; i += 3, button++) {
                short off = 0;

                switch (i) {
                case 12:
                    if (!soundOn)
                        off = 1;
                    break;
                case 15:
                    if (!musicOn)
                        off = 1;
                    break;
                case 18:
                    if (!clickToDragOption)
                        off = 1;
                    break;
                case 21:
                    if (transitionsOn)
                        off = 1;
                    break;
                }
                if (off) {
                    cel[i]++;
                    cel[i + 24]++;
                }
                if (buttonPressed[button])
                    cel[i] = -1;
                else
                    cel[i + 24] = -1;
            }
        } else {
            if (buttonPressed[8])
                cel[0] = -1;
            else
                cel[6] = -1;
            if (buttonPressed[9]) {
                creditsShowing = 0;
                cel[3] = -1;
            } else {
                creditsShowing = 1;
                cel[9] = -1;
            }
        }
    } else if (pressedFirst || pressedSecond) {
        if (dialogPressed != 17) {
            queueViewSound(999, 0);
            waitForEventFor(0, 2, 0, 1);
        }
        view->tag = dialogPressed;
        buttonPressed[dialogPressed - 1] = 1;
        view->changed = 1;
        dialogPressed = -1;
        view->nextUpdate = clockTime() + 2;
    } else if (view->tag && view->nextUpdate) {
        if (clockTime() > view->nextUpdate || clockTime() < view->nextUpdate - 2) {
            buttonPressed[view->tag - 1] = 0;
            view->nextUpdate = 0;
            view->changed = 1;
            switch (view->tag) {
            case 1:
                askNewGame();
                break;
            case 2:
                askLoadGame();
                break;
            case 3:
                askSaveGame();
                break;
            case 4:
                askQuit();
                break;
            case 5:
                soundOn = !soundOn;
                break;
            case 6:
                musicOn = !musicOn;
                if (musicOn)
                    queueViewSound(0, 0);
                else
                    stopSounds(currentDialogSound, RESOURCE_TYPE(0, 'S', 'N', 'D'));
                break;
            case 7:
                clickToDragOption = !clickToDragOption;
                break;
            case 8:
                transitionsOn = !transitionsOn;
                break;
            case 9:
                if (!dialogClosing)
                    dialogClosing = 1;
                break;
            case 10:
                showDialog(5, 0, 0, 0);
                break;
            }
            dialogPressed = view->tag = 0;
        }
    }
    while (*cel) {
        if (*cel == -1)
            removeFirstCel((ViewCel *)cel);
        else
            cel += 3;
    }
}

/*
 * A key while a dialog shows: typing a saved game's name (letters, space,
 * backspace, left and right), else the shortcuts: ^N ^L ^S ^Q (new, load,
 * save, quit), ^D ^B ^J ^T (the toggles), Enter, Escape, up and down.
 */
/* @zoombi32 0x004682f9 */
void dialogKey(unsigned short key)
{
    short done = 0;

    if (dialogPressed)
        return;
    if (dialogFlags & 0x10) {
        dialogClosing = 5;
        return;
    }
    if (!done && !(dialogFlags & 8)
        && ((key >= 0x20 && key <= 0x7a) || key == 8 || key == 0x124 || key == 0x126)) {
        short length;

        if (askingReplace) {
            askingReplace = 0;
            selectedGame = 0;
            {
                View *view = findView(gamesDialogView);

                if (view)
                    view->changed = 1;
                view = findView(gamesDialogButtons);
                if (view)
                    view->changed = 1;
            }
        }
        length = strlen(saveName);
        if (saveNameLength > length)
            saveNameLength = length;
        switch (key) {
        case 8:
            if (length && saveNameLength) {
                for (short i = saveNameLength - 1; i < 21; i++)
                    saveName[i] = saveName[i + 1];
                saveNameLength--;
                length--;
                saveName[length] = 0;
            } else {
                saveName[0] = 0;
                length = saveNameLength = 0;
            }
            break;
        case 0x124:
            if (saveNameLength)
                saveNameLength--;
            break;
        case 0x126:
            if (saveNameLength < length)
                saveNameLength++;
            break;
        default:
            if (length + 4 < 22) {
                for (short i = 21; i > saveNameLength; i--)
                    saveName[i] = saveName[i - 1];
                saveName[saveNameLength] = key;
                saveNameLength++;
                saveName[length + 1] = 0;
                length++;
            }
            break;
        }
        while (length && saveName[0] == ' ') {
            for (short i = 0; i < length; i++)
                saveName[i] = saveName[i + 1];
            saveName[length] = 0;
        }
        length = strlen(saveName);
        if (saveNameLength > length)
            saveNameLength = length;
        return;
    }
    switch (key) {
    case 17:
        if (dialogFlags == 1) {
            dialogPressed = 4;
        } else {
            if (dialogFlags & 8) {
                dialogClosing = 4;
                updateViews();
            }
            if (dialogFlags & 2) {
                dialogClosing = 2;
                updateViews();
            }
            if (dialogFlags & 4) {
                dialogClosing = 3;
                updateViews();
                thirdButtonPresses++;
                if (thirdButtonPresses > 1)
                    practiceLevel = 1;
            }
            gameKey(key);
        }
        break;
    case 0x125:
        if ((dialogFlags & 6) && !(dialogFlags & 8))
            dialogPressed = 11;
        break;
    case 0x127:
        if ((dialogFlags & 6) && !(dialogFlags & 8))
            dialogPressed = 12;
        break;
    case 14:
        if (dialogFlags == 1)
            dialogPressed = 1;
        break;
    case 12:
        if (dialogFlags == 1)
            dialogPressed = 2;
        break;
    case 19:
        if (dialogFlags == 1)
            dialogPressed = 3;
        break;
    case 4:
        if (dialogFlags == 1)
            dialogPressed = 5;
        break;
    case 2:
        if (dialogFlags == 1)
            dialogPressed = 6;
        break;
    case 10:
        if (dialogFlags == 1)
            dialogPressed = 7;
        break;
    case 20:
        if (dialogFlags == 1)
            dialogPressed = 8;
        break;
    case 27:
        if (dialogFlags & 8) {
            if (dialogButton1Text)
                dialogPressed = 16;
        } else if (dialogFlags & 6) {
            dialogPressed = 14;
        }
        break;
    case 13:
        if (dialogFlags & 8)
            dialogPressed = 15;
        else if (dialogFlags & 6)
            dialogPressed = 13;
        else if (dialogFlags & 1)
            dialogPressed = 9;
        break;
    }
}

/*
 * A dialog part's drawing: its cels, and then what the dialog showing puts
 * on it: a message's text and buttons; the save dialog's title, games and
 * (blinking every few ticks) the name being typed; the load dialog's list
 * (the selected game outlined); or the menu's titles, toggles and items.
 */
/* Functional: the cels are drawn as in drawCels (the original reads each
   cel's words as it pushes them). */
/* @zoombi32-functional 0x00467745 */
void drawDialogPart(View *view)
{
    Color saved;
    ShortRect rect;
    short caret;

    if (!view->body.running)
        return;
    {
        short *cel = (short *)view->body.cels;
        ImageBank *bank = dialogImages;

        while (*cel && *cel <= bank->count) {
            unsigned short *image = (unsigned short *)(bank->offsets[*cel++] + (char *)bank);
            short x = *cel++;
            short y = *cel++;

            drawImageData(image, x, y, 8);
        }
    }
    if (dialogFlags & 8) {
        if (!view->changed)
            return;
        if (dialogText) {
            setFont(fonts[2]);
            drawOutlinedText(0xe, 0x2d, messageTitleRect, 0x22, dialogText);
            setFont(fonts[1]);
        }
        if (dialogButton2Text)
            drawText(dialogButton2Rect, 0x22, dialogButton2Text, 0xffff);
        if (dialogButton1Text)
            drawText(dialogButton1Rect, 0x22, dialogButton1Text, 0xffff);
    } else if (dialogFlags & 4) {
        if (view->changed) {
            if (view->id == gamesDialogButtons) {
                drawText(dialogOkRect, 0x22, dialogTexts[textSave], 0xffff);
                drawText(dialogCancelRect, 0x22, dialogTexts[textCancel], 0xffff);
            } else if (view->id == gamesDialogView) {
                saved = setForeColor(Color(0xd));
                setFont(fonts[2]);
                drawOutlinedText(0xe, 0x2d, saveTitleRect, 0x22, dialogTexts[textSaveAGame]);
                drawText(saveAsRect, 1, dialogTexts[textSaveGameAs], 0xffff);
                setForeColor(Color(0xd));
                rect.left = 0xc0;
                rect.right = 0x195;
                rect.top = 0x5e;
                {
                    short shown = 0;

                    for (short game = firstGameShown; game < savedGames && shown < 8; game++, shown++) {
                        rect.bottom = rect.top + 20;
                        drawText(rect, 1, savedGameList->games[game].name, 0xffff);
                        rect.top += 20;
                    }
                }
                setFont(fonts[1]);
                setForeColor(saved);
            }
        }
        if (view->id != gamesDialogView)
            return;
        if (!saveFieldSave)
            saveRect(&saveFieldSave, &saveField, 1, 0);
        if (!view->nextUpdate) {
            view->nextUpdate = 1;
            return;
        }
        if (!saveFieldSave)
            return;
        if (!view->changed && clockTime() - lastCaretBlink <= 4)
            return;
        lastCaretBlink = clockTime();
        caretBlink++;
        caretBlink &= 3;
        unionRgnRect(currentViewRgn, &saveField);
        setClip(currentViewRgn);
        copyPortBits(workPort, saveFieldSave->port, saveField, saveField, 0);
        strlen(saveName);
        setFont(fonts[2]);
        if (caretBlink & 2)
            caret = textWidth(saveName, saveNameLength) + saveField.left;
        drawText(saveField, 1, saveName, 0xffff);
        setFont(fonts[1]);
        if (caretBlink & 2) {
            moveTo(caret, saveField.top);
            lineTo(caret, saveField.bottom);
        }
        copyPortBits(screenPort, workPort, saveField, saveField, 0);
    } else if (dialogFlags & 2) {
        if (!view->changed)
            return;
        saved = setForeColor(Color(0x2d));
        if (view->id == gamesDialogView) {
            setFont(fonts[2]);
            drawOutlinedText(0xe, 0x2d, loadTitleRect, 0x22, dialogTexts[textLoadAGame]);
            rect.left = 0xc0;
            rect.right = 0x195;
            rect.top = 0x6e;
            {
                short game = firstGameShown;

                for (short shown = 0; game < savedGames && shown < 8; shown++) {
                    rect.bottom = rect.top + 20;
                    if (game == selectedGame - 1) {
                        drawOutlinedText(0x2d, 0x22, rect, 1, savedGameList->games[game].name);
                        setForeColor(saved);
                    } else {
                        drawText(rect, 1, savedGameList->games[game].name, 0xffff);
                    }
                    rect.top += 20;
                    game++;
                }
            }
            setFont(fonts[1]);
        } else if (view->id == gamesDialogButtons) {
            drawText(dialogOkRect, 0x22, dialogTexts[textLoad], 0xffff);
            drawText(dialogCancelRect, 0x22, dialogTexts[loadCancelAlt ? text40 : textCancel], 0xffff);
        }
        setForeColor(saved);
    } else if ((dialogFlags & 1) && view->changed) {
        if (view->id == dialogButton1) {
            saved = setForeColor(Color(0x2d));
            setFont(fonts[2]);
            drawOutlinedText(0xe, 0x2d, optionsTitleRect, 0x22, dialogTexts[textOptions]);
            drawText(togglesRect, 1, dialogTexts[textToggles], 0xffff);
            drawText(onRect, 1, dialogTexts[textOn], 0xffff);
            drawText(offRect, 1, dialogTexts[textOff], 0xffff);
            setFont(fonts[1]);
            for (short i = 0; i < 8; i++) {
                menuItemRect.top = menuItemTops[i];
                menuItemRect.bottom = menuItemRect.top + 18;
                drawText(menuItemRect, 1, dialogTexts[textMenuItems + i], 0xffff);
            }
            setForeColor(saved);
        } else if (view->id == dialogButton2) {
            setFont(fonts[2]);
            for (short spot = 9; spot < 11; spot++) {
                rect = dialogSpots[spot - 1];
                if (!buttonPressed[spot - 1]) {
                    rect.top--;
                    rect.bottom--;
                } else {
                    rect.top++;
                    rect.bottom++;
                }
                if (spot == 9)
                    drawText(rect, 0x22, dialogTexts[textOk], 0xffff);
                else
                    drawText(rect, 0x22, dialogTexts[textCredits], 0xffff);
            }
            setFont(fonts[1]);
        }
    }
}

/*
 * The load, save and message dialogs' buttons and list: shows them pressed,
 * and a moment after a press does what they ask: scroll the list, load or
 * save the game (asking before replacing one), pick a game (twice quickly
 * loads it), or close the dialog.
 */
/* @zoombi32 0x00468bde */
void placeDialogList(View *view)
{
    short first;
    short found;
    short length;
    short otherLength;
    unsigned long now;
    char file[32];
    short i;
    short hit = 0;
    short *cel = (short *)view->body.cels;

    if (view->id == gamesDialogPart2) {
        first = 11;
        if (dialogPressed >= 11 && dialogPressed <= 12)
            hit = 1;
    } else if (view->id == gamesDialogButtons) {
        first = 13;
        if ((dialogPressed >= 13 && dialogPressed <= 14) || dialogPressed == 17)
            hit = 1;
    } else if (view->id == confirmDialogButtons) {
        if (!dialogButton1Text) {
            cel[3] = -1;
            cel[9] = -1;
            cel[1] -= 90;
            cel[7] -= 90;
        }
        first = 15;
        if (dialogPressed >= 15 && dialogPressed <= 16)
            hit = 1;
    } else {
        return;
    }
    if (view->changed) {
        i = 0;
        for (short spot = first; spot <= first + 1; spot++) {
            if (buttonPressed[spot - 1])
                cel[i] = -1;
            else
                cel[i + 6] = -1;
            i += 3;
        }
    } else if (hit) {
        if ((dialogPressed != 13 || !askingReplace) && dialogPressed != 17) {
            queueViewSound(999, 0);
            waitForEventFor(0, 2, 0, 1);
        }
        view->tag = dialogPressed;
        buttonPressed[dialogPressed - 1] = 1;
        view->changed = 1;
        dialogPressed = -1;
        view->nextUpdate = clockTime() + 2;
    } else if (view->tag && view->nextUpdate) {
        if (clockTime() > view->nextUpdate || clockTime() < view->nextUpdate - 2) {
            buttonPressed[view->tag - 1] = 0;
            view->nextUpdate = 0;
            view->changed = 1;
            switch (view->tag) {
            case 11:
                if (firstGameShown > 0) {
                    firstGameShown -= 8;
                    if (firstGameShown < 0)
                        firstGameShown = 0;
                    {
                        View *other = findView(gamesDialogView);

                        if (other)
                            other->changed = 1;
                        other = findView(gamesDialogButtons);
                        if (other)
                            other->changed = 1;
                    }
                }
                break;
            case 12:
                if (firstGameShown < savedGames - 8) {
                    firstGameShown += 8;
                    if (firstGameShown > savedGames - 8)
                        firstGameShown = savedGames - 8;
                    if (firstGameShown < 0)
                        firstGameShown = 0;
                    {
                        View *other = findView(gamesDialogView);

                        if (other)
                            other->changed = 1;
                        other = findView(gamesDialogButtons);
                        if (other)
                            other->changed = 1;
                    }
                }
                break;
            case 13:
                if (!dialogClosing) {
                    if (dialogFlags & 2) {
                        if (selectedGame > 0) {
                            strcpy(gameName, savedGameList->games[selectedGame - 1].name);
                            strcpy(userFileName, savedGameList->games[selectedGame - 1].file);
                            strcat(userFileName, ".txt");
                            fillRosterHeader(1);
                            readRoster();
                            viewsLocked = 1;
                            rosterChanged = 0;
                            if (!sceneDue)
                                sceneDue = 3;
                            skipJourneyMap = 1;
                            dialogClosing = 2;
                        }
                    } else {
                        found = 0;
                        tooManyGames = 0;
                        length = strlen(saveName);
                        while (length > 0 && saveName[length - 1] == ' ') {
                            saveName[length - 1] = 0;
                            length--;
                        }
                        if (length) {
                            for (i = 0; !found && i < savedGames; i++) {
                                otherLength = strlen(savedGameList->games[i].name);
                                if (length == otherLength
                                    && !strncmp(savedGameList->games[i].name, saveName, length)) {
                                    found = i + 1;
                                    selectedGame = found;
                                    lastGameClickTime = clockTime();
                                    if (!askingReplace) {
                                        askingReplace = 1;
                                        strcpy(confirmText, dialogTexts[textSureReplace]);
                                        strcat(confirmText, saveName);
                                        strcat(confirmText, " \" ?");
                                        showDialog(4, confirmText, dialogTexts[textReplace],
                                                   dialogTexts[textCancel]);
                                    } else {
                                        askingReplace = 0;
                                    }
                                }
                            }
                            if (dialogQuestion == 3) {
                                dialogQuestion = 0;
                                askingReplace = 0;
                            }
                            if (!askingReplace) {
                                i = 0;
                                if (found) {
                                    i = 1;
                                } else if (savedGames >= 50) {
                                    showDialog(4, dialogTexts[textTooManyGames], dialogTexts[textOk2], 0);
                                    tooManyGames = 1;
                                } else {
                                    i = 2;
                                }
                                rosterChanged = i;
                                switch (i) {
                                case 1:
                                    strcpy(userFileName, savedGameList->games[found - 1].file);
                                    strcat(userFileName, ".txt");
                                    strcpy(gameName, savedGameList->games[found - 1].name);
                                    viewsLocked = 0;
                                    strandParty();
                                    break;
                                case 2:
                                    newSaveFileName(saveName, file, &nextSaveId);
                                    strcpy(gameName, saveName);
                                    strcpy(userFileName, file);
                                    strcpy(savedGameList->games[savedGames].name, gameName);
                                    strcpy(savedGameList->games[savedGames].file, userFileName);
                                    savedGames++;
                                    savedGameList->count = savedGames;
                                    savedGameList->nextId = nextSaveId;
                                    strcat(userFileName, ".txt");
                                    viewsLocked = 0;
                                    rosterReady = 0;
                                    strandParty();
                                    rosterReady = 1;
                                    readWriteSavedGames(savedGameList, 3);
                                    break;
                                }
                                if (i) {
                                    dialogClosing = 3;
                                    if (currentScene == 1)
                                        showMapBox();
                                }
                            }
                        }
                    }
                }
                break;
            case 14:
                if (!dialogClosing) {
                    if (dialogFlags & 2)
                        dialogClosing = 2;
                    else
                        dialogClosing = 3;
                }
                break;
            case 17:
                if (!dialogClosing) {
                    now = clockTime();
                    i = dialogWhere.y - dialogFrame.top;
                    if (i)
                        i /= 20;
                    if (i > 19)
                        i = 19;
                    i = i + firstGameShown + 1;
                    if (i == selectedGame && now - lastGameClickTime <= 30 && selectedGame) {
                        dialogClosing = 0x1000;
                        pendingDialogPress = 13;
                    }
                    {
                        View *other = findView(gamesDialogView);

                        if (other)
                            other->changed = 1;
                    }
                    if (i <= savedGames)
                        selectedGame = i;
                    else
                        selectedGame = 0;
                    lastGameClickTime = now;
                }
                break;
            case 15:
                if (!dialogClosing) {
                    dialogClosing = 4;
                    dialogQuestion = 3;
                    if (newGameAsked) {
                        startNewGame();
                    } else {
                        if (quitRequested == 2)
                            quitRequested = 3;
                        else if ((dialogFlags & 4) && !tooManyGames)
                            pendingDialogPress = 13;
                        if (dialogStage == 1)
                            dialogStage++;
                    }
                }
                break;
            case 16:
                if (!dialogClosing) {
                    dialogStage = 0;
                    newGameAsked = 0;
                    dialogClosing = 4;
                    dialogQuestion = 2;
                    if (quitRequested == 2)
                        quitRequested = -1;
                    else
                        askingReplace = 0;
                }
                break;
            }
            dialogPressed = view->tag = 0;
        }
    }
    while (*cel) {
        if (*cel == -1)
            removeFirstCel((ViewCel *)cel);
        else
            cel += 3;
    }
}
