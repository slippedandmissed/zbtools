/*
 * features (0x465bd0-0x4696f0): 'Feature Group out of range', 'Que overflow
 * id: '
 *
 * The views' ("features'") image banks by group, their drawing, and the
 * running of their scripts.
 */

#include "zoombinis.h"

/* Loads group `group`'s image bank (id `id`), and with `hotspots` its
   images' hotspots (ids `id` and `id` + 1). */
/* @zoombi32 0x00465bd0 */
void loadFeatureGroup(short id, short group, short hotspots)
{
    if (group >= 0 && group < 8) {
        if (!groupBankResources[group]) {
            groupBanks[group] = loadImageBank(id, &groupBankResources[group]);
            if (hotspots) {
                groupHotX[group] = fn_456dbe(id, &groupHotXResources[group]);
                groupHotY[group] = fn_456dbe(id + 1, &groupHotYResources[group]);
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
        fn_46c602(&groupBankResources[i]);
        fn_46c602(&groupHotXResources[i]);
        fn_46c602(&groupHotYResources[i]);
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

    if (sound >= 30000 || (!sound && g_4a7d42 >= 30000)) {
        if (!g_4b87ff)
            return;
        if (!sound)
            sound = g_4a7d42;
        channels = &viewSounds2;
        g_4a7d42 = sound;
    } else {
        if (!g_4b87fe)
            return;
        channels = &viewSounds;
    }
    for (short i = 0; i < 32; i++)
        if (!channels->sounds[i]) {
            channels->active = 1;
            channels->sounds[i] = sound;
            channels->unknown42[i] = streamed;
            channels->state[i] = 0;
            return;
        }
    if (soundTests)
        fn_462749(sound, "Que overflow id: ", 0, 0, 1);
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

    if (g_4b9684)
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
                if (g_4b8b43[view->body.group]) {
                    if (!g_4b8b32[view->body.group])
                        g_4b8b32[view->body.group] = due = view->nextUpdate <= updateTime;
                    else
                        due = 0;
                } else {
                    g_4b8b32[view->body.group] = due = view->nextUpdate <= updateTime;
                }
            } else if (g_4b8b43[view->body.group]) {
                switch (g_4b8b32[view->body.group]) {
                case 1:
                    g_4b8b32[view->body.group] = 2;
                    due = 0;
                    break;
                case 2:
                    g_4b8b32[view->body.group] = 0;
                    due = 1;
                    break;
                }
            } else {
                due = g_4b8b32[view->body.group];
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
        view->unknown2e = 0;
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
                if (view->unknown1e) {
                    dx = view->unknown1e;
                    view->unknown1e = 0;
                    if (dx < 0) {
                        dx = -dx;
                        setViewScript(view, dx, 1);
                        view->flags |= 0x2000000;
                    } else {
                        setViewScript(view, dx, 1);
                    }
                    view->unknown1e = 0;
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
                    g_4b8b43[view->body.group] = 0;
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
        } else if (!view->unknown2e) {
            view->body.frame++;
        } else {
            view->unknown2e = 0;
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
        dx = view->body.x - view->body.unknownAa;
        dy = view->body.y - view->body.unknownAc;
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
                    fn_462749(bank->count, " ixy[].Part > ", &view->id, "Feature id ", 1);
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

    if (view->body.running && !g_4b9684 && view->nextUpdate <= updateTime) {
        ShortRect rect;
        short *hotY;
        short *cels;
        ImageBank *bank;

        view->nextUpdate = updateTime + view->interval;
        if (view->reset) {
            setViewScript(view, view->kind, 1);
            view->unknown2e = 0;
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
    fn_459c84(1, 1);
    switch (currentScene) {
    case 1:
    case 6:
        party()->count = 0;
        fn_41f551();
        return;
    case 4:
        *savedParty() = *party();
        party()->count = 0;
        fn_41f551();
        savedParty()->count = 0;
        return;
    case 5:
        waitingParties()[2] = *party();
        party()->count = 0;
        fn_41f551();
        waitingParties()[2].count = 0;
        return;
    case 3:
        waitingParties()[0] = *party();
        party()->count = 0;
        fn_41f551();
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
        fn_41f551();
        party()->count = 0;
        return;
    case -1:
    case 0:
    case 19:
    case 20:
    case 21:
        g_4afb32 = 0;
        party()->count = 0;
        break;
    }
}

/* @zoombi32 0x00466b93 */
void fn_466b93()
{
    if (g_4b966e) {
        if (lastViewSound == g_4b966e) {
            stopSounds(lastViewSound, RESOURCE_TYPE(0, 'S', 'N', 'D'));
            lastViewSound = 0;
            return;
        }
        queueViewSound(g_4b966e, 0);
    }
}

/* Loads the dialogs' images and scripts (from the sounds' map). */
/* @zoombi32 0x00466bd6 */
void loadDialogs()
{
    long saved;

    if (!dialogResource) {
        g_4b98cc = 0;
        g_4b9686 = g_4b97fc = g_4b9688 = 0;
        dialogView = dialogButton1 = dialogButton2 = g_4b9804 = 0;
        g_4b9806 = g_4b9808 = g_4b980a = g_4b980c = g_4b980e = 0;
        saved = g_4a7f58;
        fn_46be2e(g_4b7b4c);
        dialogImages = loadImageBank(1, &dialogResource);
        for (short i = 0; i < 11; i++)
            dialogScripts[i] = loadSwappedResource(&dialogScriptResources[i], i + 1,
                                                   RESOURCE_TYPE('S', 'C', 'R', 'B'));
        g_4a7f58 = saved;
    }
}

/* @zoombi32 0x00466c95 */
void freeDialogs()
{
    g_4b9686 = g_4b97fc = 0;
    dialogView = dialogButton1 = dialogButton2 = 0;
    g_4b9806 = g_4b9808 = g_4b980a = g_4b980c = g_4b980e = 0;
    if (g_4a7d4c) {
        disposePtr(g_4a7d4c);
        g_4a7d4c = 0;
    }
    if (g_4a7d50) {
        freeSave(&g_4a7d50);
        g_4a7d50 = 0;
    }
    if (dialogResource) {
        fn_46c602(&dialogResource);
        for (short i = 0; i < 11; i++)
            fn_46c602(&dialogScriptResources[i]);
    }
}

/* Asks whether to keep the party, in a scene of the journey. */
/* @zoombi32 0x00466d3d */
void askKeepParty()
{
    if (!g_4b754a && currentScene >= 1 && currentScene <= 18) {
        g_4b9688 = 1;
        showDialog(4, keepPartyText, loseEmText, keepEmText);
    }
}

/* Which of the dialog's hot spots (1-17) a click hits, of those the dialog
   showing (g_4b9684) has; notes it in g_4b97fc. */
/* @zoombi32 0x0046879e */
void dialogClick(Point where)
{
    short hit;
    short i;

    if (g_4b9684 & 0x10) {
        g_4b9686 = 5;
        return;
    }
    for (i = 0; i < 17; i++)
        g_4b98b2[i] = 0;
    dialogWhere = where;
    hit = 0;
    for (i = 0; !hit && i < 17; i++)
        if (ptInRect(&dialogSpots[i], where)) {
            short spot = i + 1;

            if (g_4b9684 & 8)
                hit = spot >= 15 && spot <= 16;
            else if (g_4b9684 & 2)
                hit = (spot >= 11 && spot <= 14) || spot == 17;
            else if (g_4b9684 & 4)
                hit = spot >= 11 && spot <= 14;
            else if (g_4b9684 & 1)
                hit = spot >= 1 && spot <= 10;
            if (hit)
                g_4b97fc = spot;
        }
}

/* @zoombi32 0x00469490 */
void startNewGame()
{
    short scene;

    g_4b80e2 = 0;
    scene = currentScene;
    fn_41f6fc(1);
    fn_41f668();
    currentScene = scene;
    *(short *)(g_4a4ba0 + 0xca) = g_4b0d56 = 3;
    g_4b0d54 = -1;
    g_4b0d50 = -1;
    g_4b0d52 = 1;
    if (currentScene != 1)
        g_4b0d52 = 3;
    *(short *)(g_4a4ba0 + 0xcc) = g_4b0d52;
    viewsLocked = 1;
    g_4b7562 = 1;
    strcpy(gameName, "New Game");
    strcpy(userFile, "ZBUser");
    strcat(userFile, ".txt");
}

/* The New Game button: asks first (warning if the game isn't saved). */
/* @zoombi32 0x00469556 */
void askNewGame()
{
    if (currentScene >= 1 && currentScene <= 18) {
        if (!g_4b754a) {
            if (!g_4b80e2) {
                g_4b80e2 = 1;
                if (g_4afb32)
                    showDialog(4, notSavedNewGameText, newGameText, cancelText);
                else
                    showDialog(4, sureNewGameText, newGameText, cancelText);
            } else {
                startNewGame();
            }
        } else {
            showDialog(4, practiceNoNewText, okText, 0);
        }
    }
}

/* The Load button. */
/* @zoombi32 0x004695e5 */
void askLoadGame()
{
    if (currentScene >= 1 && currentScene <= 18) {
        if (!g_4b754a)
            showDialog(2, 0, 0, 0);
        else
            showDialog(4, practiceNoLoadText, okText, 0);
    }
}

/* The Save button. */
/* @zoombi32 0x00469627 */
void askSaveGame()
{
    if (currentScene >= 1 && currentScene <= 18) {
        if (!g_4b754a)
            showDialog(3, 0, 0, 0);
        else
            showDialog(4, practiceNoSaveText, okText, 0);
    }
}

/* The Quit button. */
/* @zoombi32 0x00469669 */
void askQuit()
{
    if (!g_4b966c && (g_4b9684 & 1))
        g_4b966c = 1;
    if (g_4b966c) {
        showDialog(4, reallyQuitText, yesText, noText);
        return;
    }
    if (!g_4b754a && currentScene >= 1 && currentScene <= 18) {
        if (!g_4b80e0 && !g_4b80e2)
            g_4b80e0 = 1;
    } else {
        g_4b80e0 = -1;
    }
}

/*
 * Shows a dialog: 1 (unused?), 2 load a game, 3 save one, 4 a message with
 * one or two buttons, 5 the credits(?). Not over another dialog or while
 * leaving a scene; each kind's flag goes in g_4b9684 while it shows.
 */
/* @zoombi32 0x00466d7e */
void showDialog(short kind, const char *text, const char *button2, const char *button1)
{
    long saved;
    short flag;
    short y;

    if (!viewsReady || !g_4b2aea || g_4a74dc != -1 || g_4b0d52 > 1 || g_4b0d50 != -1)
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
        g_4b9664 = 0;
        g_4b9666 = 1;
        g_4b9668 = 0;
        flag = 2;
        y = 0x6e;
        break;
    case 3:
        g_4b98c8 = g_4b98ca = 0;
        g_4b98c4 = 0;
        g_4b9664 = 0;
        strcpy(saveName, gameName);
        saveNameLength = strlen(saveName);
        flag = 4;
        y = 0x5e;
        if (g_4a7d50) {
            freeSave(&g_4a7d50);
            g_4a7d50 = 0;
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
    if (g_4b9684 & flag)
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
            g_4a7d3c++;
            showDialog(4, noSavedGamesText, okText, 0);
            kind = 0;
            break;
        }
    case 3:
        {
            short script;

            g_4a7d3c++;
            if (kind == 2)
                script = 4;
            else
                script = 7;
            g_4a7d4c = newPtr(0x646);
            if (!g_4a7d4c)
                fn_41f195("Out of Memory.");
            fn_41f2c8((long)g_4a7d4c, 2);
            g_4b9806 = addView(0x4001000, drawDialogPart, updateDialogPart, script, 0, 0, 0, 0);
            g_4b9808 = addView(0x4000000, drawDialogPart, updateDialogPart, script + 1, 11, 0, 0, 0);
            g_4b980a = addView(0x4000000, drawDialogPart, updateDialogPart, script + 2, 13, 0, 0, 0);
            {
                View *view = findView(g_4b9808);

                if (view)
                    view->placed = placeDialogList;
            }
            {
                View *view = findView(g_4b980a);

                if (view)
                    view->placed = placeDialogList;
            }
        }
        break;
    case 4:
        g_4a7d3c++;
        g_4b980c = addView(0x4001000, drawDialogPart, updateDialogPart, 10, 0, 0, 0, 0);
        g_4b980e = addView(0x4001000, drawDialogPart, updateDialogPart, 11, 15, 0, 0, 0);
        {
            View *view = findView(g_4b980e);

            if (view)
                view->placed = placeDialogList;
        }
        g_4b9688 = 1;
        break;
    case 5:
        {
            long interval;

            g_4a7d3c++;
            saved = g_4a7f58;
            fn_46be2e(g_4b7b4c);
            g_4b9678 = loadImageBank(20, &g_4b9670);
            g_4b967c = loadSwappedResource(&g_4b9674, 20, RESOURCE_TYPE('S', 'C', 'R', 'B'));
            interval = 1;
            g_4b9804 = addView(0x4001000, fn_467227, tickView, 0, interval, 0, 0, 0);
            g_4a7f58 = saved;
            queueViewSound(20104, 0);
        }
        break;
    }
    if (kind)
        g_4b9684 |= flag;
}
