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
