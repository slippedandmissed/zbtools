/*
 * game (0x44b550-0x455530): cheats ('Cheat on ', 'You have entered the psychedelic ZB Zone!'), memory statistics
 */

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zoombinis.h"

/*
 * The game's part of each pass of the main loop (WinMain registers it with
 * fn_415604; mainLoopEvents calls it): runs the current scene's frame
 * function, and every 12 ticks steps through g_4a4976 (setCursorMode; an
 * animated cursor?).
 */
/* @zoombi32 0x00454f61 */
void gameFrame()
{
    if (currentScene != -1 && scenes[currentScene]->frame) {
        basePort *saved = getPort();
        setPort(workPort);
        scenes[currentScene]->frame();
        setPort(saved);
    }
    if (g_4a4974)
        fn_455023(1);
    else
        fn_455023(0);
    if (g_4b80d2 >= 1) {
        unsigned long now = fn_41571f();
        if (now >= g_4b80d4) {
            g_4b80d4 = now + 12;
            if (g_4b2aee >= 12)
                g_4b2aee = 0;
            setCursorMode(g_4a4976[g_4b2aee]);
            g_4b2aee++;
        }
    }
}

/* @zoombi32 0x00455013 */
short noteOutOfMemory(unsigned long, short)
{
    outOfMemory = 1;
    return 0;
}

/*
 * The program: checks it's the only copy running, starts the engine (each
 * step fatal if it fails), checks for enough memory and for sound devices,
 * finds the game data, opens the 640x480, 256-colour display, loads fonts,
 * cursors and QuickTime, then runs the main loop until it's told to quit.
 * Its messages come from a table of named strings (msg...), not literals.
 * The version check swaps the version's bytes with swapShort (the original
 * evidently had Mac-style byte-swapping helpers; see zoombinis.h).
 */
/* @zoombi32 0x004546f8 */
int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR commandLine, int showCommand)
{
    char osBuffer[0x5f50];
    MemoryInfo memory;
    DisplayMode mode;
    HWND window;
    short i;

    appInstance = instance;
    appPreviousInstance = previous;
    appCommandLine = commandLine;
    appShowCommand = showCommand;
    aboveWindows311 = swapShort((WORD)GetVersion()) > 0x30b;
    atexit(fn_454ca4);

    /* The window class is named after the program's file: if a window of
       that class exists, the game is running already. */
    GetModuleFileName(appInstance, moduleFileName, 0x100);
    window = FindWindow(moduleFileName, 0);
    if (window) {
        ShowWindow(window, SW_SHOWMAXIMIZED);
        SetForegroundWindow(window);
        return 0;
    }
    moduleFileName[0] = 0;
    if (previous)
        return 0;
    if (GlobalFindAtom("Zoombini"))
        return 0;
    instanceAtom = GlobalAddAtom("Zoombini");
    if (*appCommandLine == 'd')
        debugMode = 1;

    initDisplayMode(&mode, 0xffff, 0xffff, -1, 0);
    g_4b2aea = 0;
    quickTimeReady = 0;
    appName = "Zoombini";
    g_4aa42a = 1;
    clockInTicks = 1;
    g_4aa428 = 0;
    g_4a4a0c = 1;
    g_4aa7cc = 0;
    fn_415604(gameFrame);
    fn_4153b0(fn_454caa);
    fn_415a11(fn_44695c);
    fn_456a2f(fn_4625b8);
    g_4b2aec = addModifierKeys(0) != 0x800;

    if (osStartup(instance, osBuffer, sizeof osBuffer))
        fatalError(msgInitOs);
    if (fn_493096())
        fatalError(msgInitTimer);
    if (initMemory(0, 0))
        fatalError(msgInitHeap);
    setGrowProc(noteOutOfMemory);
    unsigned long free = availableVirtualMemory();
    if (free < 0x189c40 || aboveWindows311 && free < 0x389c40)
        fatalError(msgNotEnoughMemory);
    if (aboveWindows311) {
        getMemoryInfo(&memory);
        if (memory.totalPhysical < 0x600000)
            fatalError(msgNotEnoughPhysicalMemory);
    }
    if (initFiles(0))
        fatalError(msgInitFileManager);
    if (fn_480642())
        fatalError(msgInitConfiguration);
    if (initResources())
        fatalError(msgInitResourceManager);
    if (initSound())
        fatalError(msgInitSound);
    if (!waveOutGetNumDevs())
        fatalError(msgNoWaveDevices);
    if (!midiOutGetNumDevs())
        fatalError(msgNoMidiDevices);

    fn_415910();
    fn_446962(g_4b29d4, rosterFileName);
    fn_41f2c8(0, 0);
    strcat(userFileName, ".txt");
    fn_446962(moduleFileName, userFileName);
    findGameData();

    mode.width = 640;
    mode.height = 480;
    mode.palettized = 1;
    mode.colors = 256;
    fn_4144d0(&mode, 1);
    if (instanceAtom) {
        GlobalDeleteAtom(instanceAtom);
        instanceAtom = 0;
    }
    setTakeStatic(0);
    realizePalette(getPortPalette(), 1);

    for (i = 0; i < 3; i++)
        fonts[i] = 0;
    fn_46cb10(&fonts[1], "CornerStone", 13, 0);
    fn_46cb10(&fonts[2], "CornerStone", 18, 0);
    setFont(fonts[1]);

    g_4b754a = 0;
    g_4a4ba0 = (char *)newPtr(0xae05);
    if (!g_4a4ba0)
        fn_41f195(msgOutOfMemory);
    fn_41f6fc(1);
    fn_41f668();
    g_4b0d52 = 0;
    g_4b0d56 = -1;
    g_4b0d54 = -1;
    currentScene = -1;
    fn_46310c();
    fn_456c67(1);

    /* Cursors 1-5 ('CURS' resources). */
    for (i = 0; i < 6; i++) {
        cursors[i] = 0;
        g_4b80c4[i] = 0;
        if (i) {
            fn_46c4fe(&cursors[i], RESOURCE_TYPE('C', 'U', 'R', 'S'), i, 0, 1);
            g_4b80c4[i] = fn_46beac(cursors[i]);
            fn_48ea00(g_4b80c4[i]);
        }
    }
    fn_46be2e(0);

    long quickTimeVersion = 0;
    if (QTInitialize(&quickTimeVersion) || quickTimeVersion < 0x2300)
        fatalError(msgRequiresQuickTime);
    if (qtim_0b())
        fatalError(msgRequiresQuickTime);
    else
        quickTimeReady = 1;

    g_4b0d50 = 0;
    while (mainLoopUpdate() && !g_4b80e0)
        mainLoopEvents();
    fn_454c8e();
    return 0;
}

/* Registered with atexit, which needs the C convention. */
/* @zoombi32 0x00454ca4 */
void __cdecl fn_454ca4()
{
    fn_454caa();
}

/* Picks a free spot (0-19) for something at g_4a4846: finds, for each of
   the 20 places in g_4a47ec, the nearest spot no earlier place has taken,
   then picks the first place without one, from either end at random (-1:
   none). */
/* @zoombi32 0x00450540 */
void fn_450540(short *result)
{
    Point where = g_4a4846;
    short i;
    short skip;
    short spot;
    short j;

    spotTaken(&where, 0, 500);
    for (i = 0; i < 20; i++) {
        skip = 0;
        spot = spotNear(&g_4a47ec[i], 500, skip);
        for (j = 0; spot && j < i; j++)
            if (spot == g_4b75ee[j]) {
                skip++;
                spot = spotNear(&g_4a47ec[i], 500, skip);
                j = 0;
            }
        g_4b75ee[i] = spot;
    }
    spot = -1;
    if (randomBetween(1, 100) <= 50) {
        for (i = 0; spot == -1 && i <= 19; i++)
            if (!g_4b75ee[i])
                spot = i;
    } else {
        for (i = 19; spot == -1 && i >= 0; i--)
            if (!g_4b75ee[i])
                spot = i;
    }
    *result = spot;
}

/* @zoombi32 0x0045062d */
void fn_45062d(short script)
{
    View *view = findView(g_4b25b0);

    if (view) {
        view->flags = 0x4008000;
        setViewScript(view, script, 1);
    }
}

/* @zoombi32 0x00450658 */
void fn_450658(short script, short running)
{
    View *view = findView(g_4b25b0);

    if (view) {
        unionRgnRect(removedRgn, &view->body.bounds);
        view->nextUpdate = 0;
        setViewScript(view, script, running);
        view->flags = 0x4188000;
        view->notify = fn_45174e;
    }
}

/* @zoombi32 0x004506a9 */
void fn_4506a9(short script)
{
    View *view = findView(g_4b25b0);

    if (view) {
        unionRgnRect(removedRgn, &view->body.bounds);
        view->nextUpdate = 0;
        setViewScript(view, script, 0);
        view->flags = 0x4188000;
    }
}

/* @zoombi32 0x004506f0 */
void fn_4506f0()
{
    View *view;

    if (g_4b26b4 < 3) {
        view = findView(g_4b25a8);
        if (view) {
            view->flags = 0x4008000;
            setViewScript(view, 11006, 1);
        }
    }
    if (g_4b26b6 < 3) {
        view = findView(g_4b25aa);
        if (view) {
            view->flags = 0x4008000;
            setViewScript(view, 11007, 1);
        }
    }
}

/* @zoombi32 0x0045074d */
void fn_45074d()
{
    View *view;

    view = findView(g_4b25a8);
    if (view) {
        view->flags = 0x5188000;
        setViewScript(view, 11006, 0);
    }
    view = findView(g_4b25aa);
    if (view) {
        view->flags = 0x5188000;
        setViewScript(view, 11007, 0);
    }
}

/* @zoombi32 0x00450796 */
void fn_450796()
{
    View *view = findView(g_4b2588);

    if (view) {
        view->flags = 0x4008000;
        setViewScript(view, 11076, 1);
    }
}

/* @zoombi32 0x004507bb */
void fn_4507bb()
{
    View *view = findView(g_4b2588);

    if (view) {
        view->flags = 0x5188000;
        setViewScript(view, 11076, 0);
    }
}

/* Whether any of the first g_4b2414 entries of g_4b2430 is set (not -1). */
/* @zoombi32 0x0044d102 */
short fn_44d102()
{
    short any = 0;
    short i;

    for (i = 0; i < g_4b2414; i++)
        if (g_4b2430[i] != -1)
            any = 1;
    return any;
}

/* Resets a cell: state 500, no links. */
/* @zoombi32 0x0044d5ad */
void fn_44d5ad(short cell)
{
    short i;

    g_4b1aea[cell].state = 500;
    for (i = 0; i <= 5; i++)
        g_4b1aea[cell].links[i] = -1;
    g_4b2324[cell] = 0;
}

/* Cuts a cell's link in one direction, clearing its bit. */
/* @zoombi32 0x0044dca0 */
void fn_44dca0(short cell, short direction, short bit)
{
    g_4b1aea[cell].links[direction] = -1;
    g_4b2324[cell] |= bit;
    g_4b2324[cell] ^= bit;
}

/* Counts the cells in state 508, and once there are g_4b2414 of them
   (the first time only), plays a sound. */
/* @zoombi32 0x0044e092 */
void fn_44e092()
{
    short count = 0;
    short i;

    for (i = 0; i < 117; i++)
        if (g_4b1aea[i].state == 508)
            count++;
    if (!g_4b2542 && count >= g_4b2414) {
        g_4b2540++;
        queueViewSound(randomBetween(20055, 20063), 0);
    }
}

/* Unlinks a cell from its neighbours (clearing the opposite direction's
   bit on each) and resets it. */
/* @zoombi32 0x0044e314 */
void fn_44e314(short cell)
{
    short i;
    short other;

    g_4b1aea[cell].state = 500;
    for (i = 0; i <= 5; i++) {
        switch (i) {
        case 0:
            other = g_4b1aea[cell].links[i];
            g_4b2324[other] |= 8;
            g_4b2324[other] ^= 8;
            break;
        case 1:
            other = g_4b1aea[cell].links[i];
            g_4b2324[other] |= 0x10;
            g_4b2324[other] ^= 0x10;
            break;
        case 2:
            other = g_4b1aea[cell].links[i];
            g_4b2324[other] |= 0x20;
            g_4b2324[other] ^= 0x20;
            break;
        case 5:
            other = g_4b1aea[cell].links[i];
            g_4b2324[other] |= 4;
            g_4b2324[other] ^= 4;
            break;
        case 4:
            other = g_4b1aea[cell].links[i];
            g_4b2324[other] |= 2;
            g_4b2324[other] ^= 2;
            break;
        case 3:
            other = g_4b1aea[cell].links[i];
            g_4b2324[other] |= 1;
            g_4b2324[other] ^= 1;
            break;
        }
        g_4b1aea[cell].links[i] = -1;
    }
    g_4b2324[cell] = 0;
}

/* @zoombi32 0x00451238 */
void fn_451238(short n)
{
    g_4b26cc[n][0] = 0;
    g_4b26cc[n][1] = 0;
    g_4b26cc[n][2] = 0;
    g_4b26cc[n][3] = 0;
}

/* @zoombi32 0x00451276 */
void fn_451276()
{
    short i;

    for (i = 0; i < 8; i++) {
        g_4b26cc[i][0] = 0;
        g_4b26cc[i][1] = 0;
        g_4b26cc[i][2] = 0;
        g_4b26cc[i][3] = 0;
    }
}

/* Starts a Zoombini view's script (see startSnoidScript), moving it into
   `group`, with `notify` if given. */
/* @zoombi32 0x0045170a */
void fn_45170a(short id, short script, short group, ViewNotify notify, char unknownF8)
{
    View *view = findView(id);

    if (view) {
        startSnoidScript(viewSnoid(view), script, 0, unknownF8);
        view->body.group = group;
        if (notify)
            view->notify = notify;
    }
}

/* Records the features of the Zoombini in view `id` as slot n's. */
/* @zoombi32 0x00450d00 */
void fn_450d00(short id, short n)
{
    View *view = findView(id);

    if (view) {
        Snoid *snoid = viewSnoid(view);
        char *features = snoid->features;

        g_4b26cc[n][0] = features[0];
        g_4b26cc[n][1] = features[1];
        g_4b26cc[n][2] = features[2];
        g_4b26cc[n][3] = features[3];
    }
}

/* Deletes the temporary file (ZBtemp), if one was made. */
/* @zoombi32 0x00454f03 */
void fn_454f03()
{
    fileSpec temp("ZBtemp");

    if (g_4a48e8) {
        g_4a48e8 = 0;
        deleteFile(temp);
    }
}

/* @zoombi32 0x00454c8e */
void fn_454c8e()
{
    g_4a48e6 = 1;
    fatalError(0, 0);
}

/* Empties one of the Zoombini views (0 and 1, or 7 and 8): no features. */
/* @zoombi32 0x004511c1 */
void fn_4511c1(short n)
{
    View *view;

    if (n == 0 || n == 1)
        view = findView(g_4b26a6[n]);
    else if (n == 7 || n == 8)
        view = findView(g_4b26ba[n]);
    if (view) {
        view->changed = 1;
        Snoid *snoid = viewSnoid(view);

        snoid->features[0] = 0;
        snoid->features[1] = 0;
        snoid->features[2] = 0;
        snoid->features[3] = 0;
        snoid->unknownF4 = 4;
    }
}

/* Stops and empties the two Zoombini views g_4b26ac. */
/* @zoombi32 0x004512ac */
void fn_4512ac()
{
    ShortRect unused[1];
    short i;
    View *view;

    for (i = 0; i < 2; i++) {
        view = findView(g_4b26ac[i]);
        if (view) {
            view->body.running = 0;
            view->changed = 1;
            Snoid *snoid = viewSnoid(view);

            snoid->features[0] = 0;
            snoid->features[1] = 0;
            snoid->features[2] = 0;
            snoid->features[3] = 0;
            unionRgnRect(removedRgn, &snoid->body.bounds);
        }
    }
}

/* A view's update: adds buttons 2 (when g_4b2792 changes) and 1 (the
   first time) to the region to redraw. */
/* @zoombi32 0x0044f180 */
void fn_44f180(View *, short region)
{
    if (g_4b2792) {
        if (!g_4a483e) {
            g_4a483e = 1;
            unionRgnRect(region, &g_4a4708[2].rect);
        }
    } else if (g_4a483e) {
        g_4a483e = 0;
        unionRgnRect(region, &g_4a4708[2].rect);
    }
    if (!g_4a4840) {
        g_4a4840 = 1;
        unionRgnRect(region, &g_4a4708[1].rect);
    }
}

/* Closes the scene. */
/* @zoombi32 0x0044f1f2 */
void fn_44f1f2()
{
    if (g_4b2790) {
        g_4b2790 = 0;
        short saved = fn_46bee9(1);

        clearViews();
        unloadSounds();
        fn_46c602(&g_4a47c8);
        fn_46c602(&g_4b2638);
        fn_46c602(&g_4b2650);
        fn_46c602(&g_4b2654);
        fn_46bee9(saved);
        fn_46ca9c(&g_4b278c);
        fadeOutViews();
        fn_4624fc();
    }
}

/* A view's drawing: its cels from the bank g_4b2634, while it runs and
   stands in the game's area. */
/* Functional: the original reads each cel's words as it pushes them
   (`drawImageData(image(*cel++), *cel++, *cel++, 8)`, relying on BCC's
   left-to-right evaluation); this reads them first. */
/* @zoombi32-functional 0x004541bf */
void fn_4541bf(View *view)
{
    if (view->body.running && ptInRect(&gameRect, *(Point *)&view->body.x)) {
        short *cel = (short *)view->body.cels;
        ImageBank *bank = g_4b2634;

        while (*cel && *cel <= bank->count) {
            unsigned short *image = (unsigned short *)(bank->offsets[*cel++] + (char *)bank);
            short x = *cel++;
            short y = *cel++;

            drawImageData(image, x, y, 8);
        }
    }
}

/* The scene to go back to: the current one if it is the town, the camp, a
   waiting place or a puzzle with a party (setting g_4b7562), else 3. */
/* @zoombi32 0x00454c10 */
short fn_454c10()
{
    short scene;

    if (savedScene() == 3 || savedScene() == 4 || savedScene() == 5 || savedScene() == 6 || savedScene() == 1
        || savedScene() >= 7 && savedScene() <= 18 && party()->count > 0) {
        scene = savedScene();
        g_4b7562 = 1;
    } else {
        scene = 3;
    }
    return scene;
}

/* Stands each Zoombini on a cell in state 508 at its cell. */
/* @zoombi32 0x0044e0e2 */
void fn_44e0e2()
{
    Point where;
    short i;
    View *view;

    g_4b2540 = 0;
    for (i = 0; i < 117; i++)
        if (g_4b1aea[i].state == 508) {
            view = findView(g_4b1aea[i].view);
            where.x = view->body.x + 24;
            where.y = view->body.y - 5;
            setSnoidAction((Snoid *)&findView(g_4b1aea[i].snoid)->body, 0, &where);
        }
}

/* Stands the placed Zoombinis (g_4b2544) on their cells (state 507). */
/* @zoombi32 0x0044e161 */
void fn_44e161()
{
    Point where;
    short i;
    short cell;
    View *view;

    for (i = 0; i < g_4b2414; i++)
        if (g_4b2544[i].cell) {
            cell = g_4b2544[i].cell;
            view = findView(g_4b1aea[g_4b2544[i].cell].view);
            where.x = view->body.x + 24;
            where.y = view->body.y - 5;
            view = findView(g_4b2544[i].snoid);
            if (view) {
                setSnoidAction(viewSnoid(view), 0, &where);
                g_4b1aea[cell].state = 507;
                g_4b1aea[cell].snoid = g_4b2544[i].snoid;
            }
        }
}

/* Loads a QuickTime movie from a file (0: failed). */
/* @zoombi32 0x004552fd */
long fn_4552fd(const char *path)
{
    long movie;
    long id;
    long file;
    long error;

    error = qtim_2c(path, &file, 0);
    if (qtim_5e() || error)
        return 0;
    id = 0;
    error = qtim_2a(&movie, file, &id, 0, 0, 0);
    if (qtim_5e() || error)
        return 0;
    error = qtim_02(file);
    if (qtim_5e() || error)
        return 0;
    return movie;
}

/* Stops the movie playing, if one is, and with `shutdown` closes
   QuickTime. */
/* @zoombi32 0x00455273 */
void fn_455273(short shutdown)
{
    if (g_4b2ad4) {
        g_4b2ad6 = 1;
        g_4b2ad4 = 0;
        qtim_31(g_4b2ad8, 0);
        qtim_07(g_4b2ad8);
        setPort(g_4b2ae4);
    }
    if (shutdown && quickTimeReady) {
        quickTimeReady = 0;
        if (g_4b2adc) {
            qtim_37(g_4b2adc);
            g_4b2adc = 0;
        }
        qtim_0c();
        QTTerminate();
    }
}

/* Copies the features of the Zoombini in view `id` to one of the slot
   views (0 and 1, or 7 and 8); for any but slot 0, while g_4b2630 is
   below 3, the Zoombini's view also stops and moves to the slot view's
   place in the list (moveView). */
/* @zoombi32 0x00450c24 */
void fn_450c24(short id, short n)
{
    View *to;
    char *features;
    View *from;

    from = findView(id);
    Snoid *fromSnoid = viewSnoid(from);
    features = fromSnoid->features;
    switch (n) {
    case 0:
        to = findView(g_4b26a6[0]);
        break;
    case 1:
        to = findView(g_4b26a6[1]);
        break;
    case 7:
        to = findView(g_4b26ba[7]);
        break;
    case 8:
        to = findView(g_4b26ba[8]);
        break;
    }
    if (to) {
        to->changed = 1;
        Snoid *snoid = viewSnoid(to);
        char *copy = snoid->features;

        copy[0] = features[0];
        copy[1] = features[1];
        copy[2] = features[2];
        copy[3] = features[3];
        snoid->unknownF4 = 4;
        if (n && g_4b2630 < 3) {
            from->changed = 1;
            from->body.running = 0;
            moveView(from->id, 0, to->id);
        }
    }
}

/* Records the features of the Zoombinis in the views g_4b2776[1-3] as
   slots 1-3's (none if a view is gone). */
/* @zoombi32 0x00450d5d */
void fn_450d5d()
{
    short i;
    View *view;

    for (i = 1; i < 4; i++) {
        view = findView(g_4b2776[i]);
        if (view) {
            Snoid *snoid = viewSnoid(view);

            g_4b26cc[i][0] = snoid->features[0];
            g_4b26cc[i][1] = snoid->features[1];
            g_4b26cc[i][2] = snoid->features[2];
            g_4b26cc[i][3] = snoid->features[3];
        } else {
            g_4b26cc[i][0] = 0;
            g_4b26cc[i][1] = 0;
            g_4b26cc[i][2] = 0;
            g_4b26cc[i][3] = 0;
        }
    }
}

/* The same for slots 4-6. */
/* @zoombi32 0x00450df2 */
void fn_450df2()
{
    short i;
    View *view;

    for (i = 4; i < 7; i++) {
        view = findView(g_4b2776[i]);
        if (view) {
            Snoid *snoid = viewSnoid(view);

            g_4b26cc[i][0] = snoid->features[0];
            g_4b26cc[i][1] = snoid->features[1];
            g_4b26cc[i][2] = snoid->features[2];
            g_4b26cc[i][3] = snoid->features[3];
        } else {
            g_4b26cc[i][0] = 0;
            g_4b26cc[i][1] = 0;
            g_4b26cc[i][2] = 0;
            g_4b26cc[i][3] = 0;
        }
    }
}

/* Copies the features of the Zoombinis in the next two views of
   g_4b2604 (from g_4b2734, up to g_4b262e) into g_4b263c; returns how many
   there were. */
/* @zoombi32 0x00452035 */
short fn_452035()
{
    short count = 0;
    View *view;

    if (g_4b2734 < g_4b262e) {
        count++;
        view = findView(g_4b2604[g_4b2734]);
        if (view) {
            Snoid *snoid = viewSnoid(view);
            char *features = snoid->features;

            g_4b263c[0] = features[0];
            g_4b263c[1] = features[1];
            g_4b263c[2] = features[2];
            g_4b263c[3] = features[3];
        }
        if (g_4b2734 + 1 < g_4b262e) {
            count++;
            view = findView(g_4b2604[g_4b2734 + 1]);
            if (view) {
                Snoid *snoid = viewSnoid(view);
                char *features = snoid->features;

                g_4b263c[4] = features[0];
                g_4b263c[5] = features[1];
                g_4b263c[6] = features[2];
                g_4b263c[7] = features[3];
            }
        } else {
            g_4b263c[4] = 0;
            g_4b263c[5] = 0;
            g_4b263c[6] = 0;
            g_4b263c[7] = 0;
        }
    }
    return count;
}

/* Starts view g_4b25ac's script (11036 on, by g_4b266c) and, with it, the
   Zoombini in view g_4b26b2 (script 12020 on, from g_4a44ac), grouping
   them. */
/* @zoombi32 0x0045162e */
void fn_45162e(short)
{
    View *view;
    View *other;

    view = findView(g_4b25ac);
    if (view) {
        setViewScript(view, g_4b266c + 11036, 1);
        view->notify = fn_45174e;
        moveView(view->id, 0, g_4b25ae);
    }
    other = findView(g_4b26b2);
    if (other && view) {
        Snoid *snoid = viewSnoid(other);

        snoid->unknownF2 = 1;
        *(Point *)&snoid->body.x = g_4a44ac;
        startSnoidScript(viewSnoid(other), g_4b266c + 12020, 0, 0);
        other->notify = fn_45174e;
        moveView(other->id, 1, view->id);
    }
    if (other)
        groupViews(other->id, view->id, 0, 0, 0, 0);
    else
        groupViews(view->id, view->id, 0, 0, 0, 0);
}

/* Whether two of the Zoombinis (g_4b2430) share a feature; g_4b2516 is
   set to the first they share (0-3). */
/* @zoombi32 0x0044cd71 */
short fn_44cd71(short first, short second)
{
    short hair;
    short eyes;
    short nose;
    short feet;
    short hair2;
    short eyes2;
    short nose2;
    short feet2;

    {
        Snoid *snoid = (Snoid *)&findView(partyViews[g_4b2430[first]])->body;

        hair = snoid->features[0];
        eyes = snoid->features[1];
        nose = snoid->features[2];
        feet = snoid->features[3];
    }
    {
        Snoid *snoid = (Snoid *)&findView(partyViews[g_4b2430[second]])->body;

        hair2 = snoid->features[0];
        eyes2 = snoid->features[1];
        nose2 = snoid->features[2];
        feet2 = snoid->features[3];
    }
    if (hair == hair2) {
        g_4b2516 = 0;
        return 1;
    }
    if (eyes == eyes2) {
        g_4b2516 = 1;
        return 1;
    }
    if (nose == nose2) {
        g_4b2516 = 2;
        return 1;
    }
    if (feet2 == feet) {
        g_4b2516 = 3;
        return 1;
    }
    return 0;
}

/* Picks a random Zoombini of the views g_4b2604 and copies its features
   into g_4b263c, and from g_4b2630 3 on, the next one's after it; returns
   how many views there were. */
/* @zoombi32 0x00451f4e */
short fn_451f4e()
{
    short i;
    short count;
    short ids[22];
    View *view;

    for (i = 0, count = 0; i < g_4b262e; i++)
        if (g_4b2604[i])
            ids[count++] = g_4b2604[i];
    if (count) {
        i = randomBetween(0, count - 1);
        view = findView(ids[i]);
        if (view) {
            Snoid *snoid = viewSnoid(view);
            char *features = snoid->features;

            g_4b263c[0] = features[0];
            g_4b263c[1] = features[1];
            g_4b263c[2] = features[2];
            g_4b263c[3] = features[3];
        }
        if (g_4b2630 >= 3 && count > 1) {
            i++;
            if (i == count)
                i = 0;
            view = findView(ids[i]);
            if (view) {
                Snoid *snoid = viewSnoid(view);
                char *features = snoid->features;

                g_4b263c[4] = features[0];
                g_4b263c[5] = features[1];
                g_4b263c[6] = features[2];
                g_4b263c[7] = features[3];
            }
        } else {
            g_4b263c[4] = 0;
            g_4b263c[5] = 0;
            g_4b263c[6] = 0;
            g_4b263c[7] = 0;
        }
    }
    return count;
}

/* The cheat's message: "You have entered the psychedelic ZB Zone!", in a
   box at g_4a447a. */
/* @zoombi32 0x0044dcdc */
void fn_44dcdc()
{
    ShortRect rect = g_4a447a;
    Color saved;
    char text[64];

    sprintf(text, "You have entered the psychedelic ZB Zone!");
    saved = setForeColor(Color(0x19));
    fillPortRect(Rect(rect), Color(0x1f), 0);
    frameRect(Rect(rect));
    drawText(Rect(rect), 0x22, text, 0xffff);
    setForeColor(saved);
    showRect(&rect);
}

/* Draws button 1 (image 5 or 6) or 2 (2 or 3, or 1 or 2 without
   g_4b2792), lit or not, and with `show` shows it. */
/* @zoombi32 0x0044f066 */
void fn_44f066(short which, short lit, short show)
{
    short image = 0;
    short handle;
    ImageBank *bank;

    switch (which) {
    case 1:
        image = 5;
        break;
    case 2:
        image = 2;
        if (!g_4b2792) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        handle = fn_46beac(g_4a47c8);
        lockHandle(handle);
        bank = (ImageBank *)handleData(handle);
        unsigned short *data = (unsigned short *)(swapLong(bank->offsets[image]) + (char *)bank);

        drawImageData(data, g_4a4708[which].rect.left, g_4a4708[which].rect.top, 8);
        unlockHandle(handle);
        if (show)
            showRect(&g_4a4708[which].rect);
    }
}

/* Gives the Zoombinis in the views g_4b26ac the features set in slots 0-3
   and 7-4 (in that order, so later slots win), and returns 2 if the two
   then differ, else 0. */
/* @zoombi32 0x004513ac */
short fn_4513ac()
{
    char *first;
    char *second;
    short i;
    View *view;
    short result;

    view = findView(g_4b26ac[0]);
    if (view) {
        view->body.running = 1;
        Snoid *snoid = (Snoid *)&view->body;

        snoid->unknownF4 = 4;
        first = snoid->features;
        for (i = 0; i < 4; i++) {
            if (g_4b26cc[i][0])
                first[0] = g_4b26cc[i][0];
            if (g_4b26cc[i][1])
                first[1] = g_4b26cc[i][1];
            if (g_4b26cc[i][2])
                first[2] = g_4b26cc[i][2];
            if (g_4b26cc[i][3])
                first[3] = g_4b26cc[i][3];
        }
    }
    view = findView(g_4b26ac[1]);
    if (view) {
        view->body.running = 1;
        Snoid *snoid = (Snoid *)&view->body;

        snoid->unknownF4 = 4;
        second = snoid->features;
        for (i = 7; i > 3; i--) {
            if (g_4b26cc[i][0])
                second[0] = g_4b26cc[i][0];
            if (g_4b26cc[i][1])
                second[1] = g_4b26cc[i][1];
            if (g_4b26cc[i][2])
                second[2] = g_4b26cc[i][2];
            if (g_4b26cc[i][3])
                second[3] = g_4b26cc[i][3];
        }
    }
    result = 0;
    if (second[0] != first[0])
        result = 2;
    else if (second[1] != first[1])
        result = 2;
    else if (second[2] != first[2])
        result = 2;
    else if (second[3] != first[3])
        result = 2;
    return result;
}

/* Starts the scene's next move: view g_4b25a4 or g_4b25a6's script, and
   the pair of views g_4b258c and g_4b258e (scripts from g_4b2714), grouped. */
/* @zoombi32 0x004514f6 */
void fn_4514f6()
{
    View *view;
    View *first;
    View *second;

    g_4b2788 = 0;
    if (!g_4b2742)
        view = findView(g_4b25a4);
    else
        view = findView(g_4b25a6);
    if (view) {
        view->flags = 0x5188000;
        if (g_4b2630 == 1 || g_4b2630 == 2) {
            setViewScript(view, g_4b2724, 0);
        } else {
            g_4b2742 = !g_4b2742;
            setViewScript(view, g_4b2726, 0);
        }
    }
    first = findView(g_4b258c);
    if (first) {
        setViewScript(first, g_4b2714[g_4b273c], 1);
        loadViewSounds(first->id, 1);
        first->notify = fn_45174e;
    }
    second = findView(g_4b258e);
    if (second) {
        setViewScript(second, g_4b2714[g_4b273c + 1], 1);
        second->notify = fn_45174e;
    }
    view = findView(g_4b26b2);
    if (view && second)
        moveView(view->id, 0, second->id);
    if (first && second)
        groupViews(first->id, second->id, 0, 0, 0, 0);
}

/* Gives the eight Zoombinis in the views g_4b2672 random features at their
   places (g_4a44cc), except that one of the first `count`, at random, gets
   the features set in g_4b263c. */
/* @zoombi32 0x004520ec */
void fn_4520ec(short count)
{
    short j;
    char *features;
    short i;
    Snoid *snoid;
    short chosen;
    View *view;

    if (count) {
        if (count >= 8)
            chosen = randomBetween(0, 7);
        else
            chosen = randomBetween(0, count - 1);
        for (i = 0; i < 8; i++) {
            view = findView(g_4b2672[i]);
            if (view) {
                view->body.running = 1;
                view->changed = 1;
                snoid = (Snoid *)&view->body;
                snoid->unknownF4 = 4;
                *(Point *)&snoid->body.x = g_4a44cc[i];
                features = snoid->features;
                for (j = 0; j < 4; j++)
                    features[j] = randomBetween(1, 4);
                snoid->unknownF1 = 1;
                if (i == chosen) {
                    if (g_4b263c[0])
                        features[0] = g_4b263c[0];
                    else
                        features[0] = randomBetween(1, 4);
                    if (g_4b263c[1])
                        features[1] = g_4b263c[1];
                    else
                        features[1] = randomBetween(1, 4);
                    if (g_4b263c[2])
                        features[2] = g_4b263c[2];
                    else
                        features[2] = randomBetween(1, 4);
                    if (g_4b263c[3])
                        features[3] = g_4b263c[3];
                    else
                        features[3] = randomBetween(1, 4);
                }
            }
        }
    }
}

/* Moves on the one feature (unknownF5) of the Zoombinis in the views
   g_4b2776[1-3] that change it: each takes the next value after its slot's (or
   an earlier slot's, if its is unset), wrapping 5 round to 1, and records
   it in the next slot. */
/* @zoombi32 0x00450e87 */
void fn_450e87()
{
    short i;
    View *view;
    Snoid *snoid;

    for (i = 0; i < 3; i++) {
        view = findView(g_4b2776[i + 1]);
        if (view) {
            snoid = (Snoid *)&view->body;
            snoid->unknownF4 = 4;
            if (snoid->unknownF5) {
                switch (i) {
                case 0:
                    snoid->features[snoid->unknownF5 - 1] = g_4b26cc[0][snoid->unknownF5 - 1] + 1;
                    break;
                case 1:
                    if (g_4b26cc[1][snoid->unknownF5 - 1])
                        snoid->features[snoid->unknownF5 - 1] = g_4b26cc[1][snoid->unknownF5 - 1] + 1;
                    else
                        snoid->features[snoid->unknownF5 - 1] = g_4b26cc[0][snoid->unknownF5 - 1] + 1;
                    break;
                case 2:
                    if (g_4b26cc[2][snoid->unknownF5 - 1])
                        snoid->features[snoid->unknownF5 - 1] = g_4b26cc[2][snoid->unknownF5 - 1] + 1;
                    else if (g_4b26cc[1][snoid->unknownF5 - 1])
                        snoid->features[snoid->unknownF5 - 1] = g_4b26cc[1][snoid->unknownF5 - 1] + 1;
                    else
                        snoid->features[snoid->unknownF5 - 1] = g_4b26cc[0][snoid->unknownF5 - 1] + 1;
                    break;
                }
                if (snoid->features[snoid->unknownF5 - 1] > 5)
                    snoid->features[snoid->unknownF5 - 1] = 1;
                g_4b26cc[i + 1][snoid->unknownF5 - 1] = snoid->features[snoid->unknownF5 - 1];
                view->unknown1e = snoid->features[snoid->unknownF5 - 1];
                snoid->unknownF8 = 0;
            }
        }
    }
}

/* The same from the other side: the views g_4b2776[6-4], from slots 7-5,
   recording in slots 6-4. */
/* @zoombi32 0x00451020 */
void fn_451020()
{
    short i;
    View *view;
    Snoid *snoid;

    for (i = 5; i > 2; i--) {
        view = findView(g_4b2776[i + 1]);
        if (view) {
            snoid = (Snoid *)&view->body;
            snoid->unknownF4 = 4;
            if (snoid->unknownF5) {
                switch (i) {
                case 3:
                    if (g_4b26cc[5][snoid->unknownF5 - 1])
                        snoid->features[snoid->unknownF5 - 1] = g_4b26cc[5][snoid->unknownF5 - 1] + 1;
                    else if (g_4b26cc[6][snoid->unknownF5 - 1])
                        snoid->features[snoid->unknownF5 - 1] = g_4b26cc[6][snoid->unknownF5 - 1] + 1;
                    else
                        snoid->features[snoid->unknownF5 - 1] = g_4b26cc[7][snoid->unknownF5 - 1] + 1;
                    break;
                case 4:
                    if (g_4b26cc[6][snoid->unknownF5 - 1])
                        snoid->features[snoid->unknownF5 - 1] = g_4b26cc[6][snoid->unknownF5 - 1] + 1;
                    else
                        snoid->features[snoid->unknownF5 - 1] = g_4b26cc[7][snoid->unknownF5 - 1] + 1;
                    break;
                case 5:
                    snoid->features[snoid->unknownF5 - 1] = g_4b26cc[7][snoid->unknownF5 - 1] + 1;
                    break;
                }
                if (snoid->features[snoid->unknownF5 - 1] > 5)
                    snoid->features[snoid->unknownF5 - 1] = 1;
                g_4b26cc[i + 1][snoid->unknownF5 - 1] = snoid->features[snoid->unknownF5 - 1];
                view->unknown1e = snoid->features[snoid->unknownF5 - 1];
                snoid->unknownF8 = 0;
            }
        }
    }
}

/* The scene's keys (with g_4b8803, the cheat keys; else only 0x16f): L shows
   the level, 0x172 and 0x173 turn cheating (g_4b2798) on and off, 0x16f
   calls fn_466b93. Returns whether the key was used. */
/* Not exact: the original keeps `key` in esi and `show` in ebx (saving esi
   around the arrays' initial copies); this swaps them. */
/* @zoombi32 0x00450a58 */
short fn_450a58(unsigned short key)
{
    Color saved;
    char digits[52] = "01234";
    char level[52] = "Level x ";
    short show = 0;
    ShortRect rect = g_4a48b2;
    char text[52];

    if (!g_4b8803 && key != 0x16f)
        return 0;
    switch (key) {
    case 'L':
        level[6] = digits[g_4b2630];
        strcpy(text, level);
        show = 1;
        key = 1;
        break;
    case ' ':
        key = 1;
        break;
    case 0x172:
        strcpy(text, " Cheat on ");
        show = 1;
        g_4b2798 = 1;
        key = 1;
        break;
    case 0x173:
        show = 0;
        g_4b2798 = 0;
        key = 1;
        break;
    case 0x16f:
        fn_466b93();
        key = 1;
        break;
    default:
        key = 0;
        break;
    }
    if (show) {
        saved = setForeColor(Color(0xb));
        fillPortRect(Rect(rect), Color(0xe), 0);
        drawText(Rect(rect), 0x22, text, 0xffff);
        showRect(&rect);
        setForeColor(saved);
    }
    return key;
}

/* Looks, two cells from `cell` in `direction`, for a waiting Zoombini
   (g_4b2430) that shares a feature with the one on `cell` (starting from a
   random feature); places it on the far cell (state 507) with the middle
   one showing the feature (state 501, view 510-513), and returns its
   index. -2: no such cells; -1: none shares a feature. (It reuses `cell`
   as the index.) */
/* `direction` is volatile only to leave it on the stack, as the original
   does (BCC would otherwise give it edi until `differ` needs it). */
/* @zoombi32 0x0044d3b8 */
short fn_44d3b8(short cell, volatile short direction)
{
    short feature;
    short differ;
    short next;
    short further;
    short feet;
    short hair;
    short eyes;
    short nose;
    short hair2;
    short feet2;
    short eyes2;
    short nose2;
    short tries;

    feature = randomUpTo(3);
    next = g_4b1aea[cell].links[direction];
    if (next < 0)
        return -2;
    further = g_4b1aea[next].links[direction];
    if (further < 0)
        return -2;
    {
        Snoid *snoid = (Snoid *)&findView(g_4b1aea[cell].snoid)->body;

        hair = snoid->features[0];
        eyes = snoid->features[1];
        nose = snoid->features[2];
        feet = snoid->features[3];
    }
    differ = 1;
    for (cell = 0; cell < g_4b2414; cell++)
        if (g_4b2430[cell] != -1) {
            Snoid *snoid = (Snoid *)&findView(partyViews[g_4b2430[cell]])->body;

            hair2 = snoid->features[0];
            eyes2 = snoid->features[1];
            nose2 = snoid->features[2];
            feet2 = snoid->features[3];
            tries = 4;
            do {
                if (feature == 0 && hair2 == hair)
                    differ = 0;
                if (feature == 1 && eyes == eyes2)
                    differ = 0;
                if (feature == 2 && nose == nose2)
                    differ = 0;
                if (feature == 3 && feet == feet2)
                    differ = 0;
                if (differ) {
                    tries--;
                    feature++;
                    if (feature > 3)
                        feature = 0;
                }
            } while (differ && tries);
            if (!differ) {
                g_4b1aea[further].state = 507;
                g_4b1aea[further].snoid = partyViews[g_4b2430[cell]];
                g_4b1aea[next].state = 501;
                g_4b1aea[next].snoid = feature + 510;
                g_4b2430[cell] = -1;
                return cell;
            }
        }
    return -1;
}

/* The memory statistics line (with g_4a48e4): free, purgeable and the
   least free seen (g_4a48e0), in thousands; with `clear`, unloads the
   sounds and blanks the line instead. */
/* Not exact: the original gives `port` esi and `freeThousands` ebx (shared
   with `available`); this gives them the other way round. */
/* @zoombi32 0x00455023 */
void fn_455023(short clear)
{
    unsigned long freeUnits;
    unsigned long freeThousands;
    unsigned long purgeableUnits;
    unsigned long purgeableThousands;
    unsigned long leastUnits;
    unsigned long leastThousands;
    Color saved;
    Font *font;
    char text[256];
    unsigned long available;
    unsigned long purgeable;
    basePort *port;

    if (clear) {
        unloadSounds();
        port = getPort();
        setPort(workPort);
        text[0] = 0;
        drawText(Rect(g_4a498e), 0x11, text, 0xffff);
        setPort(port);
    } else if (g_4a48e4) {
        available = availableVirtualMemory();
        purgeable = purgeMemory(0, 0);
        if (available < g_4a48e0)
            g_4a48e0 = available;
        freeUnits = available % 1000;
        freeThousands = available / 1000;
        purgeableUnits = purgeable % 1000;
        purgeableThousands = purgeable / 1000;
        leastUnits = g_4a48e0 % 1000;
        leastThousands = g_4a48e0 / 1000;
        port = getPort();
        setPort(workPort);
        font = getFont();
        setFont(fonts[1]);
        sprintf(text, "Free:%d,%03d  Purg:%d,%03d  Min:%d,%03d", (short)freeThousands, (short)freeUnits,
                (short)purgeableThousands, (short)purgeableUnits, (short)leastThousands, (short)leastUnits);
        fillPortRect(Rect(g_4a498e), Color(0xff), 0);
        saved = setForeColor(Color(0));
        drawText(Rect(g_4a498e), 0x11, text, 0xffff);
        setForeColor(saved);
        setFont(font);
        setPort(port);
        showRect(&g_4a498e);
    }
}

/* A view's drawing: buttons 1 and 2, unlit. */
/* @zoombi32 0x0044f163 */
void fn_44f163(View *)
{
    fn_44f066(1, 0, 0);
    fn_44f066(2, 0, 0);
}

/* Lets the movie play (cmgr_09; MCIdle?); once it has stopped (flag 0x40
   clear; mcInfoIsPlaying?), closes it and returns 1. 0: still playing;
   -1: no movie. */
/* @zoombi32 0x00455229 */
short fn_455229()
{
    long flags;

    if (!g_4b2adc)
        return -1;
    cmgr_05(g_4b2adc, &flags);
    cmgr_09(g_4b2adc);
    if (flags & 0x40)
        return 0;
    fn_455273(0);
    return 1;
}

/* Sets the Zoombinis' features from the slots (fn_4513ac; g_4b2740 if the
   two differ) and starts the views g_4b2590 and g_4b2592 (11018, 11019),
   grouped. */
/* @zoombi32 0x00451315 */
void fn_451315()
{
    View *first;
    View *second;

    g_4b273c = fn_4513ac();
    g_4b2740 = 0;
    if (g_4b273c)
        g_4b2740 = 1;
    first = findView(g_4b2590);
    if (first) {
        setViewScript(first, 11018, 1);
        first->notify = fn_45174e;
    }
    second = findView(g_4b2592);
    if (second) {
        setViewScript(second, 11019, 1);
        second->notify = fn_45174e;
    }
    if (first && second)
        groupViews(first->id, second->id, 0, 0, 0, 0);
}

/* Fills the free cells of a set of 20 with waiting Zoombinis, while any
   are left. */
/* Its loop runs to 22, past the end of the cells, and each Zoombini's view
   is read after its entry in g_4b2430 is cleared (partyViews[-1]); both as
   in the original. */
/* @zoombi32 0x0044d5f5 */
void fn_44d5f5()
{
    short cells[20] = {55, 57, 59, 61, 38, 74, 40, 76, 42, 78, 44, 80, 21, 93, 23, 95, 25, 97, 4, 112};
    short i;
    short j;

    for (i = 0; i < 22; i++)
        if (g_4b1aea[cells[i]].state != 507) {
            g_4b1aea[cells[i]].state = 507;
            for (j = 0; j < g_4b2414; j++)
                if (g_4b2430[j] != -1) {
                    g_4b2430[j] = -1;
                    g_4b1aea[cells[i]].snoid = partyViews[g_4b2430[j]];
                    break;
                }
            if (!fn_44d102())
                break;
        }
}

/* Clears cells of a line of three (a cell's `snoid` from 510 is a feature
   marker, below it a Zoombini): all three when both ends hold Zoombinis and
   the middle is in state 501; otherwise, with the middle in state 507, the
   second end if both ends hold Zoombinis or only the second does, the first
   if only the first does. */
/* @zoombi32 0x0044e21a */
void fn_44e21a(short first, short second, short middle)
{
    if (g_4b1aea[first].snoid < 510 && g_4b1aea[second].snoid < 510 && g_4b1aea[middle].state == 501) {
        fn_44e314(first);
        fn_44e314(second);
        fn_44e314(middle);
    } else if (g_4b1aea[first].snoid < 510 && g_4b1aea[second].snoid < 510 && g_4b1aea[middle].state == 507) {
        fn_44e314(second);
    } else if (g_4b1aea[first].snoid < 510 && g_4b1aea[second].snoid >= 510 && g_4b1aea[middle].state == 507) {
        fn_44e314(first);
    } else if (g_4b1aea[first].snoid >= 510 && g_4b1aea[second].snoid < 510 && g_4b1aea[middle].state == 507) {
        fn_44e314(second);
    }
}

/* Plays a QuickTime movie from a file, centred in the window: loads it
   (fn_4552fd), makes or reuses the movie controller (g_4b2adc; qtim_38 is
   NewMovieController?), and starts it. Returns 0, or 1 if it failed. */
/* @zoombi32 0x0045537f */
short fn_45537f(const char *path)
{
    POINT where;
    Point offset;
    RECT bounds;
    short failed;
    short i;

    failed = 1;
    g_4b2ad8 = fn_4552fd(path);
    if (g_4b2ad8) {
        g_4b2ae4 = getPort();
        setPort(screenPort);
        qtim_0f(g_4b2ad8, &bounds);
        OffsetRect(&bounds, -bounds.left, -bounds.top);
        offset.x = ((screenRect.right - screenRect.left) - (gameRect.right - gameRect.left)) / 2;
        offset.y = ((screenRect.bottom - screenRect.top) - (gameRect.bottom - gameRect.top)) / 2;
        OffsetRect(&bounds, offset.x, offset.y);
        if (!g_4b2adc) {
            g_4b2adc = qtim_38(g_4b2ad8, &bounds, 11, mainWindow);
        } else {
            where.x = offset.x;
            where.y = offset.y;
            cmgr_0d(g_4b2adc, g_4b2ad8, mainWindow, where);
        }
        cmgr_0e(g_4b2adc, &bounds, 0, 11);
        cmgr_00(g_4b2adc, mainWindow, 1);
        cmgr_01(g_4b2adc, 0x20, 0);
        qtim_31(g_4b2ad8, 1);
        qtim_62(0, 0);
        qtim_2f(g_4b2ad8, 0, 0x10000);
        for (i = 0; i < 100; i++)
            cmgr_09(g_4b2adc);
        cmgr_01(g_4b2adc, 8, 0x10000);
        failed = 0;
        g_4b2ad6 = 0;
        g_4b2ad4 = 1;
    }
    return failed;
}

/* Recomputes the cells' link bits: a cell in state 502, 504, 505 or 508
   clears its bit for each neighbour in state 501, 506 or 507; one in state
   501, 506 or 507 for each neighbour in state 502, 508 or g_4b2512. Then
   starts script 7000 on the views of the cells in state 501, 506 or 507. */
/* @zoombi32 0x0044ddc9 */
void fn_44ddc9()
{
    short i;
    View *view;
    short direction;
    short state;

    for (i = 0; i < 117; i++)
        switch (g_4b1aea[i].state) {
        case 502:
        case 504:
        case 505:
        case 508:
            for (direction = 0; direction <= 5; direction++) {
                state = g_4b1aea[g_4b1aea[i].links[direction]].state;
                if (state == 501 || state == 506 || state == 507)
                    switch (direction) {
                    case 0:
                        g_4b2324[i] |= 1;
                        g_4b2324[i] ^= 1;
                        break;
                    case 1:
                        g_4b2324[i] |= 2;
                        g_4b2324[i] ^= 2;
                        break;
                    case 2:
                        g_4b2324[i] |= 4;
                        g_4b2324[i] ^= 4;
                        break;
                    case 3:
                        g_4b2324[i] |= 8;
                        g_4b2324[i] ^= 8;
                        break;
                    case 4:
                        g_4b2324[i] |= 0x10;
                        g_4b2324[i] ^= 0x10;
                        break;
                    case 5:
                        g_4b2324[i] |= 0x20;
                        g_4b2324[i] ^= 0x20;
                        break;
                    }
            }
            break;
        case 501:
        case 506:
        case 507:
            for (direction = 0; direction <= 5; direction++) {
                state = g_4b1aea[g_4b1aea[i].links[direction]].state;
                if (state == 502 || state == 508 || state == g_4b2512)
                    switch (direction) {
                    case 0:
                        g_4b2324[i] |= 1;
                        g_4b2324[i] ^= 1;
                        break;
                    case 1:
                        g_4b2324[i] |= 2;
                        g_4b2324[i] ^= 2;
                        break;
                    case 2:
                        g_4b2324[i] |= 4;
                        g_4b2324[i] ^= 4;
                        break;
                    case 3:
                        g_4b2324[i] |= 8;
                        g_4b2324[i] ^= 8;
                        break;
                    case 4:
                        g_4b2324[i] |= 0x10;
                        g_4b2324[i] ^= 0x10;
                        break;
                    case 5:
                        g_4b2324[i] |= 0x20;
                        g_4b2324[i] ^= 0x20;
                        break;
                    }
            }
            break;
        }
    for (i = 0; i < 117; i++)
        if (g_4b1aea[i].state == 501 || g_4b1aea[i].state == 506 || g_4b1aea[i].state == 507) {
            view = findView(g_4b1aea[i].view);
            setViewScript(view, 7000, 1);
            view->placed = fn_4489ce;
        }
    updateViews();
}

/* Turns cells in state 506 back to 501, lists the placed Zoombinis (state
   507) in g_4b2544 (each one's view lands in the next entry, as in the
   original), and for each of a fixed set of cells (or pairs) found in state
   501, resets it and the cells next to it and cuts the neighbours' links
   to them. */
/* @zoombi32 0x0044d127 */
void fn_44d127()
{
    short count = 0;
    short i;

    for (i = 0; i < 117; i++) {
        if (g_4b1aea[i].state == 506)
            g_4b1aea[i].state = 501;
        if (g_4b1aea[i].state == 507) {
            g_4b2544[count].cell = i;
            count++;
            g_4b2544[count].snoid = g_4b1aea[i].snoid;
        }
    }
    if (g_4b1aea[2].state == 501 && g_4b1aea[19].state == 501) {
        fn_44d5ad(2);
        fn_44d5ad(19);
        fn_44d5ad(10);
        fn_44d5ad(11);
        fn_44d5ad(28);
        fn_44dca0(38, 0, 1);
        fn_44dca0(21, 0, 1);
    }
    if (g_4b1aea[91].state == 501 && g_4b1aea[110].state == 501) {
        fn_44d5ad(91);
        fn_44d5ad(110);
        fn_44d5ad(100);
        fn_44d5ad(82);
        fn_44d5ad(101);
        fn_44dca0(74, 2, 4);
        fn_44dca0(93, 2, 4);
    }
    if (g_4b1aea[112].state == 501) {
        fn_44d5ad(112);
        fn_44d5ad(102);
        fn_44d5ad(103);
        fn_44dca0(93, 3, 8);
        fn_44dca0(95, 2, 4);
    }
    if (g_4b1aea[114].state == 501) {
        fn_44d5ad(114);
        fn_44d5ad(104);
        fn_44d5ad(105);
        fn_44dca0(95, 3, 8);
        fn_44dca0(97, 2, 4);
    }
    if (g_4b1aea[4].state == 501) {
        fn_44d5ad(4);
        fn_44d5ad(12);
        fn_44d5ad(13);
        fn_44dca0(21, 5, 0x20);
        fn_44dca0(23, 0, 1);
    }
    if (g_4b1aea[6].state == 501) {
        fn_44d5ad(6);
        fn_44d5ad(14);
        fn_44d5ad(15);
        fn_44dca0(23, 5, 0x20);
        fn_44dca0(25, 0, 1);
    }
    if (g_4b1aea[97].state == 501 && g_4b1aea[80].state == 501) {
        fn_44d5ad(97);
        fn_44d5ad(80);
        fn_44d5ad(88);
        fn_44d5ad(87);
        fn_44d5ad(70);
        fn_44d5ad(105);
        fn_44dca0(78, 3, 8);
        fn_44dca0(61, 3, 8);
        fn_44dca0(114, 5, 0x20);
    }
    if (g_4b1aea[25].state == 501 && g_4b1aea[44].state == 501) {
        fn_44d5ad(25);
        fn_44d5ad(44);
        fn_44d5ad(34);
        fn_44d5ad(15);
        fn_44d5ad(33);
        fn_44d5ad(52);
        fn_44dca0(42, 5, 0x20);
        fn_44dca0(61, 5, 0x20);
        fn_44dca0(6, 3, 8);
    }
}

/*
 * Drags a Zoombini (from `where`), snapping it to the spot it's over: with
 * g_4b2630 below 3, the one spot g_4a4534 (4) unless g_4b2704; otherwise
 * one of the three spots in row g_4b26b4 of g_4a4584 (0-2) or row g_4b26b6
 * of g_4a45cc (3-5). Returns the spot it was over when released (-1:
 * none).
 */
/* @zoombi32 0x00453e8c */
short fn_453e8c(View *view, Point where)
{
    View *dragged;
    Snoid *snoid;
    volatile short dy; /* volatile: on the stack, where the original has it */
    short id;
    Point current;
    ShortRect unused[1];
    ShortRect rect;
    unsigned long savedInterval;
    short dx;
    short spot;
    short i;
    short x;
    short y;

    setViewsLocked(0);
    id = view->id;
    if ((dragged = removeView(id, 0)) == 0)
        return 0;
    dragged->id = -3;
    insertViewAtEnd(dragged);
    savedInterval = dragged->interval;
    dragged->interval = 3;
    snoid = (Snoid *)&dragged->body;
    current = where;
    dx = 0;
    dy = 0;
    if (g_4b2630 == 3 || g_4b2630 == 4)
        fn_4506f0();
    else if (!g_4b26b0)
        fn_450796();
    while (keepDragging()) {
        getCursorPosition(&current);
        spot = -1;
        if (g_4b2630 < 3) {
            if (ptInRect(&g_4a4534, current) && !g_4b2704) {
                spot = 4;
                current.x = g_4a4528.x;
                current.y = g_4a4528.y;
            }
        } else {
            if (g_4b26b4 < 3)
                for (i = 0; i < 3; i++)
                    if (ptInRect(&g_4a4584[g_4b26b4][i], current)) {
                        spot = i;
                        current.x = g_4a4584[g_4b26b4][i].left + 25;
                        current.y = g_4a4584[g_4b26b4][i].top + 31;
                        i = 3;
                    }
            if (spot < 0 && g_4b26b6 < 3)
                for (i = 0; i < 3; i++)
                    if (ptInRect(&g_4a45cc[g_4b26b6][i], current)) {
                        spot = i + 3;
                        current.x = g_4a45cc[g_4b26b6][i].left + 25;
                        current.y = g_4a45cc[g_4b26b6][i].top + 31;
                        i = 3;
                    }
        }
        x = current.x - dx;
        if (x >= gameRect.left && x <= gameRect.right) {
            snoid->body.x = x;
            snoid->home.x = x;
        }
        y = current.y - dy;
        if (y >= gameRect.top && y <= gameRect.bottom) {
            snoid->body.y = y;
            snoid->home.y = y;
        }
        snoid->unknownF4 = 5;
        mainLoopEvents();
        resetViewClock();
    }
    snoid->unknownF4 = 4;
    rect = dragged->body.bounds;
    unionRgnRect(removedRgn, &rect);
    dragged->id = id;
    dragged->interval = savedInterval;
    return spot;
}

/* Marks `cell` with a feature (view 510-513) that the Zoombini `index`
   shares with the one on `neighbour` (if that one is placed, state 507),
   checking the features from a random one on. */
/* @zoombi32 0x0044d974 */
void fn_44d974(short neighbour, short index, short cell)
{
    long other;

    if (g_4b1aea[neighbour].state == 507)
        switch (randomUpTo(3)) {
        case 0:
            other = g_4b1aea[neighbour].snoid;
            if (snoidFeatures[0][index] == snoidFeatures[0][other])
                g_4b1aea[cell].snoid = 510;
            else if (snoidFeatures[1][index] == snoidFeatures[1][other])
                g_4b1aea[cell].snoid = 511;
            else if (snoidFeatures[2][index] == snoidFeatures[2][other])
                g_4b1aea[cell].snoid = 512;
            else if (snoidFeatures[3][index] == snoidFeatures[3][other])
                g_4b1aea[cell].snoid = 513;
            break;
        case 1:
            other = g_4b1aea[neighbour].snoid;
            if (snoidFeatures[1][index] == snoidFeatures[1][other])
                g_4b1aea[cell].snoid = 511;
            else if (snoidFeatures[2][index] == snoidFeatures[2][other])
                g_4b1aea[cell].snoid = 512;
            else if (snoidFeatures[3][index] == snoidFeatures[3][other])
                g_4b1aea[cell].snoid = 513;
            else if (snoidFeatures[0][index] == snoidFeatures[0][other])
                g_4b1aea[cell].snoid = 510;
            break;
        case 2:
            other = g_4b1aea[neighbour].snoid;
            if (snoidFeatures[2][index] == snoidFeatures[2][other])
                g_4b1aea[cell].snoid = 512;
            else if (snoidFeatures[3][index] == snoidFeatures[3][other])
                g_4b1aea[cell].snoid = 513;
            else if (snoidFeatures[0][index] == snoidFeatures[0][other])
                g_4b1aea[cell].snoid = 510;
            else if (snoidFeatures[1][index] == snoidFeatures[1][other])
                g_4b1aea[cell].snoid = 511;
            break;
        case 3:
            other = g_4b1aea[neighbour].snoid;
            if (snoidFeatures[3][index] == snoidFeatures[3][other])
                g_4b1aea[cell].snoid = 513;
            else if (snoidFeatures[0][index] == snoidFeatures[0][other])
                g_4b1aea[cell].snoid = 510;
            else if (snoidFeatures[1][index] == snoidFeatures[1][other])
                g_4b1aea[cell].snoid = 511;
            else if (snoidFeatures[2][index] == snoidFeatures[2][other])
                g_4b1aea[cell].snoid = 512;
            break;
        }
}

/*
 * Lays out a Zoombini's cels in this scene (unless it's in state 2, which
 * it's put in unless it's in 3 or 5): a part for its pose (unknownF1), then
 * its face (layer unknownC2[unknownF1], if all four features are set) and
 * its features on top, placed by the hot spots in g_4b2658/g_4b265c; then
 * its bounds from the images in g_4b2634.
 */
/* @zoombi32 0x00454374 */
void fn_454374(Snoid *snoid)
{
    short face;
    short dy;
    ShortRect rect;
    Point where;
    short *cel;
    short base;
    short part;
    ImageBank *bank;

    snoid->body.bounds.left = 0;
    snoid->body.bounds.top = 0;
    snoid->body.bounds.right = 0;
    snoid->body.bounds.bottom = 0;
    cel = (short *)snoid->body.cels;
    switch (snoid->unknownF4) {
    case 2:
        return;
    case 3:
    case 5:
        break;
    default:
        snoid->unknownF4 = 2;
        break;
    }
    if (!snoid->features[3] || !snoid->features[2] || !snoid->features[1] || !snoid->features[0])
        face = 0;
    else
        face = snoid->unknownC2[snoid->unknownF1];
    base = snoid->unknownC2[snoid->unknownF1];
    where = *(Point *)&snoid->body.x;
    dy = 0;
    switch (snoid->unknownF1) {
    case 0:
        part = 0x41;
        break;
    case 1:
        part = 0x40;
        break;
    case 2:
        part = 0x42;
        break;
    case 6:
        part = 0x41;
        break;
    case 7:
        part = 0x45;
        break;
    case 8:
        part = 0x43;
        break;
    default:
        part = 0;
        break;
    }
    if (part) {
        *cel++ = part;
        *cel++ = where.x - g_4b2658[part];
        *cel++ = where.y - g_4b265c[part];
    }
    if (face) {
        *cel++ = base;
        *cel++ = where.x - g_4b2658[base];
        *cel++ = where.y - g_4b265c[base] - dy;
    }
    if (snoid->features[3]) {
        part = snoid->features[3] + base + 15;
        *cel++ = part;
        *cel++ = where.x - g_4b2658[part];
        *cel++ = where.y - g_4b265c[part] - dy;
    }
    if (snoid->features[1]) {
        part = snoid->features[1] + base + 5;
        *cel++ = part;
        *cel++ = where.x - g_4b2658[part];
        *cel++ = where.y - g_4b265c[part] - dy;
    }
    if (snoid->features[2]) {
        part = snoid->features[2] + base + 10;
        *cel++ = part;
        *cel++ = where.x - g_4b2658[part];
        *cel++ = where.y - g_4b265c[part] - dy;
    }
    if (snoid->features[0]) {
        part = snoid->features[0] + base;
        *cel++ = part;
        *cel++ = where.x - g_4b2658[part];
        *cel++ = where.y - g_4b265c[part] - dy;
    }
    *cel++ = 0;
    *cel++ = 0;
    *cel = 0;
    cel = (short *)snoid->body.cels;
    bank = g_4b2634;
    while (*cel && *cel <= bank->count) {
        unsigned short *image = (unsigned short *)(bank->offsets[*cel] + (char *)bank);

        cel++;
        rect.left = *cel++;
        rect.top = *cel++;
        rect.right = swapShort(image[0]) + rect.left;
        rect.bottom = swapShort(image[1]) + rect.top;
        unionRect(&snoid->body.bounds, &rect);
    }
}

/*
 * Gives a Zoombini (for slot n, 0-3) one or two random features, drawing
 * features (g_4b27ac) and values (g_4b27b6) without repeats: for slots 0
 * and 1 values unlike the ones already chosen (g_4b279c), recorded in slot
 * n + 1; for 2 and 3, mostly the chosen ones (else those set in g_4b263c),
 * recorded in slot n + 2 and g_4b27a4. Then updates g_4b263c, and places
 * the Zoombini at g_4a4514[n].
 */
/* @zoombi32 0x00452258 */
void fn_452258(Snoid *snoid, short n)
{
    short *order = g_4b27ac;
    short left;
    short r;
    short start;
    short i;
    short j;
    short k;

    for (j = 0; j < 4; j++)
        snoid->features[j] = 0;
    for (i = 0; i < 5; i++)
        order[i] = i;
    for (i = 0; i < 6; i++)
        g_4b27b6[i] = i + 1;
    g_4b27c6 = 0;
    g_4b27c8 = 3;
    g_4b27c2 = 0;
    g_4b27c4 = 4;
    if (!n)
        for (j = 0; j < 4; j++) {
            g_4b279c[j] = 0;
            g_4b27a4[j] = 0;
        }
    left = randomBetween(1, 2);
    start = left;
    for (i = 0; i < 4 && left > 0; i++) {
        g_4b27c2 = randomBetween(0, g_4b27c4);
        g_4b27c6 = randomBetween(0, g_4b27c8);
        switch (n) {
        case 0:
        case 1:
            snoid->unknownF1 = 0;
            if (g_4b279c[order[g_4b27c6]] != g_4b27b6[g_4b27c2]) {
                snoid->features[order[g_4b27c6]] = g_4b27b6[g_4b27c2];
                g_4b26cc[n + 1][order[g_4b27c6]] = g_4b27b6[g_4b27c2];
                g_4b279c[order[g_4b27c6]] = g_4b27b6[g_4b27c2];
                left--;
            }
            break;
        case 2:
            snoid->unknownF1 = 2;
            k = randomBetween(0, 3);
            if (g_4b279c[k]) {
                if ((r = randomBetween(0, 100)) > 65 || start == left && i == 3) {
                    snoid->features[k] = g_4b279c[k];
                    g_4b26cc[n + 2][k] = g_4b279c[k];
                    g_4b27a4[k] = g_4b279c[k];
                    left--;
                }
            } else {
                snoid->features[k] = g_4b263c[k];
                g_4b26cc[n + 2][k] = g_4b263c[k];
                g_4b27a4[k] = g_4b263c[k];
                left--;
            }
            break;
        case 3:
            snoid->unknownF1 = 2;
            k = randomBetween(0, 3);
            if (g_4b279c[k]) {
                if ((r = randomBetween(0, 100)) > 65 || start == left && i == 3) {
                    snoid->features[k] = g_4b279c[k];
                    g_4b26cc[n + 2][k] = g_4b279c[k];
                    g_4b27a4[k] = g_4b279c[k];
                    left--;
                }
            } else {
                snoid->features[k] = g_4b263c[k];
                g_4b26cc[n + 2][k] = g_4b263c[k];
                g_4b27a4[k] = g_4b263c[k];
                left--;
            }
            break;
        }
        for (j = g_4b27c6; j < g_4b27c8 + 1; j++)
            order[j] = order[j + 1];
        g_4b27c8--;
        for (j = g_4b27c2; j < g_4b27c4 + 1; j++)
            g_4b27b6[j] = g_4b27b6[j + 1];
        g_4b27c4--;
    }
    if (g_4b279c[0])
        g_4b263c[0] = g_4b279c[0];
    if (g_4b279c[1])
        g_4b263c[1] = g_4b279c[1];
    if (g_4b279c[2])
        g_4b263c[2] = g_4b279c[2];
    if (g_4b279c[3])
        g_4b263c[3] = g_4b279c[3];
    if (n == 3) {
        for (j = 5; j > 3; j--) {
            if (g_4b26cc[j][0])
                g_4b27a4[0] = g_4b26cc[j][0];
            if (g_4b26cc[j][1])
                g_4b27a4[1] = g_4b26cc[j][1];
            if (g_4b26cc[j][2])
                g_4b27a4[2] = g_4b26cc[j][2];
            if (g_4b26cc[j][3])
                g_4b27a4[3] = g_4b26cc[j][3];
        }
        if (g_4b263c[0] == g_4b27a4[0])
            g_4b263c[0] = 0;
        if (g_4b263c[1] == g_4b27a4[1])
            g_4b263c[1] = 0;
        if (g_4b263c[2] == g_4b27a4[2])
            g_4b263c[2] = 0;
        if (g_4b263c[3] == g_4b27a4[3])
            g_4b263c[3] = 0;
    }
    *(Point *)&snoid->body.x = g_4a4514[n];
    snoid->unknownF4 = 4;
}

/* Deals features to the four Zoombinis in the views g_4b269a. */
/* @zoombi32 0x0045222d */
void fn_45222d()
{
    short i;
    View *view;

    for (i = 0; i < 4; i++) {
        view = findView(g_4b269a[i]);
        if (view)
            fn_452258((Snoid *)&view->body, i);
    }
}

/* Adds a view for a Zoombini of this scene (drawn by fn_4541bf, updated by
   fn_454228) from `snoid`; returns its id. */
/* @zoombi32 0x00454165 */
short fn_454165(Snoid *snoid)
{
    short id;
    View *view;

    id = addView(1, fn_4541bf, fn_454228, 0, 6, snoid, 0, 0);
    view = findView(id);
    if (view) {
        fn_454374(snoid);
        view->unknown1e = 0;
        view->nextUpdate = 0;
        view->body.frameOffset = 0;
        view->flags = 0x4000002;
    }
    return id;
}

/* The scene's Zoombini views' update: cycles the feature being changed
   (unknownF5) through its values every 60 ticks while unknownF8 is set,
   else flashes it (every 30) between unknown1e and nothing; lays the
   Zoombini out again when it changes. */
/* @zoombi32 0x00454228 */
void fn_454228(View *view, short region)
{
    short changed = 0;
    Snoid *snoid;
    char *features;

    if (!g_4b9684 && view->body.running && clockTime() >= view->nextUpdate) {
        view->nextUpdate = clockTime() + view->interval;
        snoid = (Snoid *)&view->body;
        if (snoid->unknownF8 && clockTime() >= view->body.frameOffset) {
            view->body.frameOffset = clockTime() + 60;
            features = snoid->features;
            snoid->unknownF8++;
            if (snoid->unknownF8 > 5)
                snoid->unknownF8 = 1;
            if (snoid->unknownF5 > 0)
                features[snoid->unknownF5 - 1] = snoid->unknownF8;
            snoid->unknownF4 = 4;
        } else if (view->unknown1e && clockTime() >= view->body.frameOffset) {
            view->body.frameOffset = clockTime() + 30;
            features = snoid->features;
            if (snoid->unknownF5 > 0 && !features[snoid->unknownF5 - 1])
                features[snoid->unknownF5 - 1] = view->unknown1e;
            else if (snoid->unknownF5 > 0)
                features[snoid->unknownF5 - 1] = 0;
            snoid->unknownF4 = 4;
        }
        switch (snoid->unknownF4) {
        case 2:
            break;
        default:
            changed = 1;
            break;
        }
        if (changed) {
            unionRgnRect(region, &view->body.bounds);
            fn_454374(snoid);
            view->changed = 1;
        }
    }
}

/*
 * The scene's views' notify: its scripts' events move the scene along
 * (setting flags the scene's idle work picks up, starting the next views'
 * scripts, handing Zoombinis from view to view); 0 and 251 turn a
 * Zoombini round.
 */
/* @zoombi32 0x0045174e */
void fn_45174e(View *view, short event)
{
    View *other;
    Snoid *first;
    Snoid *second;
    short i;

    switch (event) {
    case 0:
        if (view->flags == 1) {
            Snoid *snoid = (Snoid *)&view->body;

            snoid->unknownF2 = !snoid->unknownF2;
        }
        break;
    case 251:
        if (view->flags == 1)
            setSnoidFacing((Snoid *)&view->body, 1);
        break;
    case 1:
        g_4b2644 = 1;
        break;
    case 2:
        fn_451315();
        break;
    case 3:
        if (g_4b2630 > 0 && g_4b2630 < 4)
            g_4b274a = 1;
        break;
    case 4:
        fn_4514f6();
        break;
    case 10:
    case 11:
    case 13:
    case 14:
        if (g_4b26b2) {
            other = findView(g_4b26b2);
            if (other) {
                Snoid *snoid = (Snoid *)&other->body;

                fn_45170a(g_4b26b2, g_4b271c[g_4b2740] + snoid->features[3], view->body.group, fn_45174e, 1);
            }
        }
        break;
    case 16:
        other = findView(g_4b26ac[0]);
        if (other) {
            other->nextUpdate = 0;
            first = (Snoid *)&other->body;
            first->unknownF4 = 4;
        }
        other = findView(g_4b26ac[1]);
        if (other) {
            other->nextUpdate = 0;
            second = (Snoid *)&other->body;
            second->unknownF4 = 4;
        }
        if (g_4b2740) {
            *(Point *)&first->body.x = g_4a46a4[g_4b2788];
            *(Point *)&second->body.x = g_4a46e8[g_4b2788];
        } else {
            *(Point *)&first->body.x = g_4a4634[g_4b2788];
            *(Point *)&second->body.x = g_4a466c[g_4b2788];
        }
        g_4b2788++;
        break;
    case 17:
        if (g_4b26b2) {
            for (i = 0; i < g_4b262e; i++)
                if (g_4b2604[i] == g_4b26b2 && g_4b2630 != 4) {
                    g_4b2604[i] = 0;
                    i = g_4b262e;
                }
            if (g_4b273c)
                g_4b26b2 = 0;
        }
        fn_45162e(view->body.group);
        break;
    case 30:
        if (g_4b2604[g_4b2734] && g_4b2734 < g_4b262e) {
            other = findView(g_4b2604[g_4b2734]);
            if (other) {
                other->body.running = 0;
                g_4b26b2 = g_4b2604[g_4b2734];
                Snoid *snoid = (Snoid *)&other->body;

                *(Point *)&snoid->body.x = g_4a44b0;
                fn_45170a(g_4b2604[g_4b2734], g_4b272a, view->body.group, fn_45174e, 0);
                moveView(g_4b2604[g_4b2734], 1, view->id);
            }
        }
        break;
    case 31:
        other = findView(g_4b25a6);
        if (other) {
            moveView(g_4b25a6, 1, g_4b25a4);
            other->flags = 0x4108000;
            setViewScript(other, g_4b2726, 1);
            other->notify = fn_45174e;
        }
        break;
    case 35:
        if (g_4b2604[g_4b2734 + 1] && g_4b2734 + 1 < g_4b262e) {
            other = findView(g_4b2604[g_4b2734 + 1]);
            if (other) {
                other->body.running = 0;
                Snoid *snoid = (Snoid *)&other->body;

                *(Point *)&snoid->body.x = g_4a44b0;
                fn_45170a(g_4b2604[g_4b2734 + 1], g_4b272c, view->body.group, fn_45174e, 0);
                moveView(g_4b2604[g_4b2734 + 1], 1, view->id);
            }
        }
        break;
    case 36:
        if (g_4b266e) {
            g_4b266e = 0;
            fn_450c24(g_4b26b2, 0);
            fn_450d00(g_4b26b2, 0);
        }
        break;
    case 37:
        if (g_4b2604[g_4b2734] && g_4b2734 < g_4b262e) {
            g_4b26b2 = g_4b2604[g_4b2734];
            fn_45170a(g_4b2604[g_4b2734], g_4b272e, view->body.group, fn_45174e, 0);
            moveView(g_4b2604[g_4b2734], 1, view->id);
        }
        if (g_4b2742) {
            other = findView(g_4b25a4);
            if (other) {
                other->flags = 0x4108000;
                setViewScript(other, g_4b2726, 1);
                other->notify = fn_45174e;
                ViewBody *body = &other->body;

                body->cels[0].image = 0;
            }
        } else {
            other = findView(g_4b25a6);
            if (other) {
                other->flags = 0x4108000;
                setViewScript(other, g_4b2726, 1);
                other->notify = fn_45174e;
                ViewBody *body = &other->body;

                body->cels[0].image = 0;
            }
        }
        break;
    case 38:
        if (g_4b2630 == 4) {
            if (g_4b2744 == 3) {
                if (g_4b26b2) {
                    fn_450c24(g_4b26b2, 0);
                    fn_450d00(g_4b26b2, 0);
                    fn_450d5d();
                    fn_450e87();
                    fn_4512ac();
                    other = findView(g_4b26ba[7]);
                    if (other) {
                        first = (Snoid *)&other->body;
                        ViewBody *body = &first->body;

                        body->cels[0].image = 0;
                    }
                    if (g_4b2604[g_4b2734]) {
                        other = findView(g_4b2594);
                        if (other) {
                            setViewScript(other, g_4b2730, 1);
                            other->notify = fn_451e5d;
                            g_4a483c = 0;
                        }
                    }
                }
            } else if (g_4b2744 == 1 && g_4b273a && g_4b266c <= g_4b262e) {
                g_4b2754 = 1;
                fn_45062d(11005);
            }
        }
        break;
    case 50:
        g_4b2736 = 1;
        break;
    case 51:
        g_4b2746 = 1;
        break;
    case 60:
        g_4b755a = 0;
        g_4b755c = 1;
        break;
    }
}

/*
 * Makes the scene's puzzle (with n 1) and gives the Zoombini for row n its
 * features. The rows (g_4b27ca, with the second set in g_4b2812) are built
 * from the features set in g_4b263c: rows 1 and 2 change one or two
 * features at random, rows 3 and 4 (at g_4b2630 3 and 4) follow on from
 * them, row 7 (and 8, for the second set) from rows 3 and 4, and rows 5
 * and 6 differ by level. g_4b285a marks the features a row changes; the
 * Zoombini's first such feature is the one it changes (unknownF5).
 */
/* @zoombi32 0x00452d5d */
void fn_452d5d(Snoid *snoid, short n)
{
    short pick;
    short last;
    short once;
    short count;
    short chosen;
    short limit;
    short row2;
    short values[8];
    short i;
    short row;
    short j;

    if (n == 1) {
        for (j = 0; j < 9; j++) {
            g_4b27ca[j][0] = 0;
            g_4b27ca[j][1] = 0;
            g_4b27ca[j][2] = 0;
            g_4b27ca[j][3] = 0;
            g_4b2812[j][0] = 0;
            g_4b2812[j][1] = 0;
            g_4b2812[j][2] = 0;
            g_4b2812[j][3] = 0;
            g_4b285a[j][0] = 0;
            g_4b285a[j][1] = 0;
            g_4b285a[j][2] = 0;
            g_4b285a[j][3] = 0;
        }
        g_4b28b2[0] = 0;
        g_4b28b2[1] = 0;
        g_4b28b2[2] = 0;
        g_4b28b2[3] = 0;
        g_4b28ba[0] = 0;
        g_4b28ba[1] = 0;
        g_4b28ba[2] = 0;
        g_4b28ba[3] = 0;
        g_4b27ca[0][0] = g_4b263c[0];
        g_4b27ca[0][1] = g_4b263c[1];
        g_4b27ca[0][2] = g_4b263c[2];
        g_4b27ca[0][3] = g_4b263c[3];
        g_4b28a2[0] = g_4b263c[0];
        g_4b28a2[1] = g_4b263c[1];
        g_4b28a2[2] = g_4b263c[2];
        g_4b28a2[3] = g_4b263c[3];
        g_4b2812[0][0] = g_4b263c[4];
        g_4b2812[0][1] = g_4b263c[5];
        g_4b2812[0][2] = g_4b263c[6];
        g_4b2812[0][3] = g_4b263c[7];
        g_4b28aa[0] = g_4b263c[4];
        g_4b28aa[1] = g_4b263c[5];
        g_4b28aa[2] = g_4b263c[6];
        g_4b28aa[3] = g_4b263c[7];
        for (row = 1; row < 3; row++) {
            for (j = 0; j < 8; j++)
                values[j] = j;
            count = 0;
            once = 0;
            last = 5;
            chosen = randomBetween(0, 4);
            for (i = 0; i < 4; i++)
                if (count < 2) {
                    pick = randomBetween(1, last);
                    if (i == chosen && randomBetween(0, 100) > 70 && !once) {
                        once = 1;
                        if (!g_4b28a2[i])
                            g_4b27ca[row][i] = g_4b263c[i] + 1;
                        else
                            g_4b27ca[row][i] = g_4b28a2[i] + 1;
                        if (g_4b27ca[row][i] > 5)
                            g_4b27ca[row][i] = 1;
                        if (!g_4b28aa[i])
                            g_4b27ca[row][i] = g_4b263c[i + 4] + 1; /* sic: not g_4b2812 */
                        else
                            g_4b2812[row][i] = g_4b28aa[i] + 1;
                        if (g_4b2812[row][i] > 5)
                            g_4b2812[row][i] = 1;
                        g_4b285a[row][i] = g_4b27ca[row][i];
                    } else if (randomBetween(0, 100) > 40 || i == 3 && count == 0) {
                        g_4b27ca[row][i] = values[pick];
                        g_4b2812[row][i] = values[pick];
                        g_4b285a[row][i] = 0;
                    }
                    if (g_4b27ca[row][i]) {
                        g_4b28a2[i] = g_4b27ca[row][i];
                        g_4b28aa[i] = g_4b2812[row][i];
                        count++;
                        for (j = pick; j < last + 1; j++)
                            values[j] = values[j + 1];
                        last--;
                    }
                }
        }
        limit = 2;
        if (g_4b2630 == 4 || g_4b2630 == 3) {
            g_4b28b2[0] = g_4b28a2[0];
            g_4b28b2[1] = g_4b28a2[1];
            g_4b28b2[2] = g_4b28a2[2];
            g_4b28b2[3] = g_4b28a2[3];
            g_4b28ba[0] = g_4b28aa[0];
            g_4b28ba[1] = g_4b28aa[1];
            g_4b28ba[2] = g_4b28aa[2];
            g_4b28ba[3] = g_4b28aa[3];
            for (row = 3; row < 5; row++) {
                for (j = 0; j < 8; j++)
                    values[j] = j;
                count = 0;
                once = 0;
                last = 5;
                randomBetween(0, 4);
                for (i = 0; i < 4; i++)
                    if (count < limit) {
                        pick = randomBetween(1, last);
                        if ((randomBetween(0, 100) > 70 || i == 3 && count == 0) && !once) {
                            once = 1;
                            if (row == 3) {
                                if (g_4b28b2[i]) {
                                    g_4b27ca[row][i] = g_4b28b2[i];
                                    g_4b2812[row][i] = g_4b28ba[i];
                                } else {
                                    g_4b27ca[row][i] = g_4b263c[i];
                                    g_4b2812[row][i] = g_4b263c[i + 4];
                                }
                            } else if (g_4b285a[row - 1][i]) {
                                g_4b27ca[row][i] = g_4b28b2[i] - 1;
                                if (g_4b27ca[row][i] < 1)
                                    g_4b27ca[row][i] = 5;
                                g_4b2812[row][i] = g_4b28ba[i] - 1;
                                if (g_4b2812[row][i] < 1)
                                    g_4b2812[row][i] = 5;
                            } else if (g_4b27ca[row - 1][i]) {
                                g_4b27ca[row][i] = values[pick];
                                g_4b2812[row][i] = values[pick];
                            } else if (g_4b28b2[i]) {
                                g_4b27ca[row][i] = g_4b28b2[i];
                                g_4b2812[row][i] = g_4b28ba[i];
                            } else {
                                g_4b27ca[row][i] = g_4b263c[i];
                                g_4b2812[row][i] = g_4b263c[i + 4];
                            }
                            g_4b285a[row][i] = g_4b27ca[row][i];
                        } else if (row == 3) {
                            if (!g_4b285a[2][i] && g_4b27ca[2][i]) {
                                g_4b27ca[row][i] = g_4b28b2[i];
                                g_4b2812[row][i] = g_4b28ba[i];
                            } else if (!g_4b285a[1][i] && g_4b27ca[1][i]) {
                                g_4b27ca[row][i] = g_4b28b2[i];
                                g_4b2812[row][i] = g_4b28ba[i];
                            } else {
                                g_4b27ca[row][i] = 0;
                                g_4b2812[row][i] = 0;
                            }
                            g_4b285a[row][i] = 0;
                        } else if (g_4b285a[row - 1][i]) {
                            if (!once) {
                                g_4b27ca[row][i] = g_4b28b2[i] - 1;
                                if (g_4b27ca[row][i] < 1)
                                    g_4b27ca[row][i] = 5;
                                g_4b2812[row][i] = g_4b28ba[i] - 1;
                                if (g_4b2812[row][i] < 1)
                                    g_4b2812[row][i] = 5;
                                once = 1;
                                g_4b285a[row][i] = g_4b27ca[row][i];
                            } else {
                                g_4b285a[row][i] = 0;
                            }
                        } else if (g_4b27ca[row - 1][i]) {
                            g_4b27ca[row][i] = values[pick];
                            g_4b2812[row][i] = values[pick];
                            g_4b285a[row][i] = 0;
                        } else if (!g_4b285a[2][i] && g_4b27ca[2][i] || !g_4b285a[1][i] && g_4b27ca[1][i]) {
                            g_4b27ca[row][i] = g_4b28b2[i];
                            g_4b2812[row][i] = g_4b28ba[i];
                            g_4b285a[row][i] = 0;
                        } else {
                            g_4b27ca[row][i] = 0;
                            g_4b2812[row][i] = 0;
                            g_4b285a[row][i] = 0;
                        }
                        if (g_4b27ca[row][i]) {
                            g_4b28b2[i] = g_4b27ca[row][i];
                            g_4b28ba[i] = g_4b2812[row][i];
                            count++;
                            for (j = pick; j < last + 1; j++)
                                values[j] = values[j + 1];
                            last--;
                        }
                    }
            }
        }
        row = 7;
        for (i = 0; i < 4; i++)
            if (g_4b285a[4][i]) {
                g_4b27ca[row][i] = g_4b27ca[4][i] - 1;
                if (g_4b27ca[row][i] < 1)
                    g_4b27ca[row][i] = 5;
            } else if (g_4b27ca[4][i]) {
                g_4b27ca[row][i] = randomBetween(1, 5);
            } else if (g_4b285a[3][i]) {
                g_4b27ca[row][i] = g_4b27ca[3][i] - 1;
                if (g_4b27ca[row][i] < 1)
                    g_4b27ca[row][i] = 5;
            } else if (g_4b27ca[3][i]) {
                g_4b27ca[row][i] = randomBetween(1, 5);
            } else {
                g_4b27ca[row][i] = g_4b28b2[i];
            }
        if (g_4b263c[4]) {
            row = 8;
            for (i = 0; i < 4; i++)
                if (g_4b285a[4][i]) {
                    g_4b2812[row][i] = g_4b2812[4][i] - 1;
                    if (g_4b2812[row][i] < 1)
                        g_4b2812[row][i] = 5;
                } else if (g_4b2812[4][i]) {
                    g_4b2812[row][i] = randomBetween(1, 5);
                } else if (g_4b285a[3][i]) {
                    g_4b2812[row][i] = g_4b2812[3][i] - 1;
                    if (g_4b2812[row][i] < 1)
                        g_4b2812[row][i] = 5;
                } else if (g_4b2812[3][i]) {
                    g_4b2812[row][i] = randomBetween(1, 5);
                } else {
                    g_4b2812[row][i] = g_4b28ba[i];
                }
        }
        if (g_4b2630 == 3) {
            for (row = 5; row < 7; row++) {
                for (j = 0; j < 8; j++)
                    values[j] = j;
                count = 0;
                last = 5;
                chosen = randomBetween(0, 3);
                for (i = 0; i < 4; i++)
                    if (count < 2) {
                        pick = randomBetween(1, last);
                        if (i == chosen && randomBetween(0, 100) > 70) {
                            g_4b27ca[row][i] = values[pick];
                            g_4b285a[row][i] = values[pick];
                        } else if (randomBetween(0, 100) > 40 || i == 3 && count == 0) {
                            g_4b27ca[row][i] = values[pick];
                            g_4b285a[row][i] = 0;
                        }
                        if (g_4b27ca[row][i]) {
                            count++;
                            for (j = pick; j < last + 1; j++)
                                values[j] = values[j + 1];
                            last--;
                        }
                    }
            }
        } else if (g_4b2630 == 4) {
            if (randomBetween(0, 1)) {
                row2 = 5;
                row = randomBetween(1, 2);
                for (i = 0; i < 4; i++)
                    if (g_4b285a[row][i]) {
                        g_4b27ca[row2][i] = g_4b27ca[row][i];
                        g_4b285a[row2][i] = g_4b27ca[row][i];
                    } else if (g_4b27ca[row][i]) {
                        g_4b27ca[row2][i] = g_4b27ca[row][i];
                        g_4b27ca[row2][i]++;
                        if (g_4b27ca[row2][i] > 5)
                            g_4b27ca[row2][i] = 1;
                        g_4b285a[row2][i] = 0;
                    }
                row2 = 6;
                randomBetween(3, 4);
                chosen = randomBetween(0, 3);
                for (i = 0; i < 4; i++)
                    if (i == chosen) {
                        g_4b27ca[row2][i] = randomBetween(1, 5);
                        g_4b285a[row2][i] = 0;
                    }
            } else {
                row2 = 6;
                row = randomBetween(3, 4);
                for (i = 0; i < 4; i++)
                    if (g_4b285a[row][i]) {
                        g_4b27ca[row2][i] = g_4b27ca[row][i];
                        g_4b285a[row2][i] = g_4b27ca[row][i];
                    } else if (g_4b27ca[row][i]) {
                        g_4b27ca[row2][i] = g_4b27ca[row][i];
                        g_4b27ca[row2][i]++;
                        if (g_4b27ca[row2][i] > 5)
                            g_4b27ca[row2][i] = 1;
                        g_4b285a[row2][i] = 0;
                    }
                row2 = 5;
                randomBetween(1, 2);
                chosen = randomBetween(0, 3);
                for (i = 0; i < 4; i++)
                    if (i == chosen) {
                        g_4b27ca[row2][i] = randomBetween(1, 5);
                        g_4b285a[row2][i] = 0;
                    }
            }
        }
    }
    count = 0;
    for (i = 0; i < 4; i++) {
        if (n == 8) {
            if (g_4b263c[4])
                snoid->features[i] = g_4b2812[n][i];
            else
                snoid->features[i] = 0;
        } else {
            snoid->features[i] = g_4b27ca[n][i];
        }
        if (g_4b285a[n][i])
            count = i + 1;
    }
    if (count) {
        snoid->unknownF8 = 1;
        snoid->unknownF5 = count;
    } else {
        snoid->unknownF8 = 0;
        snoid->unknownF5 = 0;
    }
}

/* Sets out the scene's Zoombinis: shuffles the places of views 1-6 (unless
   cheating), makes the puzzle's rows (fn_452d5d) and gives each view its
   row's features and place, then fills slot 7 and slot 0. */
/* @zoombi32 0x004508db */
void fn_4508db()
{
    short values[8];
    short i;
    short last;
    short pick;
    View *view;
    Snoid *snoid;

    for (i = 0; i < 8; i++) {
        values[i] = i;
        g_4b2768[i] = i;
    }
    if (!g_4b2798) {
        last = 6;
        for (i = 1; i < 7; i++) {
            pick = randomBetween(1, last);
            g_4b2768[i] = values[pick];
            for (; pick < last + 1; pick++)
                values[pick] = values[pick + 1];
            last--;
        }
    }
    for (i = 1; i < g_4b2662; i++) {
        view = findView(g_4b26ba[i]);
        if (view) {
            snoid = (Snoid *)&view->body;
            fn_452d5d(snoid, i);
            if (i < 7) {
                snoid->unknownF1 = 7;
                *(Point *)&snoid->body.x = g_4a44f0[g_4b2768[i]];
            } else if (i == 7) {
                snoid->unknownF1 = 0;
                *(Point *)&snoid->body.x = g_4a4528;
            }
            if (i == 8) {
                snoid->unknownF1 = 2;
                *(Point *)&snoid->body.x = g_4a4530;
            }
            snoid->unknownF4 = 4;
        }
    }
    fn_450c24(g_4b26ba[7], 7);
    fn_450d00(g_4b26ba[7], 7);
    g_4b26b2 = g_4b2604[g_4b2734];
    fn_450c24(g_4b26b2, 0);
    fn_450d00(g_4b26b2, 0);
    g_4b26b6 = 0;
    g_4b26b4 = 0;
    fillMemory(&g_4b2776[1], 0, 12);
}

/* Adds `count` Zoombini views of one kind to the scene (1: random ones at
   g_4a44cc, one of them with the features set in g_4b263c; 2: dealt ones;
   3: the puzzle's rows; 4 and 5: empty ones), recording them in that
   kind's list. */
/* @zoombi32 0x00452857 */
void fn_452857(short kind, short count)
{
    View *view;
    short chosen;
    Snoid snoid;
    Snoid *made = &snoid;
    short i;
    short j;

    chosen = randomBetween(0, count - 1);
    for (i = 0; i < count; i++) {
        switch (kind) {
        case 1:
            for (j = 0; j < 4; j++)
                made->features[j] = randomBetween(1, 5);
            made->unknownF1 = 1;
            *(Point *)&made->body.x = g_4a44cc[i];
            if (i == chosen) {
                if (g_4b263c[0])
                    made->features[0] = g_4b263c[0];
                else
                    made->features[0] = randomBetween(1, 5);
                if (g_4b263c[1])
                    made->features[1] = g_4b263c[1];
                else
                    made->features[1] = randomBetween(1, 5);
                if (g_4b263c[2])
                    made->features[2] = g_4b263c[2];
                else
                    made->features[2] = randomBetween(1, 5);
                if (g_4b263c[3])
                    made->features[3] = g_4b263c[3];
                else
                    made->features[3] = randomBetween(1, 5);
            }
            made->unknownF8 = 0;
            made->unknownF5 = 0;
            made->unknownF4 = 4;
            break;
        case 2:
            fn_452258(made, i);
            made->unknownF8 = 0;
            made->unknownF5 = 0;
            made->unknownF4 = 4;
            break;
        case 3:
            fn_452d5d(made, i + 1);
            if (i + 1 < 7)
                made->unknownF1 = 7;
            else if (i + 1 == 7)
                made->unknownF1 = 5;
            if (i + 1 == 8)
                made->unknownF1 = 3;
            *(Point *)&made->body.x = g_4a44f0[i + 1];
            made->unknownF4 = 4;
            break;
        case 4:
            for (j = 0; j < 4; j++)
                made->features[j] = 0;
            if (i == 0) {
                made->unknownF1 = 8;
                *(Point *)&made->body.x = g_4a4524[i];
            }
            if (i == 1) {
                made->unknownF1 = 5;
                *(Point *)&made->body.x = g_4a4524[i];
            }
            made->unknownF8 = 0;
            made->unknownF5 = 0;
            made->unknownF4 = 4;
            break;
        case 5:
            for (j = 0; j < 4; j++)
                made->features[j] = 0;
            *(Point *)&made->body.x = g_4a462c[i];
            if (!i)
                made->unknownF1 = 5;
            else
                made->unknownF1 = 3;
            made->unknownF8 = 0;
            made->unknownF5 = 0;
            made->unknownF4 = 4;
            break;
        default:
            made->features[1] = 1;
            made->body.x = 10;
            made->body.y = 10;
            made->unknownF1 = 0;
            made->unknownF8 = 0;
            made->unknownF5 = 0;
            break;
        }
        made->unknownC2[0] = 1;
        made->unknownC2[1] = 22;
        made->unknownC2[2] = 43;
        made->unknownC2[3] = 43;
        made->unknownC2[4] = 22;
        made->unknownC2[5] = 1;
        made->unknownC2[6] = 1;
        made->unknownC2[7] = 22;
        made->unknownC2[8] = 43;
        made->unknownF2 = 0;
        made->name[0] = 0;
        made->home = *(Point *)&made->body.x;
        *(Point *)&made->body.unknownAa = *(Point *)&made->body.x;
        *(Point *)&made->targetX = *(Point *)&made->body.x;
        made->unknownEa = 0;
        made->unknownEb = 0;
        made->unknownEc = 0;
        made->unknownEe = 0;
        made->unknownF0 = 0;
        made->unknownF7 = 0;
        j = fn_454165(made);
        if (j) {
            view = findView(j);
            if (view)
                view->flags = 0x4000002;
            switch (kind) {
            case 1:
                g_4b2672[g_4b2660] = j;
                g_4b2660++;
                moveView(j, 0, g_4b258e);
                break;
            case 2:
                g_4b269a[g_4b2664] = j;
                g_4b2664++;
                moveView(j, 1, g_4b2590);
                break;
            case 3:
                g_4b26ba[g_4b2662] = j;
                g_4b2662++;
                moveView(j, 0, g_4b258e);
                if (g_4b2662 == 8 && g_4b2630 >= 3) {
                    fn_450c24(g_4b26ba[7], 7);
                    fn_450d00(g_4b26ba[7], 7);
                    g_4b26b0 = 1;
                }
                if (g_4b2662 == 9 && g_4b2630 >= 3)
                    fn_450c24(g_4b26ba[8], 8);
                break;
            case 4:
                g_4b26a6[g_4b2666] = j;
                g_4b2666++;
                moveView(j, 1, g_4b258e);
                break;
            case 5:
                g_4b26ac[g_4b2668] = j;
                g_4b2668++;
                moveView(j, 1, g_4b258c);
                if (view) {
                    view->body.running = 0;
                    view->changed = 1;
                }
                break;
            }
        }
    }
}
