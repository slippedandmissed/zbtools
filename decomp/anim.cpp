/*
 * anim (0x41008c-0x411350): 'e2SetupAnim error', 'animation', '%s #%d', 'Cast member out of range %d'
 *
 * Animations: a script of opcodes (see stepAnim) moving up to 32 sprites
 * (cast members, images from resources) over a background. Drawing goes to a
 * port through lists of changed rectangles, which are then copied to the
 * screen.
 */

#include <string.h>
#include "zoombinis.h"
#include "anim.h"
#include "debug.h"
#include "e2memory.h"
#include "events.h"
#include "graphics.h"
#include "jointext.h"
#include "loading.h"
#include "platform.h"
#include "sound.h"

#define SOUND RESOURCE_TYPE(0, 'S', 'N', 'D')
#define MIDI RESOURCE_TYPE('t', 'M', 'I', 'D')

basePort **screenPortRef = &screenPort;
/* The opcodes (the last four are families: 0x10 and 0x14 by the top six
   bits, the others by the top three), and the size of each one's operands. */
unsigned char animOpcodes[13] = {0, 1, 2, 3, 4, 5, 6, 7, 0x10, 0x14, 0x20, 0x60, 0x80};
unsigned char animOperandSizes[13] = {0, 4, 2, 2, 2, 2, 1, 0, 2, 4, 4, 4, 4};

/* Frees an animation: its cast, background and script. */
/* @zoombi32 0x0041008c */
void freeAnim(Anim **anim)
{
    short i;

    if (*anim) {
        for (i = 0; i < (*anim)->castCount; i++)
            freeResource(&(*anim)->cast[i]);
        destroyPort(&(*anim)->background, 1);
        freeScript(&(*anim)->script);
        freeAndClear((void **)anim);
    }
}

/*
 * Runs an animation's script up to its next timed opcode, drawing into its
 * port. Whether it's finished. The opcodes, each followed by its operands:
 *   0          the end (once the last frame's time is up)
 *   1 n t      show a frame; the next n frames last t ticks each
 *   2 t        pause for t ticks
 *   3 s        play sound s when the frame's time is up
 *   4 s        stop sound s
 *   5 s        wait for sound s to finish
 *   6 v        wait for MIDI value v
 *   7          draw
 *   0x10+n v   call callback n with v
 *   0x20+n a o sprite n's flags: and a, or o
 *   0x60+n x y move sprite n
 *   0x80+n i   sprite n shows cast member i (0: none)
 */
/* @zoombi32 0x004100e5 */
short stepAnim(Anim *anim)
{
    short finished;
    unsigned char *operands;
    Sprite *sprite;
    basePort *saved;
    short index;
    short stop;
    short advance;
    unsigned char op;

    finished = stop = 0;
    animDrawing = 1;
    saved = getPort();
    setPort(anim->port);
    do {
        op = *anim->pc;
        operands = anim->pc + 1;
        advance = 0;
        switch (op) {
        case 0:
            if (clockTicks() >= anim->frameTicks + anim->frameTime)
                finished = 1;
            stop = 1;
            break;
        case 1:
            drawAnim(anim);
            if (!anim->frameTicks)
                anim->frameTime = clockTicks();
            if (clockTicks() >= anim->frameTicks + anim->frameTime) {
                if (keepFrameRate)
                    anim->frameTime += anim->frameTicks;
                else
                    anim->frameTime = clockTicks();
                anim->frame++;
                showAnim(anim);
                if (!anim->wait.count) {
                    anim->wait.count = *(unsigned short *)operands;
                    anim->frameTicks = *(unsigned short *)(operands + 2);
                }
                if (!--anim->wait.count)
                    advance = 5;
            }
            stop = 1;
            break;
        case 2:
            if (!anim->wait.until) {
                anim->wait.until = clockTicks() + *(unsigned short *)operands;
                if (anim->frameTime)
                    anim->frameTime += *(unsigned short *)operands;
            } else if (clockTicks() >= anim->wait.until) {
                anim->wait.until = 0;
                advance = 3;
            }
            stop = 1;
            break;
        case 3:
            if (clockTicks() >= anim->frameTicks + anim->frameTime) {
                playSoundOn(*(short *)operands, SOUND, 0);
                advance = 3;
            }
            stop = 1;
            break;
        case 4:
            stopSounds(*(unsigned short *)operands, SOUND);
            advance = 3;
            stop = 1;
            break;
        case 5:
            if (!isSoundPlaying(*(unsigned short *)operands, SOUND))
                advance = 3;
            stop = 1;
            break;
        case 6:
            if (!anim->wait.value)
                anim->wait.value = *(char *)operands;
            if (soundValueReached(anim->wait.value, MIDI)) {
                anim->wait.value = 0;
                advance = 2;
            }
            stop = 1;
            break;
        case 7:
            drawAnim(anim);
            advance = 1;
            stop = 1;
            break;
        default:
            if ((op & ~3) == 0x10) {
                if (anim->callbacks[index = op & 3])
                    anim->callbacks[index](anim, *(short *)operands);
                advance = 3;
            } else {
                sprite = &anim->sprites[index = op & 0x1f];
                switch (op & ~0x1f) {
                case 0x20:
                    sprite->flags.all &= *(unsigned short *)operands;
                    sprite->flags.all |= *(unsigned short *)(operands + 2);
                    advance = 5;
                    break;
                case 0x60:
                    markSprite(anim, index, 1);
                    sprite->x = *(short *)operands;
                    sprite->y = *(short *)(operands + 2);
                    markSprite(anim, index, 0);
                    advance = 5;
                    break;
                case 0x80:
                    markSprite(anim, index, 1);
                    if ((sprite->image = *(short *)operands) != 0)
                        markSprite(anim, index, 0);
                    else
                        resetSprite(sprite);
                    advance = 5;
                    break;
                default:
                    finished = 1;
                    stop = 1;
                }
            }
        }
        anim->pc += advance;
    } while (!stop);
    setPort(saved);
    return finished;
}

/* Runs an animation's script without waiting, up to frame `frames` (or its
   end), then shows the result. Whether it's finished. */
/* @zoombi32 0x00410427 */
short skipAnim(Anim *anim, short frames)
{
    short finished;
    unsigned char *operands;
    Sprite *sprite;
    basePort *saved;
    short which, i;
    short stop;
    short advance;
    unsigned char op;

    if (!anim->drawn)
        for (which = 0; which < 2; which++)
            for (i = 0; i < 32; i++)
                addRect(anim, &anim->rects[which][i], which);
    animDrawing = 0;
    finished = stop = 0;
    saved = getPort();
    setPort(anim->port);
    do {
        op = *anim->pc;
        operands = anim->pc + 1;
        advance = 0;
        switch (op) {
        case 0:
            finished = stop = 1;
            break;
        case 1:
            anim->frame++;
            anim->frameTime += anim->frameTicks;
            if (!anim->wait.count) {
                anim->wait.count = *(unsigned short *)operands;
                anim->frameTicks = *(unsigned short *)(operands + 2);
            }
            if (!--anim->wait.count)
                advance = 5;
            if (anim->frame >= frames)
                stop = 1;
            break;
        case 2:
            advance = 3;
            break;
        case 3:
            advance = 3;
            break;
        case 4:
            advance = 3;
            break;
        case 5:
            advance = 3;
            break;
        case 6:
            advance = 2;
            break;
        case 7:
            advance = 1;
            break;
        default:
            if ((op & ~3) == 0x10)
                advance = 3;
            else {
                sprite = &anim->sprites[which = op & 0x1f];
                switch (op & ~0x1f) {
                case 0x20:
                    sprite->flags.all &= *(unsigned short *)operands;
                    sprite->flags.all |= *(unsigned short *)(operands + 2);
                    advance = 5;
                    break;
                case 0x60:
                    if (*(unsigned short *)operands != sprite->x
                        || *(unsigned short *)(operands + 2) != sprite->y) {
                        markSprite(anim, which, 1);
                        sprite->x = *(short *)operands;
                        sprite->y = *(short *)(operands + 2);
                        markSprite(anim, which, 0);
                    }
                    advance = 5;
                    break;
                case 0x80:
                    if (*(unsigned short *)operands != sprite->image) {
                        markSprite(anim, which, 1);
                        if ((sprite->image = *(short *)operands) != 0)
                            markSprite(anim, which, 0);
                        else
                            resetSprite(sprite);
                    }
                    advance = 5;
                    break;
                default:
                    finished = stop = 1;
                }
            }
        }
        anim->pc += advance;
    } while (!stop);
    anim->drawn = 0;
    showAnim(anim);
    setPort(saved);
    return finished;
}

/* Draws an animation and copies what changed to the screen. */
/* @zoombi32 0x0041069b */
Anim *showAnim(Anim *anim)
{
    short i;

    drawAnim(anim);
    anim->drawn = 0;
    for (i = 0; i < anim->counts[0]; i++)
        copyBits(anim->screen, anim->port, &anim->rects[0][i]);
    anim->counts[0] = 0;
    memset(anim->rects[0], 0, sizeof(anim->rects[0]));
    return anim;
}

/* Draws what changed in an animation (once until shown): erases what's to be
   erased, then draws every sprite clipped to each changed rectangle. */
/* @zoombi32 0x004106f1 */
void drawAnim(Anim *anim)
{
    short region;
    ShortRect clip;
    short which, i;

    if (!anim->drawn) {
        if (animDrawing)
            for (which = 0; which < 2; which++)
                for (i = 0; i < 32; i++)
                    addRect(anim, &anim->rects[which][i], which);
        addRect(anim, &anim->changed, 1);
        for (which = 0; which < anim->counts[1]; which++) {
            if (anim->background)
                copyBits(anim->port, anim->background, &anim->rects[1][which]);
            else
                eraseRect(anim->rects[1][which]);
        }
        region = 0;
        getClipRegion(&region, 1);
        for (which = 0; which < anim->counts[0]; which++) {
            clip = anim->rects[0][which];
            sectRect(&clip, &anim->bounds);
            clipPortToRect(clip);
            for (i = 0; i < 32; i++)
                drawSprite(anim, &anim->sprites[i]);
            setClip(region);
        }
        freeRegion(&region);
        memset(&anim->changed, 0, sizeof(anim->changed));
        memset(anim->rects[1], 0, sizeof(anim->rects[1]));
        anim->counts[1] = 0;
        anim->drawn = 1;
    }
}

/* @zoombi32 0x0041085c */
void drawSprite(Anim *anim, Sprite *sprite)
{
    short handle;

    if (sprite->image) {
        if (sprite->flags.bits.mode == 10)
            sprite->flags.bits.mode = 8;
        handle = usedResourceHandle(anim->cast[sprite->image - 1]);
        drawImageData((unsigned short *)lockHandle(handle), sprite->x + anim->bounds.left,
                  sprite->y + anim->bounds.top, sprite->flags.bits.mode);
        unlockHandle(handle);
    }
}

/* Records a sprite's rectangle as changed: to show (which 0) or to erase
   (1). */
/* @zoombi32 0x004108c7 */
void markSprite(Anim *anim, short index, short which)
{
    Sprite *sprite = &anim->sprites[index];
    ShortRect rect;
    unsigned short *image;

    if (sprite->image) {
        image = (unsigned short *)resourceData(anim->cast[sprite->image - 1]);
        rect.left = sprite->x;
        rect.right = swapShort(image[0]) + rect.left;
        rect.top = sprite->y;
        rect.bottom = swapShort(image[1]) + rect.top;
        offsetRect(&rect, anim->bounds.left, anim->bounds.top);
        if (sectRect(&rect, &anim->bounds)) {
            if (animDrawing) {
                if (which) {
                    if (emptyRect(&anim->rects[1][index]))
                        anim->rects[1][index] = rect;
                } else
                    anim->rects[0][index] = rect;
            } else
                addRect(anim, &rect, which);
        }
    }
}

/* @zoombi32 0x004109df */
void resetSprite(Sprite *sprite)
{
    memset(sprite, 0, sizeof(Sprite));
    sprite->flags.bits.mode = 2;
    sprite->x = sprite->y = 0x8000;
}

/*
 * Adds a rectangle to an animation's list (0 to show, 1 to erase), merging
 * it with any it overlaps; a rectangle to erase is also shown, unless the
 * animation's noOverlap is set.
 */
/* @zoombi32 0x00410a0e */
void addRect(Anim *anim, ShortRect *rect, short which)
{
    ShortRect copy;
    ShortRect overlap;
    short i;

    if (which) {
        addRect(anim, rect, 0);
        if (anim->noOverlap)
            return;
    }
    copy = *rect;
    if (!emptyRect(&copy)) {
        for (i = 0; i < anim->counts[which]; i++) {
            overlap = copy;
            if (sectRect(&overlap, &anim->rects[which][i])) {
                unionRect(&copy, &anim->rects[which][i]);
                anim->counts[which]--;
                if (i < anim->counts[which])
                    memcpy(&anim->rects[which][i], &anim->rects[which][i + 1],
                           (anim->counts[which] - i) * sizeof(ShortRect));
                i = -1;
            }
        }
        if (anim->counts[which] >= 32)
            unionRect(&anim->rects[which][31], &copy);
        else {
            anim->rects[which][anim->counts[which]] = copy;
            anim->counts[which]++;
        }
    }
}

/* Sets up an animation, plays it and frees it. Whether it wasn't
   interrupted. */
/* @zoombi32 0x00410b66 */
short playAnimation(AnimSpec *spec)
{
    short result;

    setupAnim(spec);
    result = playAnim(spec);
    freeAnimSpec(spec);
    return result;
}

/*
 * Loads an animation ("animation #<id + offset>"): its cast (the members its
 * script uses), script, background copy (with saveBackground), sounds and
 * colours.
 */
/* @zoombi32 0x00410b8a */
void setupAnim(AnimSpec *spec)
{
    short count;
    unsigned short first;
    AnimFlags *flags;
    short resource;
    char name[20];
    short size;
    short i;
    Anim *anim;
    short *header;

    if (spec->anim)
        fatalError("e2SetupAnim error");
    formatText(sizeof(name), name, "%s #%d", "animation", spec->id + spec->idOffset);
    flags = &spec->flags;
    resource = spec->idOffset + spec->id;
    loadingAnimation = 1;
    loadShapeListInfo(&castInfo, spec->id, &count, name);
    first = swapShort(*(unsigned short *)resourceData(castInfo));
    freeShapeListInfo(&castInfo);
    loadingAnimation = 0;
    mainLoopEvents();
    size = (char *)&spec->anim->cast[count] - (char *)spec->anim;
    if (!allocateBlock((void **)&spec->anim, size))
        reportJoinedError(name);
    anim = spec->anim;
    memset(anim, 0, size);
    anim->noOverlap = flags->noOverlap;
    anim->castCount = count;
    anim->port = workPort;
    anim->screen = *screenPortRef;
    for (i = 0; i < 32; i++)
        resetSprite(&anim->sprites[i]);
    loadScript(&anim->script, resource, name);
    header = resourceShorts(anim->script);
    anim->pc = (unsigned char *)(header + 4);
    anim->bounds.left = header[1];
    anim->bounds.top = header[0];
    anim->bounds.right = header[3];
    anim->bounds.bottom = header[2];
    if (flags->saveBackground) {
        createPort(&anim->background, &anim->bounds, 1, name);
        copyBits(anim->background, anim->port, &anim->bounds);
    }
    loadCast(anim, first, name);
    loadPalette(&anim->unknown352, spec->id, name, 0);
    freePalette(&anim->unknown352);
    if (spec->colorCount)
        setColors(&g_4aa7e8[spec->firstColor], spec->firstColor, spec->colorCount);
    loadSoundList(&anim->sounds, resource, name);
}

/*
 * Loads the cast members an animation's script uses (all of them with
 * loadWholeCast), from cast resources numbered from `first`.
 *
 * Not exact: the original computes i + 1 with mov/inc rather than lea.
 */
/* @zoombi32 0x00410da6 */
void loadCast(Anim *anim, short first, const char *name)
{
    char used[256];
    unsigned char *pc;
    short i;
    unsigned char op, code;
    short member;

    for (i = 0; i < 256; i++)
        used[i] = loadWholeCast;
    if (!loadWholeCast) {
        pc = anim->pc;
        do {
            op = *pc++;
            i = -1;
            do {
                i++;
                code = op;
                if (i >= 8) {
                    if (i == 8)
                        code &= 0xfc;
                    else if (i == 9)
                        code &= 0xfc;
                    else
                        code &= 0xe0;
                }
            } while (i < 12 && code != animOpcodes[i]);
            if (code == 0x80) {
                member = *(short *)pc;
                if (member) {
                    if (member > anim->castCount)
                        fatalError("Cast member out of range %d", member);
                    else
                        used[member - 1] = 1;
                }
            }
            pc += animOperandSizes[i];
        } while (code);
    }
    for (i = 0; i < anim->castCount; i++) {
        if (used[i])
            loadShapeMember(&anim->cast[i], first, i + 1, name);
        else
            anim->cast[i] = 0;
    }
}

#define INTERRUPTED \
    (interrupted || (spec->flags.interruptible && (interrupted = isEventWaiting(3, 0)) != 0))

/*
 * Plays a set-up animation to its end (or until a click or key, if it's
 * interruptible, then skipping to the end if asked), then handles its sounds
 * and restores its background if asked. Whether it wasn't interrupted.
 */
/* @zoombi32 0x00410ec8 */
short playAnim(AnimSpec *spec)
{
    short count;
    Anim *anim;
    short sounds;
    unsigned short stopped, left, waited; /* each sound id is kept, as in the original */
    short interrupted;
    short i;

    if (!spec->flags.keepMidi)
        stopSounds(0xffff, MIDI);
    interrupted = 0;
    anim = spec->anim;
    while (!INTERRUPTED && !stepAnim(anim))
        mainLoopEvents();
    if (INTERRUPTED && spec->flags.finishOnInterrupt)
        skipAnim(anim, 0x7fff);
    sounds = usedResourceHandle(anim->sounds);
    count = i = *(short *)handleData(sounds); /* (through i, as the original does) */
    for (i = 0; i < count; i++) {
        if (INTERRUPTED)
            stopSounds(stopped = ((unsigned short *)handleData(sounds))[i + 1], SOUND);
        else
            endSoundLoops(left = ((unsigned short *)handleData(sounds))[i + 1], SOUND);
    }
    if (spec->flags.waitForSounds)
        for (i = 0; i < count; i++)
            while (soundPlayingOrStop(waited = ((unsigned short *)handleData(sounds))[i + 1], SOUND, INTERRUPTED))
                mainLoopEvents();
    spritesBounds(anim, &animArea);
    if (spec->flags.restoreBackground && anim->background) {
        copyBits(anim->port, anim->background, &animArea);
        copyBits(anim->screen, anim->port, &animArea);
    }
    discardEvents(3);
    return !INTERRUPTED;
}

/* @zoombi32 0x004110fd */
void freeAnimSpec(AnimSpec *spec)
{
    freeShapeListInfo(&castInfo);
    if (spec->anim) {
        freePalette(&spec->anim->unknown352);
        freeSoundListNow(&spec->anim->sounds);
        freeShapeListInfo(&spec->anim->unknown34e);
        freeAnim(&spec->anim);
    }
}

/* Starts an animation's script again (running its first frame if asked). */
/* @zoombi32 0x00411145 */
void restartAnim(Anim *anim, short run)
{
    anim->pc = (unsigned char *)resourceData(anim->script) + 8;
    anim->frameTicks = 0;
    anim->frame = 0;
    anim->drawn = 0;
    memset(anim->sprites, 0, sizeof(anim->sprites));
    if (run)
        runFrame(anim);
}

/* Runs the main loop until an animation shows its next frame (or ends). */
/* @zoombi32 0x00411194 */
void runFrame(Anim *anim)
{
    unsigned short frame = anim->frame;

    do
        mainLoopEvents();
    while (!stepAnim(anim) && frame == anim->frame);
}

/* @zoombi32 0x004111bc */
void loadScript(long *script, short id, const char *name)
{
    joinText(&scriptText, name, "script");
    loadResourceAs(script, RESOURCE_TYPE(0, 'S', 'C', 'R'), id, scriptText, 1);
    freeText((void **)&scriptText);
}

/* @zoombi32 0x004111f9 */
void freeScript(long *script)
{
    freeText((void **)&scriptText);
    freeResource(script);
}

/* @zoombi32 0x00411212 */
short animAlwaysTrue()
{
    return 1;
}

/* Sets up an animation drawn into the current port and runs its first frame;
   afterwards it shows on the screen. */
/* @zoombi32 0x00411217 */
void setupAnimOffscreen(AnimSpec *spec)
{
    basePort *port = getPort();

    screenPortRef = &port;
    setupAnim(spec);
    runFrame(spec->anim);
    spec->anim->screen = screenPort;
    screenPortRef = &screenPort;
}

/* The rectangle covering an animation's sprites. */
/* @zoombi32 0x00411257 */
void spritesBounds(Anim *anim, ShortRect *into)
{
    ShortRect rect;
    unsigned short *image;
    Sprite *sprite;
    short i;

    memset(into, 0, sizeof(ShortRect));
    for (i = 0; i < 32; i++) {
        sprite = &anim->sprites[i];
        if (sprite->image) {
            image = (unsigned short *)resourceData(anim->cast[sprite->image - 1]);
            rect.left = sprite->x;
            rect.top = sprite->y;
            rect.right = swapShort(image[0]) + rect.left;
            rect.bottom = swapShort(image[1]) + rect.top;
            offsetRect(&rect, anim->bounds.left, anim->bounds.top);
            if (sectRect(&rect, &anim->bounds)) {
                if (emptyRect(into))
                    *into = rect;
                else
                    unionRect(into, &rect);
            }
        }
    }
}
