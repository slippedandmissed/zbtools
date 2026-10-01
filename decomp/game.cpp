/*
 * game (0x44b550-0x455530): cheats ('Cheat on ', 'You have entered the psychedelic ZB Zone!'), memory statistics
 */

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zoombinis.h"
#include "basecamp.h"
#include "bctwo.h"
#include "bridge.h"
#include "config.h"
#include "debug.h"
#include "e2memory.h"
#include "features.h"
#include "ferry.h"
#include "fleens.h"
#include "focus.h"
#include "game.h"
#include "graphics.h"
#include "hotel.h"
#include "isle.h"
#include "lilly.h"
#include "loading.h"
#include "mainloop.h"
#include "maze.h"
#include "net.h"
#include "os_manager.h"
#include "picker.h"
#include "pizza.h"
#include "platform.h"
#include "random.h"
#include "roster.h"
#include "slides.h"
#include "snoids.h"
#include "sound.h"
#include "town.h"
#include "tunnels.h"
#include "view.h"
#include "xfer.h"

Scene *scenes[22] = {
    g_4a73fc, g_4a1f40, g_4a7eaa, g_4a2f0c, g_4a0810, g_4a0ac8, g_4a74b0, g_4a0e10, g_4a76f4,
    g_4a3d24, g_4a1508, g_4a1bd4, g_4a3fb0, g_4a163c, g_4a1770, g_4a2e3e, g_4a0fbc, g_4a47b4,
    g_4a21a0, g_4a2052, g_4a209c, g_4a2052,
};
short level3OpenCells[26] = {
    2, 4, 6, 19, 21, 23, 25, 38, 40, 42, 44, 55, 57, 59, 61, 74, 76, 78, 80, 91, 93, 95, 97, 110,
    112, 114,
};
short level3BlockedCells[43] = {
    10, 11, 12, 13, 14, 15, 28, 29, 30, 31, 32, 33, 34, 46, 47, 48, 49, 50, 51, 52, 56, 58, 60, 64,
    65, 66, 67, 68, 69, 70, 82, 83, 84, 85, 86, 87, 88, 100, 101, 102, 103, 104, 105,
};
short level3Links36Cells[20] = {
    10, 12, 14, 29, 31, 33, 46, 48, 50, 52, 65, 67, 69, 82, 84, 86, 88, 101, 103, 105,
};
short level3Links9Cells[20] = {
    11, 13, 15, 28, 30, 32, 34, 47, 49, 51, 64, 66, 68, 70, 83, 85, 87, 100, 102, 104,
};
short rowFirstCells[14] = {0, 54, 45, 36, 27, 18, 9, 0, 18, 18, 9, 9};
short rowSteps[14] = {0, 0, 18, 18, 18, 18, 18, 18, 9, 9, 9, 9, 9, 9};
short level2BlockedCells[18] = {
    10, 12, 14, 28, 30, 32, 46, 48, 50, 64, 66, 68, 82, 84, 86, 100, 102, 104,
};
short level2OpenCells[18] = {
    11, 29, 47, 65, 83, 101, 13, 31, 49, 67, 85, 103, 24, 60, 96, 19, 55, 91,
};
short level2StartCells[3] = {18, 54, 90};
short level2Links5Cells[3] = {24, 60, 96};
short level2Links18Cells[12] = {11, 13, 29, 31, 47, 49, 65, 67, 83, 85, 101, 103};
ShortRect zoneMessageRect = {100, 20, 540, 47};
Point crossingStart = {530, 384};
Point crossingStart2 = {-8, 258};
Point smokeSpotPoint = {43, 258};
Point randomPlaces[8] = {
    {459, 26}, {535, 25}, {429, 80}, {500, 84}, {619, 76}, {423, 168}, {525, 167}, {605, 163},
};
Point rowPlaces[8] = {
    {0}, {441, 66}, {531, 70}, {605, 67}, {421, 160}, {483, 153}, {612, 153}, {548, 255},
};
Point dealtPlaces[4] = {{187, 255}, {247, 255}, {424, 255}, {484, 255}};
Point madePlaces1[3] = {{124, 255}, {548, 255}, {580, 258}};
Point smokeRowStart = {616, 253};
ShortRect spot4Rect = {525, 211, 582, 300};
Point leftRowPlaces[3][3] = {
    {{211, 255}}, {{187, 255}, {247, 255}}, {{164, 255}, {211, 255}, {258, 255}},
};
Point rightRowPlaces[3][3] = {
    {{457, 255}}, {{424, 255}, {484, 255}}, {{409, 255}, {457, 255}, {505, 255}},
};
ShortRect leftRowSpots[3][3] = {
    {{185, 230, 245, 293}}, {{137, 230, 197, 293}, {236, 230, 296, 293}},
    {{124, 230, 184, 293}, {197, 230, 257, 293}, {265, 230, 325, 293}},
};
ShortRect rightRowSpots[3][3] = {
    {{426, 230, 486, 293}}, {{372, 230, 432, 293}, {474, 230, 534, 293}},
    {{350, 230, 400, 293}, {419, 230, 469, 293}, {485, 230, 535, 293}},
};
ShortRect dealButtonRect = {9, 300, 75, 364};
Point madePlaces2[2] = {{317, 254}, {354, 254}};
Point movePlaces3[14] = {
    {317, 263}, {317, 248}, {317, 236}, {317, 210}, {317, 201}, {317, 192}, {317, 201}, {317, 210},
    {317, 236}, {317, 248}, {317, 263}, {317, 254}, {317, 254},
};
Point movePlaces4[14] = {
    {354, 263}, {354, 248}, {354, 236}, {354, 210}, {354, 201}, {354, 192}, {354, 201}, {354, 210},
    {354, 236}, {354, 248}, {354, 263}, {354, 254}, {354, 254},
};
Point movePlaces1[17] = {
    {317, 257}, {317, 261}, {317, 264}, {317, 267}, {317, 270}, {317, 274}, {317, 277}, {317, 280},
    {317, 277}, {317, 270}, {317, 267}, {317, 264}, {317, 261}, {317, 257}, {317, 254}, {317, 254},
    {317, 254},
};
Point movePlaces2[8] = {
    {354, 257}, {354, 261}, {354, 264}, {354, 267}, {354, 270}, {354, 274}, {354, 277}, {354, 280},
};
SceneButton smokeButtons[3] = {
    {{600, 403, 639, 440}}, {{600, 441, 639, 478}}, {{0, 0, 640, 480}},
};
Group g_4a4798[1] = {{g_4a0766, (InputItem *)smokeButtons, 3, 0x2068}};
GroupList smokeGroups = {g_4a4798, 1, 0, smokeClicked};
Scene g_4a47b4[1] = {{openSmoke, closeSmoke, smokeFrame, 0, smokeKey}};
long smokeButtonResource = 0;
long smokeDragOriginStart = 0;
ShortRect smokeWaitArea = {0, 31, 262, 244};
short smokeWaitRows[5] = {38, 81, 126, 176, 221};
Point smokePlaces[20] = {
    {214, 128}, {175, 126}, {135, 127}, {94, 126}, {53, 128}, {237, 176}, {196, 177}, {150, 178},
    {110, 176}, {69, 178}, {234, 36}, {195, 37}, {155, 36}, {114, 35}, {73, 38}, {237, 79},
    {196, 78}, {150, 80}, {110, 78}, {69, 79},
};
short dealerRunning = 0;
short smokeButton2Lit = 0;
short smokeButton1Drawn = 0;
short inSmokeFrame = 0;
Point freeSpotOrigin = {0};
ShortRect smokeButtonsRect = {275, 0, 375, 18};
unsigned long leastFreeMemory = 0x98967f;
unsigned short showMemoryStats = 0;
short leavingGame = 0;
short tempFileExists = 0;
char userFileName[] = "ZBUser";
char rosterFileName[42] = {
    90, 111, 111, 109, 98, 105, 110, 105, 46, 119, 104, 111, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 122, 111, 111, 109, 115, 105, 116, 101,
};
short aboveWindows311 = 0;
short shuttingDown = 0;
short saveBeforeQuitting = 0;
short loadingImages = 0;
short cursorAnimation[12] = {2, 4, 2, 3, 2, 5, 2, 5, 2, 4, 3, 5};
ShortRect memoryStatsRect = {300, 0, 639, 14};
char msgRequiresQuickTime[97] = {
    82, 101, 113, 117, 105, 114, 101, 115, 32, 81, 117, 105, 99, 107, 84, 105, 109, 101, 32, 102,
    111, 114, 32, 87, 105, 110, 100, 111, 119, 115, 32, 118, 101, 114, 115, 105, 111, 110, 32, 50,
    46, 49, 46, 0, 82, 101, 113, 117, 105, 114, 101, 115, 32, 83, 111, 117, 110, 100, 32, 77, 97,
    110, 97, 103, 101, 114, 32, 51, 46, 49, 32, 111, 114, 32, 108, 97, 116, 101, 114, 32, 116, 111,
    32, 98, 101, 32, 105, 110, 115, 116, 97, 108, 108, 101, 100, 46,
};
char msgInitOs[] = "unable to initialize os";
char msgInitTimer[] = "unable to initialize timer";
char msgInitHeap[] = "unable to initialize heap";
char msgNotEnoughMemory[] = "Not enough free memory";
char msgNotEnoughPhysicalMemory[] = "Not enough free physical memory";
char msgInitFileManager[] = "unable to initialize file manager";
char msgInitResourceManager[] = "unable to initialize resource manager";
char msgInitConfiguration[] = "unable to initialize configuration file manager";
char msgInitSound[] = "unable to initialize sound";
char msgNoWaveDevices[] = "no digital sound devices found";
char msgNoMidiDevices[261] = {
    110, 111, 32, 109, 105, 100, 105, 32, 115, 111, 117, 110, 100, 32, 100, 101, 118, 105, 99, 101,
    115, 32, 102, 111, 117, 110, 100, 0, 71, 101, 116, 86, 111, 108, 32, 102, 97, 105, 108, 101,
    100, 46, 32, 32, 66, 111, 111, 116, 32, 100, 114, 105, 118, 101, 46, 0, 71, 101, 116, 86, 73,
    110, 102, 111, 32, 102, 97, 105, 108, 101, 100, 46, 0, 83, 101, 116, 86, 111, 108, 32, 102, 97,
    105, 108, 101, 100, 46, 32, 32, 66, 111, 111, 116, 32, 100, 114, 105, 118, 101, 46, 0, 83, 101,
    116, 86, 111, 108, 32, 102, 97, 105, 108, 101, 100, 46, 32, 32, 68, 101, 102, 97, 117, 108, 116,
    32, 100, 105, 114, 101, 99, 116, 111, 114, 121, 46, 0, 71, 101, 116, 86, 111, 108, 32, 102, 97,
    105, 108, 101, 100, 46, 32, 32, 68, 101, 102, 97, 117, 108, 116, 32, 100, 105, 114, 101, 99,
    116, 111, 114, 121, 46, 0, 83, 121, 115, 69, 110, 118, 105, 114, 111, 110, 115, 32, 102, 97,
    105, 108, 101, 100, 46, 0, 82, 101, 113, 117, 105, 114, 101, 115, 32, 97, 116, 32, 108, 101, 97,
    115, 116, 32, 51, 48, 48, 75, 32, 111, 102, 32, 102, 114, 101, 101, 32, 115, 112, 97, 99, 101,
    32, 111, 110, 32, 98, 111, 111, 116, 32, 100, 114, 105, 118, 101, 46, 0, 68, 105, 114, 67, 114,
    101, 97, 116, 101, 32, 102, 97, 105, 108, 101, 100, 46,
};
char msgOutOfMemory[] = "Out of Memory.";

PlacedSnoid placedSnoids[17];
short view11076;
short view11009;
short pairView1;
short pairView2;
short view11018;
short view11019;
short dealerView;
short view11077;
short moverView1;
short moverView2;
short leftRowView;
short rightRowView;
short view11036;
short view11008;
short dealButtonView;
short crossedMarkers[20];
short crossedSnoids[20];
short crossingViews[21];
short crossingCount;
short smokeLevel;
ImageBank *smokeImages;
long smokeImagesResource;
char targetFeatures[8];
short view11017Due;
short unusedSmoke1;
long smokeHotSpotsXResource;
long smokeHotSpotsYResource;
short *smokeHotSpotsX;
short *smokeHotSpotsY;
short randomViewCount;
short rowViewCount;
short dealtViewCount;
short slotPairViewCount;
short comparedViewCount;
short crossedCount;
short crossOnceFlag;
short crossedChosen;
short randomViews[8];
short dealtViews[4];
short slotPairViews[3];
short comparedViews[2];
short randomDragSlot;
short crossingSnoid;
short leftRow;
short rightRow;
short rowViews[9];
short featureSlots[8][4];
short pairScripts[4];
short crossScripts[2];
short moverScripts[3];
short crossScript1;
short crossScript2;
short crossScript3;
short dealerScript;
short dealerScript2;
short nextCrossing;
short crossedDue;
short unusedSmoke2;
short featuresTaken;
short pairMismatch;
short pairScriptIndex;
short slotsDiffer;
short useSecondMover;
short level4Stage;
short anchorDue;
short dealLightDue;
short dealDimDue;
short unusedSmoke3;
short soundOnBeforeSmoke;
short leadersDue;
short dealLit;
short dealButtonState;
long lastSmokeFidgetTime;
unsigned long smokeFidgetersUsed;
short smokeFidgets;
short smokeFidgeting;
long unusedSmoke4;
short rowPlaceOrder[8];
short slotViews[6];
short movePlace;
long smokeFile;
short smokeOpen;
short smokeGoReady;
long smokeDragOrigin;
short cheatMode;
short leaderGroup;
short leftChosenValues[4];
short rightChosenValues[4];
short featureOrder[5];
short valueOrder[6];
short featurePick;
short featurePickMax;
short valuePick;
short valuePickMax;
short rowFeatures[9][4];
short rowFeatures2[9][4];
short rowChanges[9][4];
short leftTargetFeatures[4];
short rightTargetFeatures[4];
short leftFeatureMarks[4];
short rightFeatureMarks[4];
Font *fonts[3];
char moduleFileName[256];
char rosterDirectory[256];
short movieShowing;
short introPending;
LONG_PTR currentMovie;
LONG_PTR movieController;
unsigned short instanceAtom;
basePort *portBeforeMovie;
short quickTimeReady;
short rosterReady;
short startedWithoutModifier;
short cursorFrame;
HINSTANCE appInstance;
HINSTANCE appPreviousInstance;
char *appCommandLine;
short blockScreenSaver;
long cursors[6];
short modeCursors[6];
unsigned long nextCursorFrameTime;

/*
 * The game's part of each pass of the main loop (WinMain registers it with
 * setFrameHook; mainLoopEvents calls it): runs the current scene's frame
 * function, and every 12 ticks steps through cursorAnimation (setCursorMode; an
 * animated cursor?).
 */
/* @zoombi32 0x00454f61 */
void gameFrame()
{
#ifdef ZB_DEBUG
    extern void zbDebugFrame(); /* the port's debug tooling (port/debug/) */
    zbDebugFrame();
#endif
    if (currentScene != -1 && scenes[currentScene]->frame) {
        basePort *saved = getPort();
        setPort(workPort);
        scenes[currentScene]->frame();
        setPort(saved);
    }
    if (loadingImages)
        drawMemoryStats(1);
    else
        drawMemoryStats(0);
    if (cursorMode >= 1) {
        unsigned long now = clockTime();
        if (now >= nextCursorFrameTime) {
            nextCursorFrameTime = now + 12;
            if (cursorFrame >= 12)
                cursorFrame = 0;
            setCursorMode(cursorAnimation[cursorFrame]);
            cursorFrame++;
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
    atexit(shutDownAtExit);

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
    rosterReady = 0;
    quickTimeReady = 0;
    appName = "Zoombini";
    quietSoundErrors = 1;
    clockInTicks = 1;
    checkSoundLoaded = 0;
    minimizeWhenInactive = 1;
    allowModeChange = 0;
    setFrameHook(gameFrame);
    setFatalHook(shutDownGame);
    setClickHook(refreshCursor);
    setAboutHook(showAboutBox);
    startedWithoutModifier = addModifierKeys(0) != 0x800;

    if (osStartup(instance, osBuffer, sizeof osBuffer))
        fatalError(msgInitOs);
    if (initTimers())
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
    if (initIni())
        fatalError(msgInitConfiguration);
    if (initResources())
        fatalError(msgInitResourceManager);
    if (initSound())
        fatalError(msgInitSound);
    if (!waveOutGetNumDevs())
        fatalError(msgNoWaveDevices);
    if (!midiOutGetNumDevs())
        fatalError(msgNoMidiDevices);

    enterGameDirectory();
    unusedPathHook(rosterDirectory, rosterFileName);
    readWriteSavedGames(0, 0);
    strcat(userFileName, ".txt");
    unusedPathHook(moduleFileName, userFileName);
    findGameData();

    mode.width = 640;
    mode.height = 480;
    mode.palettized = 1;
    mode.colors = 256;
    initGraphics(&mode, 1);
    if (instanceAtom) {
        GlobalDeleteAtom(instanceAtom);
        instanceAtom = 0;
    }
    setTakeStatic(0);
    realizePalette(getPortPalette(), 1);

    for (i = 0; i < 3; i++)
        fonts[i] = 0;
    loadFont(&fonts[1], "CornerStone", 13, 0);
    loadFont(&fonts[2], "CornerStone", 18, 0);
    setFont(fonts[1]);

    practiceLevel = 0;
    gameState = (char *)newPtr(0xae05);
    if (!gameState)
        reportRosterError(msgOutOfMemory);
    fillRosterHeader(1);
    applyPlayerSettings();
    sceneDue = 0;
    journeyFrom = -1;
    journeyTo = -1;
    currentScene = -1;
    initViews();
    loadSnoids(1);

    /* Cursors 1-5 ('CURS' resources). */
    for (i = 0; i < 6; i++) {
        cursors[i] = 0;
        modeCursors[i] = 0;
        if (i) {
            loadResourceAs(&cursors[i], RESOURCE_TYPE('C', 'U', 'R', 'S'), i, 0, 1);
            modeCursors[i] = usedResourceHandle(cursors[i]);
            lockHandleAlias(modeCursors[i]);
        }
    }
    setCurrentMap(0);

    long quickTimeVersion = 0;
    if (QTInitialize(&quickTimeVersion) || quickTimeVersion < 0x2300)
        fatalError(msgRequiresQuickTime);
    if (qtim_0b())
        fatalError(msgRequiresQuickTime);
    else
        quickTimeReady = 1;

    pendingScene = 0;
    while (mainLoopUpdate() && !quitRequested)
        mainLoopEvents();
    quitSilently();
    return 0;
}

/* Registered with atexit, which needs the C convention. */
/* @zoombi32 0x00454ca4 */
void __cdecl shutDownAtExit()
{
    shutDownGame();
}

/* Picks a free spot (0-19) for something at freeSpotOrigin: finds, for each of
   the 20 places in smokePlaces, the nearest spot no earlier place has taken,
   then picks the first place without one, from either end at random (-1:
   none). */
/* @zoombi32 0x00450540 */
void pickFreeSpot(short *result)
{
    Point where = freeSpotOrigin;
    short i;
    short skip;
    short spot;
    short j;

    spotTaken(&where, 0, 500);
    for (i = 0; i < 20; i++) {
        skip = 0;
        spot = spotNear(&smokePlaces[i], 500, skip);
        for (j = 0; spot && j < i; j++)
            if (spot == sortedIds[j]) {
                skip++;
                spot = spotNear(&smokePlaces[i], 500, skip);
                j = 0;
            }
        sortedIds[i] = spot;
    }
    spot = -1;
    if (randomBetween(1, 100) <= 50) {
        for (i = 0; spot == -1 && i <= 19; i++)
            if (!sortedIds[i])
                spot = i;
    } else {
        for (i = 19; spot == -1 && i >= 0; i--)
            if (!sortedIds[i])
                spot = i;
    }
    *result = spot;
}

/* @zoombi32 0x0045062d */
void lightDealButton(short script)
{
    View *view = findView(dealButtonView);

    if (view) {
        view->flags = 0x4008000;
        setViewScript(view, script, 1);
    }
}

/* @zoombi32 0x00450658 */
void pressDealButton(short script, short running)
{
    View *view = findView(dealButtonView);

    if (view) {
        unionRgnRect(removedRgn, &view->body.bounds);
        view->nextUpdate = 0;
        setViewScript(view, script, running);
        view->flags = 0x4188000;
        view->notify = smokeViewNotify;
    }
}

/* @zoombi32 0x004506a9 */
void dimDealButton(short script)
{
    View *view = findView(dealButtonView);

    if (view) {
        unionRgnRect(removedRgn, &view->body.bounds);
        view->nextUpdate = 0;
        setViewScript(view, script, 0);
        view->flags = 0x4188000;
    }
}

/* @zoombi32 0x004506f0 */
void startRowViews()
{
    View *view;

    if (leftRow < 3) {
        view = findView(leftRowView);
        if (view) {
            view->flags = 0x4008000;
            setViewScript(view, 11006, 1);
        }
    }
    if (rightRow < 3) {
        view = findView(rightRowView);
        if (view) {
            view->flags = 0x4008000;
            setViewScript(view, 11007, 1);
        }
    }
}

/* @zoombi32 0x0045074d */
void stopRowViews()
{
    View *view;

    view = findView(leftRowView);
    if (view) {
        view->flags = 0x5188000;
        setViewScript(view, 11006, 0);
    }
    view = findView(rightRowView);
    if (view) {
        view->flags = 0x5188000;
        setViewScript(view, 11007, 0);
    }
}

/* @zoombi32 0x00450796 */
void startView11076()
{
    View *view = findView(view11076);

    if (view) {
        view->flags = 0x4008000;
        setViewScript(view, 11076, 1);
    }
}

/* @zoombi32 0x004507bb */
void stopView11076()
{
    View *view = findView(view11076);

    if (view) {
        view->flags = 0x5188000;
        setViewScript(view, 11076, 0);
    }
}

/* Whether any of the first partySize entries of waitingSnoids is set (not -1). */
/* @zoombi32 0x0044d102 */
short anyLeftToPlace()
{
    short any = 0;
    short i;

    for (i = 0; i < partySize; i++)
        if (waitingSnoids[i] != -1)
            any = 1;
    return any;
}

/* Resets a cell: state 500, no links. */
/* @zoombi32 0x0044d5ad */
void resetCell(short cell)
{
    short i;

    hexCells[cell].state = 500;
    for (i = 0; i <= 5; i++)
        hexCells[cell].links[i] = -1;
    cellLinkBits[cell] = 0;
}

/* Cuts a cell's link in one direction, clearing its bit. */
/* @zoombi32 0x0044dca0 */
void cutLink(short cell, short direction, short bit)
{
    hexCells[cell].links[direction] = -1;
    cellLinkBits[cell] |= bit;
    cellLinkBits[cell] ^= bit;
}

/* Counts the cells in state 508, and once there are partySize of them
   (the first time only), plays a sound. */
/* @zoombi32 0x0044e092 */
void checkAllFilled()
{
    short count = 0;
    short i;

    for (i = 0; i < 117; i++)
        if (hexCells[i].state == 508)
            count++;
    if (!slidesFidgetStarted && count >= partySize) {
        slidesMoves++;
        queueViewSound(randomBetween(20055, 20063), 0);
    }
}

/* Unlinks a cell from its neighbours (clearing the opposite direction's
   bit on each) and resets it. */
/* @zoombi32 0x0044e314 */
void unlinkCell(short cell)
{
    short i;
    short other;

    hexCells[cell].state = 500;
    for (i = 0; i <= 5; i++) {
        switch (i) {
        case 0:
            other = hexCells[cell].links[i];
            cellLinkBits[other] |= 8;
            cellLinkBits[other] ^= 8;
            break;
        case 1:
            other = hexCells[cell].links[i];
            cellLinkBits[other] |= 0x10;
            cellLinkBits[other] ^= 0x10;
            break;
        case 2:
            other = hexCells[cell].links[i];
            cellLinkBits[other] |= 0x20;
            cellLinkBits[other] ^= 0x20;
            break;
        case 5:
            other = hexCells[cell].links[i];
            cellLinkBits[other] |= 4;
            cellLinkBits[other] ^= 4;
            break;
        case 4:
            other = hexCells[cell].links[i];
            cellLinkBits[other] |= 2;
            cellLinkBits[other] ^= 2;
            break;
        case 3:
            other = hexCells[cell].links[i];
            cellLinkBits[other] |= 1;
            cellLinkBits[other] ^= 1;
            break;
        }
        hexCells[cell].links[i] = -1;
    }
    cellLinkBits[cell] = 0;
}

/* @zoombi32 0x00451238 */
void clearFeatureSlot(short n)
{
    featureSlots[n][0] = 0;
    featureSlots[n][1] = 0;
    featureSlots[n][2] = 0;
    featureSlots[n][3] = 0;
}

/* @zoombi32 0x00451276 */
void clearFeatureSlots()
{
    short i;

    for (i = 0; i < 8; i++) {
        featureSlots[i][0] = 0;
        featureSlots[i][1] = 0;
        featureSlots[i][2] = 0;
        featureSlots[i][3] = 0;
    }
}

/* Starts a Zoombini view's script (see startSnoidScript), moving it into
   `group`, with `notify` if given. */
/* @zoombi32 0x0045170a */
void startSmokeSnoidScript(short id, short script, short group, ViewNotify notify, char idleTicks)
{
    View *view = findView(id);

    if (view) {
        startSnoidScript(viewSnoid(view), script, 0, idleTicks);
        view->body.group = group;
        if (notify)
            view->notify = notify;
    }
}

/* Records the features of the Zoombini in view `id` as slot n's. */
/* @zoombi32 0x00450d00 */
void recordSlotFeatures(short id, short n)
{
    View *view = findView(id);

    if (view) {
        Snoid *snoid = viewSnoid(view);
        char *features = snoid->features;

        featureSlots[n][0] = features[0];
        featureSlots[n][1] = features[1];
        featureSlots[n][2] = features[2];
        featureSlots[n][3] = features[3];
    }
}

/* Deletes the temporary file (ZBtemp), if one was made. */
/* @zoombi32 0x00454f03 */
void deleteTempFile()
{
    fileSpec temp("ZBtemp");

    if (tempFileExists) {
        tempFileExists = 0;
        deleteFile(temp);
    }
}

/* Shuts the game down (once): offers to save a game in progress, then
   closes every scene and frees everything, layer by layer. */
/* @zoombi32 0x00454caa */
void shutDownGame()
{
    short i;

    if (shuttingDown)
        return;
    if (!isWindowed() && !practiceLevel && viewsReady && rosterReady && rosterChanged && !dialogFlags
        && currentScene >= 1 && currentScene <= 18) {
        quitRequested = 2;
        i = dialogFlags;
        showDialog(4, dialogTexts[32], dialogTexts[33], dialogTexts[34]);
        do {
            mainLoopUpdate();
            mainLoopEvents();
        } while (dialogFlags);
        if (i == dialogFlags) {
            if (quitRequested == 3)
                saveBeforeQuitting = 1;
            quitRequested = 0;
        }
        if (saveBeforeQuitting) {
            askSaveGame();
            do {
                mainLoopUpdate();
                mainLoopEvents();
            } while (dialogFlags);
        }
    }
    leavingGame = shuttingDown = 1;
    setFrameHook(0);
    setFreeAtOnce(1);
    stopMovie(1);
    unloadSounds();
    if (graphicsBufferSize())
        fadeOutViews();
    for (i = 0; i < 22; i++)
        if (scenes[i]->close)
            scenes[i]->close();
    for (i = 0; i < 6; i++)
        freeResource(&cursors[i]);
    if (rosterReady) {
        if (!practiceLevel)
            readWriteSavedGames(0, 1);
        deleteTempFile();
        if (viewsReady)
            closeViews();
        closeSnoids();
    }
    for (i = 0; i < 3; i++)
        freeFont(&fonts[i]);
    closeGraphics();
    leaveGameDirectory();
    if (soundBufferSize())
        closeSounds();
    if (iniBufferSize())
        closeAllIni();
    if (resourceBufferSize())
        closeResources();
    if (fileLayerVersion())
        closeFiles();
    if (memoryBufferSize())
        closeMemory();
    if (timerBufferSize())
        closeTimers();
    if (osVersion())
        osShutdown();
    if (instanceAtom)
        GlobalDeleteAtom(instanceAtom);
}

/* @zoombi32 0x00454c8e */
void quitSilently()
{
    leavingGame = 1;
    fatalError(0, 0);
}

/* Empties one of the Zoombini views (0 and 1, or 7 and 8): no features. */
/* @zoombi32 0x004511c1 */
void emptySlotView(short n)
{
    View *view;

    if (n == 0 || n == 1)
        view = findView(slotPairViews[n]);
    else if (n == 7 || n == 8)
        view = findView(rowViews[n]);
    if (view) {
        view->changed = 1;
        Snoid *snoid = viewSnoid(view);

        snoid->features[0] = 0;
        snoid->features[1] = 0;
        snoid->features[2] = 0;
        snoid->features[3] = 0;
        snoid->action = 4;
    }
}

/* Stops and empties the two Zoombini views comparedViews. */
/* @zoombi32 0x004512ac */
void emptyPairViews()
{
    ShortRect unused[1];
    short i;
    View *view;

    for (i = 0; i < 2; i++) {
        view = findView(comparedViews[i]);
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

/* A view's update: adds buttons 2 (when smokeGoReady changes) and 1 (the
   first time) to the region to redraw. */
/* @zoombi32 0x0044f180 */
void updateSmokeButtons(View *, short region)
{
    if (smokeGoReady) {
        if (!smokeButton2Lit) {
            smokeButton2Lit = 1;
            unionRgnRect(region, &smokeButtons[1].rect);
        }
    } else if (smokeButton2Lit) {
        smokeButton2Lit = 0;
        unionRgnRect(region, &smokeButtons[1].rect);
    }
    if (!smokeButton1Drawn) {
        smokeButton1Drawn = 1;
        unionRgnRect(region, &smokeButtons[0].rect);
    }
}

/* Closes the scene. */
/* @zoombi32 0x0044f1f2 */
void closeSmoke()
{
    if (smokeOpen) {
        smokeOpen = 0;
        short saved = setFreeAtOnce(1);

        clearViews();
        unloadSounds();
        freeResource(&smokeButtonResource);
        freeResource(&smokeImagesResource);
        freeResource(&smokeHotSpotsXResource);
        freeResource(&smokeHotSpotsYResource);
        setFreeAtOnce(saved);
        closeGameFile(&smokeFile);
        fadeOutViews();
        showBusyCursor();
    }
}

/* Scene 17's frame: leaves for the scene due; else runs the queued
   steps (view11017Due, anchorDue-leadersDue: the views to start and move, the
   next Zoombini across, crossedCount of crossingCount), and every 30 ticks
   while smokeFidgeting has an idle Zoombini of the party fidget. */
/* @zoombi32 0x0044f25d */
void smokeFrame()
{
    View *view;
    short i;
    short j;
    short done;

    if (inSmokeFrame || !smokeOpen)
        return;
    inSmokeFrame = 1;
    updateViews();
    if (sceneDue) {
        if (isSoundPlaying(996, RESOURCE_TYPE(0, 'S', 'N', 'D'))) {
            inSmokeFrame = 0;
            return;
        }
        if (!dialogQuestion || dialogQuestion == 3) {
            if (dialogQuestion == 3)
                chooseSnoids(0, 0);
            if (viewsLocked || !snoidsOnTheirWay || snoidsArrived >= 1) {
                pendingScene = sceneDue;
                sceneDue = 0;
                setCurrentMap(0);
                closeSmoke();
                inSmokeFrame = 0;
                return;
            }
        } else if (dialogQuestion == 2) {
            dialogQuestion = 0;
            sceneDue = 0;
        }
    }
    if (dialogFlags) {
        playAmbientSound();
        inSmokeFrame = 0;
        return;
    }
    if (view11017Due) {
        view11017Due = 0;
        view = findView(pairView1);
        if (view) {
            view->flags = 0x4188000;
            setViewScript(view, 11017, 1);
        }
    }
    if (anchorDue) {
        anchorDue = 0;
        moveView(view11036, 0, view11077);
        if (crossingSnoid)
            moveView(crossingSnoid, 1, view11036);
    }
    if (dealLightDue) {
        dealLightDue = 0;
        view = findView(dealerView);
        if (view) {
            setViewScript(view, dealerScript, 0);
            dealLit = 1;
            dealButtonState = 2;
            lightDealButton(11003);
            dealerRunning = 1;
        }
    }
    if (dealDimDue) {
        dealDimDue = 0;
        if (smokeLevel == 1 || smokeLevel == 2) {
            dealLit = 0;
            dimDealButton(11002);
        } else if (smokeLevel == 3) {
            dealLit = 1;
            dealButtonState = 2;
            lightDealButton(11003);
        }
    }
    if (crossedDue) {
        crossedDue = 0;
        crossedMarkers[crossedCount] = addView(0x4180000, drawCels, runViewScript, 11071 - crossedCount, 6, 0, 0, 0);
        if (!pairMismatch && crossingSnoid) {
            crossedSnoids[crossedCount] = crossingSnoid;
            view = findView(crossingSnoid);
            if (view) {
                Snoid *snoid;

                view->flags = 0x4000001;
                snoid = viewSnoid(view);
                snoid->chosen = 1;
                if (crossedChosen > 2)
                    startSnoidScript(viewSnoid(view), snoid->features[3] + 12044, 0, 0);
            }
            crossedChosen++;
            if (crossedChosen == 1) {
                smokeGoReady = 1;
                unionRgnRect(removedRgn, &smokeButtons[1].rect);
            }
        }
        if (!crossedCount) {
            moveView(crossedMarkers[crossedCount], 0, view11077);
            moveView(crossingSnoid, 1, crossedMarkers[crossedCount]);
        } else {
            moveView(crossedMarkers[crossedCount], 0, crossedMarkers[crossedCount - 1]);
            moveView(crossingSnoid, 1, crossedMarkers[crossedCount]);
        }
        crossedCount++;
        if (crossedCount == crossingCount) {
            short chosen = countChosenSnoids();

            if (chosen == crossingCount) {
                smokeFidgeting = 1;
                queueViewSound(randomBetween(20055, 20063), 0);
            } else if (chosen < crossingCount) {
                if (randomBetween(0, 4) > smokeLevel - 1
                    || (*(short *)(gameState + 0x42) & 0xfff) <= 3)
                    queueViewSound(randomBetween(20045, 20048), 0);
            }
        }
        crossingSnoid = 0;
        if (smokeLevel == 1 || smokeLevel == 2) {
            view = findView(moverView1);
            if (view) {
                ViewCel *cels = view->body.cels;

                cels[0].image = 0;
                view->flags = 0x4108000;
                setViewScript(view, 11032, 1);
            }
        } else {
            nextCrossing++;
            if (useSecondMover)
                view = findView(moverView2);
            else
                view = findView(moverView1);
            if (view) {
                setViewScript(view, moverScripts[2], 1);
                view->notify = smokeViewNotify;
            }
        }
        if (smokeLevel == 1 || smokeLevel == 2)
            randomDragSlot = 0;
        if (smokeLevel < 4 || smokeLevel == 4 && level4Stage == 1) {
            if (smokeLevel == 4 || smokeLevel == 3)
                featuresTaken = takeNextTwoFeatures();
            else
                featuresTaken = takeRandomFeatures();
            if (featuresTaken && crossedCount <= crossingCount && smokeLevel < 4) {
                dealButtonState = 1;
                lightDealButton(11005);
            } else {
                dealButtonState = 0;
                dimDealButton(11004);
            }
        }
    }
    if (leadersDue) {
        leadersDue = 0;
        for (i = 0; i < 3; i++) {
            view = findView(crossedMarkers[i]);
            if (view) {
                setViewScript(view, i + 11072, 1);
                view->notify = smokeViewNotify;
                if (!i) {
                    groupViews(crossedMarkers[i], crossedMarkers[i], 0, 0, 0, 0);
                    leaderGroup = view->body.group;
                } else
                    view->body.group = leaderGroup;
                view = findView(crossedSnoids[i]);
                if (view) {
                    startSnoidScript(viewSnoid(view), i + 12041, 0, 0);
                    view->body.group = leaderGroup;
                }
            }
        }
    }
    if (smokeFidgeting && smokeFidgets < crossingCount - 1) {
        if (clockTime() - lastSmokeFidgetTime > 30) {
            done = 0;
            lastSmokeFidgetTime = clockTime();
            for (j = 0; j < crossingCount && !done; j++) {
                short n = allocateSlot(&smokeFidgetersUsed, crossingCount - 1, 0);

                if (partyViews[n] && partyViews[n] != crossedSnoids[0] && partyViews[n] != crossedSnoids[1]
                    && partyViews[n] != crossedSnoids[2]) {
                    view = idleSnoidView(partyViews[n]);
                    if (view && view->body.running) {
                        Snoid *snoid = viewSnoid(view);

                        startSnoidScript(viewSnoid(view), snoid->features[3] + 12044, 0, 0);
                        if (!smokeFidgets)
                            smokeFidgets = 4;
                        else
                            smokeFidgets++;
                        done = 1;
                    }
                } else if (crossingCount < 5) {
                    view = idleSnoidView(partyViews[3]);
                    if (view && view->body.running) {
                        Snoid *snoid = viewSnoid(view);

                        startSnoidScript(viewSnoid(view), snoid->features[3] + 12044, 0, 0);
                    }
                    smokeFidgets = 4;
                    done = 1;
                }
            }
        }
    } else if (smokeFidgets >= crossingCount - 1)
        smokeFidgets = smokeFidgeting = lastSmokeFidgetTime = smokeFidgetersUsed = 0;
    playAmbientSound();
    inSmokeFrame = 0;
}

/* A view's drawing: its cels from the bank smokeImages, while it runs and
   stands in the game's area. */
/* Functional: the original reads each cel's words as it pushes them
   (`drawImageData(image(*cel++), *cel++, *cel++, 8)`, relying on BCC's
   left-to-right evaluation); this reads them first. */
/* @zoombi32-functional 0x004541bf */
void drawSmokeSnoid(View *view)
{
    if (view->body.running && ptInRect(&gameRect, *(Point *)&view->body.x)) {
        short *cel = (short *)view->body.cels;
        ImageBank *bank = smokeImages;

        while (*cel && *cel <= bank->count) {
            unsigned short *image = (unsigned short *)(bank->offsets[*cel++] + (char *)bank);
            short x = *cel++;
            short y = *cel++;

            drawImageData(image, x, y, 8);
        }
    }
}

/* The scene to go back to: the current one if it is the town, the camp, a
   waiting place or a puzzle with a party (setting skipJourneyMap), else 3. */
/* @zoombi32 0x00454c10 */
short sceneToReturnTo()
{
    short scene;

    if (savedScene() == 3 || savedScene() == 4 || savedScene() == 5 || savedScene() == 6 || savedScene() == 1
        || savedScene() >= 7 && savedScene() <= 18 && party()->count > 0) {
        scene = savedScene();
        skipJourneyMap = 1;
    } else {
        scene = 3;
    }
    return scene;
}

/* Stands each Zoombini on a cell in state 508 at its cell. */
/* @zoombi32 0x0044e0e2 */
void standFilledCells()
{
    Point where;
    short i;
    View *view;

    slidesMoves = 0;
    for (i = 0; i < 117; i++)
        if (hexCells[i].state == 508) {
            view = findView(hexCells[i].view);
            where.x = view->body.x + 24;
            where.y = view->body.y - 5;
            setSnoidAction((Snoid *)&findView(hexCells[i].snoid)->body, 0, &where);
        }
}

/* Stands the placed Zoombinis (placedSnoids) on their cells (state 507). */
/* @zoombi32 0x0044e161 */
void standPlacedSnoids()
{
    Point where;
    short i;
    short cell;
    View *view;

    for (i = 0; i < partySize; i++)
        if (placedSnoids[i].cell) {
            cell = placedSnoids[i].cell;
            view = findView(hexCells[placedSnoids[i].cell].view);
            where.x = view->body.x + 24;
            where.y = view->body.y - 5;
            view = findView(placedSnoids[i].snoid);
            if (view) {
                setSnoidAction(viewSnoid(view), 0, &where);
                hexCells[cell].state = 507;
                hexCells[cell].snoid = placedSnoids[i].snoid;
            }
        }
}

/* Loads a QuickTime movie from a file (0: failed). */
/* @zoombi32 0x004552fd */
LONG_PTR loadMovie(const char *path)
{
    LONG_PTR movie;
    long id;
    LONG_PTR file;
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
void stopMovie(short shutdown)
{
    if (movieShowing) {
        introPending = 1;
        movieShowing = 0;
        qtim_31(currentMovie, 0);
        qtim_07(currentMovie);
        setPort(portBeforeMovie);
    }
    if (shutdown && quickTimeReady) {
        quickTimeReady = 0;
        if (movieController) {
            qtim_37(movieController);
            movieController = 0;
        }
        qtim_0c();
        QTTerminate();
    }
}

/* Copies the features of the Zoombini in view `id` to one of the slot
   views (0 and 1, or 7 and 8); for any but slot 0, while smokeLevel is
   below 3, the Zoombini's view also stops and moves to the slot view's
   place in the list (moveView). */
/* @zoombi32 0x00450c24 */
void copyToSlotView(short id, short n)
{
    View *to;
    char *features;
    View *from;

    from = findView(id);
    Snoid *fromSnoid = viewSnoid(from);
    features = fromSnoid->features;
    switch (n) {
    case 0:
        to = findView(slotPairViews[0]);
        break;
    case 1:
        to = findView(slotPairViews[1]);
        break;
    case 7:
        to = findView(rowViews[7]);
        break;
    case 8:
        to = findView(rowViews[8]);
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
        snoid->action = 4;
        if (n && smokeLevel < 3) {
            from->changed = 1;
            from->body.running = 0;
            moveView(from->id, 0, to->id);
        }
    }
}

/* Records the features of the Zoombinis in the views slotViews[0-2] as
   slots 1-3's (none if a view is gone). */
/* @zoombi32 0x00450d5d */
void recordLeftSlots()
{
    short i;
    View *view;

    for (i = 1; i < 4; i++) {
        view = findView(slotViews[i - 1]);
        if (view) {
            Snoid *snoid = viewSnoid(view);

            featureSlots[i][0] = snoid->features[0];
            featureSlots[i][1] = snoid->features[1];
            featureSlots[i][2] = snoid->features[2];
            featureSlots[i][3] = snoid->features[3];
        } else {
            featureSlots[i][0] = 0;
            featureSlots[i][1] = 0;
            featureSlots[i][2] = 0;
            featureSlots[i][3] = 0;
        }
    }
}

/* The same for slots 4-6. */
/* @zoombi32 0x00450df2 */
void recordRightSlots()
{
    short i;
    View *view;

    for (i = 4; i < 7; i++) {
        view = findView(slotViews[i - 1]);
        if (view) {
            Snoid *snoid = viewSnoid(view);

            featureSlots[i][0] = snoid->features[0];
            featureSlots[i][1] = snoid->features[1];
            featureSlots[i][2] = snoid->features[2];
            featureSlots[i][3] = snoid->features[3];
        } else {
            featureSlots[i][0] = 0;
            featureSlots[i][1] = 0;
            featureSlots[i][2] = 0;
            featureSlots[i][3] = 0;
        }
    }
}

/* Copies the features of the Zoombinis in the next two views of
   crossingViews (from nextCrossing, up to crossingCount) into targetFeatures; returns how many
   there were. */
/* @zoombi32 0x00452035 */
short takeNextTwoFeatures()
{
    short count = 0;
    View *view;

    if (nextCrossing < crossingCount) {
        count++;
        view = findView(crossingViews[nextCrossing]);
        if (view) {
            Snoid *snoid = viewSnoid(view);
            char *features = snoid->features;

            targetFeatures[0] = features[0];
            targetFeatures[1] = features[1];
            targetFeatures[2] = features[2];
            targetFeatures[3] = features[3];
        }
        if (nextCrossing + 1 < crossingCount) {
            count++;
            view = findView(crossingViews[nextCrossing + 1]);
            if (view) {
                Snoid *snoid = viewSnoid(view);
                char *features = snoid->features;

                targetFeatures[4] = features[0];
                targetFeatures[5] = features[1];
                targetFeatures[6] = features[2];
                targetFeatures[7] = features[3];
            }
        } else {
            targetFeatures[4] = 0;
            targetFeatures[5] = 0;
            targetFeatures[6] = 0;
            targetFeatures[7] = 0;
        }
    }
    return count;
}

/* Starts view view11036's script (11036 on, by crossedCount) and, with it, the
   Zoombini in view crossingSnoid (script 12020 on, from crossingStart), grouping
   them. */
/* @zoombi32 0x0045162e */
void startNextCrossing(short)
{
    View *view;
    View *other;

    view = findView(view11036);
    if (view) {
        setViewScript(view, crossedCount + 11036, 1);
        view->notify = smokeViewNotify;
        moveView(view->id, 0, view11008);
    }
    other = findView(crossingSnoid);
    if (other && view) {
        Snoid *snoid = viewSnoid(other);

        snoid->facingLeft = 1;
        *(Point *)&snoid->body.x = crossingStart;
        startSnoidScript(viewSnoid(other), crossedCount + 12020, 0, 0);
        other->notify = smokeViewNotify;
        moveView(other->id, 1, view->id);
    }
    if (other)
        groupViews(other->id, view->id, 0, 0, 0, 0);
    else
        groupViews(view->id, view->id, 0, 0, 0, 0);
}

/* Lays out the hex grid for level stoneRiseLevel (0-3), with the start cells
   (state startState, 504 or at level 3 perhaps 505), the cells open (506)
   and blocked (501), the Zoombinis already placed (507) and the cells'
   link bits (cellLinkBits); notes the open cells (listedCount of them, where to
   show them in listedCellPlaces), then adds every cell's view and the rows'
   views, and puts the party behind them. */
/* @zoombi32 0x0044b550 */
void layOutGrid()
{
    short row;
    short k;
    short base;
    short step;
    short placed;
    short extra;
    short unused[2]; /* only takes stack space */
    short cell;
    short count;
    short found;
    View *view;

    fillMemory(placedSnoids, 0, 4);
    startState = 504;
    linkImageOffset = 48;
    if (stoneRiseLevel == 3) {
        startState = randomUpTo(1) + 504;
        if (startState != 504)
            linkImageOffset = 24;
    }
    k = 0;
    readPartyFeatures();
    switch (stoneRiseLevel) {
    case 0:
        pairByFeatures();
        base = rowFirstCells[groupCount];
        step = rowSteps[groupCount];
        for (row = 0; row < groupCount; row++) {
            extra = 0;
            cell = row * step + base;
            if (groupCount > 7 && cell % 2) {
                short c;

                hexCells[cell].state = startState;
                cellLinkBits[cell] = 16;
                hexCells[cell].links[4] = cell + 1;
                for (c = cell + 1; c < cell + 4; c++) {
                    hexCells[c].state = startState;
                    cellLinkBits[c] = 18;
                    hexCells[c].links[4] = c + 1;
                    hexCells[c].links[1] = c - 1;
                }
                extra = 3;
            }
            hexCells[cell + extra].state = startState;
            hexCells[cell + extra + 1].state = 506;
            listedCount++;
            listedCellPlaces[listedCount - 1] = cellPoints[cell + extra + 1];
            listedCells[listedCount] = extra + cell + 1;
            listedCellPlaces[listedCount - 1].x += 24;
            listedCellPlaces[listedCount - 1].y -= 5;
            if (pairFeatures[k] != 501) {
                hexCells[cell + extra + 2].snoid = pairFeatures[k];
                hexCells[cell + extra + 2].state = 501;
                hexCells[cell + extra + 3].state = 506;
                listedCount++;
                listedCellPlaces[listedCount - 1] = cellPoints[cell + extra + 3];
                listedCells[listedCount] = extra + cell + 3;
                listedCellPlaces[listedCount - 1].x += 24;
                listedCellPlaces[listedCount - 1].y -= 5;
                cellLinkBits[cell + extra] |= 16;
                cellLinkBits[cell + extra + 1] = 18;
                cellLinkBits[cell + extra + 2] = 18;
                cellLinkBits[cell + extra + 3] |= 2;
                k++;
            } else {
                cellLinkBits[cell + extra] |= 16;
                cellLinkBits[cell + extra + 1] |= 2;
                k++;
            }
        }
        linkCells();
        break;
    case 1:
        groupInThrees();
        base = rowFirstCells[groupCount];
        step = rowSteps[groupCount];
        placed = 0;
        for (row = 0; row < groupCount; row++) {
            cell = row * step + base;
            hexCells[cell].state = startState;
            hexCells[cell + 1].state = 506;
            listedCount++;
            listedCellPlaces[listedCount - 1] = cellPoints[cell + 1];
            listedCells[listedCount] = cell + 1;
            listedCellPlaces[listedCount - 1].x += 24;
            listedCellPlaces[listedCount - 1].y -= 5;
            if (++placed >= partySize) {
                cellLinkBits[cell] = 16;
                cellLinkBits[cell + 1] = 2;
                break;
            }
            if (pairFeatures[k] && pairFeatures[k] != 501) {
                hexCells[cell + 2].snoid = pairFeatures[k];
                hexCells[cell + 2].state = 501;
                hexCells[cell + 3].state = 506;
                listedCount++;
                listedCellPlaces[listedCount - 1] = cellPoints[cell + 3];
                listedCells[listedCount] = cell + 3;
                listedCellPlaces[listedCount - 1].x += 24;
                listedCellPlaces[listedCount - 1].y -= 5;
                cellLinkBits[cell] |= 16;
                cellLinkBits[cell + 1] = 18;
                cellLinkBits[cell + 2] = 18;
                cellLinkBits[cell + 3] |= 2;
                k++;
                if (++placed >= partySize)
                    break;
            } else if (pairFeatures[k]) {
                hexCells[cell + 2].snoid = 0;
                hexCells[cell + 2].state = 501;
                hexCells[cell + 3].state = 506;
                listedCount++;
                listedCellPlaces[listedCount - 1] = cellPoints[cell + 3];
                listedCells[listedCount] = cell + 3;
                listedCellPlaces[listedCount - 1].x += 24;
                listedCellPlaces[listedCount - 1].y -= 5;
                cellLinkBits[cell] |= 16;
                cellLinkBits[cell + 1] = 18;
                cellLinkBits[cell + 2] = 18;
                cellLinkBits[cell + 3] |= 2;
                k++;
                if (++placed >= partySize)
                    break;
            }
            if (pairFeatures[k] && pairFeatures[k] != 501) {
                hexCells[cell + 4].snoid = pairFeatures[k];
                hexCells[cell + 4].state = 501;
                hexCells[cell + 5].state = 506;
                listedCount++;
                listedCellPlaces[listedCount - 1] = cellPoints[cell + 5];
                listedCells[listedCount] = cell + 5;
                listedCellPlaces[listedCount - 1].x += 24;
                listedCellPlaces[listedCount - 1].y -= 5;
                cellLinkBits[cell + 3] |= 16;
                cellLinkBits[cell + 4] = 18;
                cellLinkBits[cell + 5] = 2;
                k++;
                if (++placed >= partySize)
                    break;
            } else if (pairFeatures[k]) {
                hexCells[cell + 4].snoid = 0;
                hexCells[cell + 4].state = 501;
                hexCells[cell + 5].state = 506;
                listedCount++;
                listedCellPlaces[listedCount - 1] = cellPoints[cell + 5];
                listedCells[listedCount] = cell + 5;
                listedCellPlaces[listedCount - 1].x += 24;
                listedCellPlaces[listedCount - 1].y -= 5;
                cellLinkBits[cell + 3] |= 16;
                cellLinkBits[cell + 4] = 18;
                cellLinkBits[cell + 5] = 2;
                k++;
                if (++placed >= partySize)
                    break;
            }
        }
        linkCells();
        break;
    case 2:
        for (cell = 0; cell < 3; cell++) {
            hexCells[level2StartCells[cell]].state = startState;
            cellLinkBits[level2StartCells[cell]] = 16;
            cellLinkBits[level2StartCells[cell] + 1] = 42;
            cellLinkBits[level2StartCells[cell] - 8] |= 4;
            cellLinkBits[level2StartCells[cell] + 10] |= 1;
        }
        for (cell = 0; cell < 3; cell++) {
            hexCells[level2Links5Cells[cell]].state = 506;
            cellLinkBits[level2Links5Cells[cell]] = 5;
            cellLinkBits[level2Links5Cells[cell] - 10] |= 8;
            cellLinkBits[level2Links5Cells[cell] + 8] |= 32;
        }
        for (cell = 0; cell < 18; cell++) {
            hexCells[level2BlockedCells[cell]].state = 501;
            hexCells[level2OpenCells[cell]].state = 506;
        }
        for (cell = 0; cell < 12; cell++) {
            cellLinkBits[level2Links18Cells[cell]] |= 18;
            cellLinkBits[level2Links18Cells[cell] - 1] |= 16;
            cellLinkBits[level2Links18Cells[cell] + 1] |= 2;
        }
        linkCells();
        orderPartyByAlike();
        if (randomUpTo(100) < 50) {
            seatPair(19);
            seatPair(55);
            seatPair(91);
        } else if (randomUpTo(100) < 50) {
            seatPair(55);
            seatPair(91);
            seatPair(19);
        } else {
            seatPair(91);
            seatPair(19);
            seatPair(55);
        }
        cell = sharedStone(11, 13);
        if (cell)
            hexCells[12].snoid = cell;
        cell = sharedStone(29, 31);
        if (cell)
            hexCells[30].snoid = cell;
        cell = sharedStone(47, 49);
        if (cell)
            hexCells[48].snoid = cell;
        cell = sharedStone(65, 67);
        if (cell)
            hexCells[66].snoid = cell;
        cell = sharedStone(83, 85);
        if (cell)
            hexCells[84].snoid = cell;
        cell = sharedStone(101, 103);
        if (cell)
            hexCells[102].snoid = cell;
        count = 0;
        for (cell = 0; cell < 18; cell++)
            if (hexCells[level2OpenCells[cell]].state == 507)
                count++;
        if (count <= partySize) {
            count = partySize - count;
            do {
                if (count) {
                    count--;
                    for (cell = 0; cell < 18; cell++)
                        if (hexCells[level2OpenCells[cell]].state != 507) {
                            hexCells[level2OpenCells[cell]].state = 507;
                            break;
                        }
                }
            } while (count);
        } else {
            count -= partySize;
            do {
                found = 0;
                for (cell = 0; cell < 6; cell++)
                    if (hexCells[level2OpenCells[cell]].state == 507 && hexCells[level2OpenCells[cell] - 1].snoid == -1) {
                        hexCells[level2OpenCells[cell]].state = 501;
                        hexCells[level2OpenCells[cell]].snoid = -1;
                        count--;
                        found++;
                        break;
                    }
                if (!found)
                    for (cell = 6; cell < 12; cell++)
                        if (hexCells[level2OpenCells[cell]].state == 507
                            && hexCells[level2OpenCells[cell] + 1].snoid == -1) {
                            hexCells[level2OpenCells[cell]].state = 501;
                            hexCells[level2OpenCells[cell]].snoid = -1;
                            count--;
                            found++;
                            break;
                        }
                if (!found)
                    for (cell = 12; cell < 15; cell++)
                        if (hexCells[level2OpenCells[cell]].state == 507
                            && hexCells[level2OpenCells[cell] - 10].snoid == -1
                            && hexCells[level2OpenCells[cell] + 8].snoid == -1) {
                            hexCells[level2OpenCells[cell]].state = 501;
                            hexCells[level2OpenCells[cell]].snoid = -1;
                            count--;
                            found++;
                            break;
                        }
                if (!found)
                    count = 0;
            } while (count);
        }
        for (cell = 0; cell < 117; cell++)
            if (hexCells[cell].state == 506) {
                hexCells[cell].state = 501;
                hexCells[cell].snoid = -1;
            }
        listedCount = 0;
        for (cell = 0; cell < 18; cell++)
            if (hexCells[level2OpenCells[cell]].state == 507) {
                listedCount++;
                listedCellPlaces[listedCount - 1] = cellPoints[level2OpenCells[cell]];
                listedCells[listedCount] = level2OpenCells[cell];
                listedCellPlaces[listedCount - 1].x += 24;
                listedCellPlaces[listedCount - 1].y -= 5;
            }
        break;
    case 3:
        for (cell = 0; cell < 43; cell++)
            hexCells[level3BlockedCells[cell]].state = 501;
        for (cell = 0; cell < 20; cell++)
            cellLinkBits[level3Links36Cells[cell]] = 36;
        for (cell = 0; cell < 20; cell++)
            cellLinkBits[level3Links9Cells[cell]] = 9;
        cellLinkBits[54] = 16;
        hexCells[54].state = startState;
        cellLinkBits[55] = 58;
        cellLinkBits[57] = cellLinkBits[59] = 63;
        cellLinkBits[56] = cellLinkBits[58] = cellLinkBits[60] = 18;
        cellLinkBits[38] = cellLinkBits[40] = cellLinkBits[42] = cellLinkBits[21] = cellLinkBits[74] = cellLinkBits[76] = cellLinkBits[78]
            = cellLinkBits[93] = cellLinkBits[23] = cellLinkBits[95] = 45;
        cellLinkBits[19] = cellLinkBits[91] = 40;
        cellLinkBits[44] = cellLinkBits[80] = 5;
        cellLinkBits[2] = cellLinkBits[4] = cellLinkBits[25] = cellLinkBits[6] = 12;
        cellLinkBits[110] = cellLinkBits[112] = cellLinkBits[97] = cellLinkBits[114] = 33;
        cellLinkBits[61] = 47;
        cellLinkBits[25] = 13;
        cellLinkBits[97] = 37;
        for (cell = 0; cell < 26; cell++)
            hexCells[level3OpenCells[cell]].state = 506;
        linkCells();
        if (partySize > 5)
            startGrid(54);
        else {
            orderPartyByAlike();
            for (cell = 0; cell < 117; cell++) {
                cellLinkBits[cell] = 0;
                hexCells[cell].state = 500;
            }
            hexCells[54].state = startState;
            hexCells[55].state = 507;
            hexCells[55].snoid = partyViews[0];
            cellLinkBits[54] = 16;
            cellLinkBits[55] = 2;
            switch (partySize) {
            case 2:
                hexCells[56].state = 501;
                hexCells[57].state = 507;
                hexCells[57].snoid = partyViews[1];
                if (shareFeature(waitingSnoids[0], waitingSnoids[1]))
                    hexCells[56].snoid = sharedFeature + 510;
                cellLinkBits[55] = 18;
                cellLinkBits[56] = 18;
                cellLinkBits[57] = 2;
                waitingSnoids[0] = waitingSnoids[1] = -1;
                break;
            case 3:
                hexCells[56].state = 501;
                hexCells[57].state = 507;
                hexCells[57].snoid = partyViews[1];
                hexCells[58].state = 501;
                hexCells[59].state = 507;
                hexCells[59].snoid = partyViews[2];
                if (shareFeature(waitingSnoids[0], waitingSnoids[1]))
                    hexCells[56].snoid = sharedFeature + 510;
                if (shareFeature(waitingSnoids[1], waitingSnoids[2]))
                    hexCells[58].snoid = sharedFeature + 510;
                cellLinkBits[55] = 18;
                cellLinkBits[56] = 18;
                cellLinkBits[57] = 18;
                cellLinkBits[58] = 18;
                cellLinkBits[59] = 2;
                waitingSnoids[0] = waitingSnoids[1] = waitingSnoids[2] = -1;
                break;
            case 4:
                hexCells[56].state = 501;
                hexCells[57].state = 507;
                hexCells[57].snoid = partyViews[1];
                hexCells[58].state = 501;
                hexCells[59].state = 507;
                hexCells[59].snoid = partyViews[2];
                hexCells[60].state = 501;
                hexCells[61].state = 507;
                hexCells[61].snoid = partyViews[3];
                if (shareFeature(waitingSnoids[0], waitingSnoids[1]))
                    hexCells[56].snoid = sharedFeature + 510;
                if (shareFeature(waitingSnoids[1], waitingSnoids[2]))
                    hexCells[58].snoid = sharedFeature + 510;
                if (shareFeature(waitingSnoids[2], waitingSnoids[3]))
                    hexCells[60].snoid = sharedFeature + 510;
                cellLinkBits[55] = 18;
                cellLinkBits[56] = 18;
                cellLinkBits[57] = 18;
                cellLinkBits[58] = 18;
                cellLinkBits[59] = 18;
                cellLinkBits[60] = 18;
                cellLinkBits[61] = 2;
                waitingSnoids[0] = waitingSnoids[1] = -1;
                waitingSnoids[2] = waitingSnoids[3] = -1;
                break;
            case 5:
                hexCells[56].state = 501;
                hexCells[57].state = 507;
                hexCells[57].snoid = partyViews[1];
                hexCells[58].state = 501;
                hexCells[59].state = 507;
                hexCells[59].snoid = partyViews[2];
                hexCells[60].state = 501;
                hexCells[61].state = 507;
                hexCells[61].snoid = partyViews[3];
                hexCells[52].state = 501;
                hexCells[44].state = 507;
                hexCells[44].snoid = partyViews[4];
                if (shareFeature(waitingSnoids[0], waitingSnoids[1]))
                    hexCells[56].snoid = sharedFeature + 510;
                if (shareFeature(waitingSnoids[1], waitingSnoids[2]))
                    hexCells[58].snoid = sharedFeature + 510;
                if (shareFeature(waitingSnoids[2], waitingSnoids[3]))
                    hexCells[60].snoid = sharedFeature + 510;
                if (shareFeature(waitingSnoids[3], waitingSnoids[4]))
                    hexCells[52].snoid = sharedFeature + 510;
                cellLinkBits[55] = 18;
                cellLinkBits[56] = 18;
                cellLinkBits[57] = 18;
                cellLinkBits[58] = 18;
                cellLinkBits[59] = 18;
                cellLinkBits[60] = 18;
                cellLinkBits[61] = 34;
                cellLinkBits[52] = 36;
                cellLinkBits[44] = 4;
                waitingSnoids[0] = waitingSnoids[1] = -1;
                waitingSnoids[2] = waitingSnoids[3] = -1;
                waitingSnoids[4] = -1;
                break;
            }
        }
        if (anyLeftToPlace())
            fillFreeCells();
        settleCells();
        if (partySize > 5) {
            clearLine(83, 84, 93);
            clearLine(29, 30, 21);
            clearLine(102, 103, 112);
            clearLine(12, 13, 4);
            clearLine(85, 86, 95);
            clearLine(31, 32, 23);
            clearLine(68, 69, 78);
            clearLine(50, 51, 42);
            clearLine(66, 67, 76);
            clearLine(104, 105, 114);
            clearLine(14, 15, 6);
            clearLine(34, 52, 44);
            clearLine(70, 88, 80);
        }
        if (hexCells[60].state == 501 && hexCells[61].state == 501 && partySize > 5
            && hexCells[69].state == 500 && hexCells[51].state == 500 && hexCells[52].state == 500
            && hexCells[70].state == 500) {
            unlinkCell(60);
            unlinkCell(61);
        }
        if (partySize <= 2) {
            fillMemory(cellLinkBits, 0, 234);
            fillMemory(hexCells, 0, 2106);
            for (cell = 0; cell < 117; cell++)
                hexCells[cell].state = 500;
            hexCells[54].state = startState;
            cellLinkBits[54] = 16;
            hexCells[54].state = startState;
            hexCells[55].snoid = partyViews[0];
            cellLinkBits[55] = 2;
            hexCells[55].state = 507;
            hexCells[54].links[4] = 55;
            hexCells[55].links[1] = 54;
            if (partySize == 2) {
                hexCells[57].snoid = partyViews[1];
                hexCells[55].links[4] = 56;
                hexCells[56].links[4] = 57;
                hexCells[56].links[1] = 55;
                hexCells[57].links[1] = 56;
                cellLinkBits[55] &= 16;
                cellLinkBits[56] = 18;
                cellLinkBits[57] = 2;
                hexCells[57].state = 507;
                hexCells[57].snoid = partyViews[1];
                hexCells[56].state = 501;
                waitingSnoids[0] = -1;
                waitingSnoids[1] = 1;
                placeAlike(55, 4);
            }
        }
        count = 0;
        for (cell = 0; cell < 26; cell++) {
            row = level3OpenCells[cell];
            if (hexCells[row].state == 507) {
                listedCount++;
                listedCellPlaces[listedCount - 1] = cellPoints[row];
                listedCells[listedCount] = row;
                listedCellPlaces[listedCount - 1].x += 24;
                listedCellPlaces[listedCount - 1].y -= 5;
                placedSnoids[count].cell = row;
                placedSnoids[count].snoid = hexCells[row].snoid;
                count++;
            }
        }
        break;
    }
    for (cell = 0; cell < 117; cell++) {
        if (hexCells[cell].state == 507)
            hexCells[cell].state = 506;
        if (hexCells[cell].state == 500)
            hexCells[cell].view = addView(0x988000, drawCels, runViewScript, 7001, 6, &cellPoints[cell], 0, 0);
        else if (hexCells[cell].state == startState) {
            hexCells[cell].view = addView(0x988000, drawCels, runViewScript, 7000, 6, &cellPoints[cell], 0, 0);
            view = findView(hexCells[cell].view);
            view->placed = placeCellViewImages;
            startCellCount++;
            startCells[startCellCount - 1] = cell;
            if (startState == 505)
                startCells[startCellCount - 1] += 20;
            if (startCellCount == 1)
                startCellsGroup = groupViews(hexCells[cell].view, hexCells[cell].view, 0, 0, 0, 0);
        } else {
            hexCells[cell].view = addView(0x988000, drawCels, runViewScript, 7000, 6, &cellPoints[cell], 0, 0);
            view = findView(hexCells[cell].view);
            view->placed = placeCellViewImages;
        }
    }
    for (cell = 1; cell < 9; cell++)
        slidesRowViews[cell] = addView(0x188000, drawCels, runViewScript, cell + 7003, 6, 0, 0, 0);
    view = findView(partyViews[0]);
    savedPartyFlags = (short)view->flags;
    for (cell = partySize - 1; cell >= 0; cell--) {
        view = findView(partyViews[cell]);
        moveView(partyViews[cell], 0, slidesRowViews[8]);
        view->flags |= 0x4008000;
    }
}

/* Starts the hex grid at the cell after `cell`: puts the first of the
   party on it (the next one alike to the one before it), then grows the
   grid from there (growGrid) and from the 13 cells of `order` while
   anyLeftToPlace allows. `cell` is reused for the search and growGrid's
   result, as in the original. */
/* @zoombi32 0x0044cc51 */
void startGrid(short cell)
{
    short order[13] = {55, 40, 76, 23, 95, 42, 78, 38, 74, 21, 93, 19, 91};
    short i = 0;
    short at;

    orderPartyByAlike();
    at = ++cell;
    hexCells[at].state = 507;
    if (partySize == 1) {
        hexCells[at].snoid = partyViews[0];
        placedSnoids[0].snoid = partyViews[0];
        waitingSnoids[0] = -1;
        return;
    }
    for (cell = 0; cell < partySize; cell++)
        if (shareFeature(cell, cell + 1)) {
            hexCells[at].snoid = partyViews[waitingSnoids[cell + 1]];
            waitingSnoids[cell + 1] = -1;
            break;
        }
    cell = growGrid(at);
    if (!cell) {
        at += 2;
        cell = growGrid(at);
        if (!cell) {
            at += 2;
            growGrid(at);
        }
    } else if (cell && cell != -1)
        growGrid(cell);
    do {
        if (++i >= 13)
            break;
        at = order[i];
        growGrid(at);
    } while (anyLeftToPlace());
}

/* Grows the grid from `cell`: when it isn't taken (507), first from a
   taken cell two steps away in direction 5 or 3; then (for cells 55, 57
   and 59) in direction 4, and towards the cells two steps away in
   directions 5 and 3 when they are open (506), linking the Zoombini two
   cells on (sharedStone). Returns -1 when it can't, the cell two on when a
   step fails, else 0. */
/* Not exact: the original keeps `n` in edi (and far5 and far3 on the
   stack); here n stays in eax and far5 gets edi. */
/* @zoombi32 0x0044ce56 */
short growGrid(short cell)
{
    short n;
    short far5; /* two steps away in direction 5 */
    short far3; /* and in direction 3 */
    short target;

    if (!anyLeftToPlace())
        return -1;
    if (hexCells[cell].state != 507) {
        short a = hexCells[cell].links[5];
        short b;

        a = hexCells[a].links[5];
        b = hexCells[cell].links[3];
        b = hexCells[b].links[3];

        if (a != -1 && hexCells[a].state == 507) {
            n = placeAlike(a, 2);
            if (n == -1)
                return -1;
        } else if (b != -1 && hexCells[b].state == 507) {
            n = placeAlike(b, 0);
            if (n == -1)
                return -1;
        } else
            return -1;
    }
    {
        short next = hexCells[cell].links[5];

        far5 = -1;
        if (next != -1)
            far5 = hexCells[next].links[5];
    }
    {
        short next = hexCells[cell].links[3];

        far3 = -1;
        if (next != -1)
            far3 = hexCells[next].links[3];
    }
    target = 0;
    if (cell == 55 || cell == 59 || cell == 57)
        target = cell + 2;
    if (target) {
        n = placeAlike(cell, 4);
        if (n == -1) {
            alikeTarget = -1;
            return -1;
        }
    }
    target = cell + 2;
    if (far5 != -1 && hexCells[far5].state == 506) {
        if (!anyLeftToPlace())
            return -1;
        n = placeAlike(cell, 5);
        if (n == -1) {
            hexCells[far5].state = 501;
            return alikeTarget = target;
        }
    }
    if (hexCells[target].state == 507) {
        n = sharedStone(target, far5);
        if (n)
            hexCells[hexCells[far5].links[3]].snoid = n;
    }
    if (far3 != -1 && hexCells[far3].state == 506) {
        if (!anyLeftToPlace())
            return -1;
        n = placeAlike(cell, 3);
        if (n == -1) {
            hexCells[far3].state = 501;
            return alikeTarget = target;
        }
        if (hexCells[target].state == 507) {
            n = sharedStone(target, far3);
            if (n)
                hexCells[hexCells[far3].links[5]].snoid = n;
        }
    }
    return 0;
}

/* Whether two of the Zoombinis (waitingSnoids) share a feature; sharedFeature is
   set to the first they share (0-3). */
/* @zoombi32 0x0044cd71 */
short shareFeature(short first, short second)
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
        Snoid *snoid = (Snoid *)&findView(partyViews[waitingSnoids[first]])->body;

        hair = snoid->features[0];
        eyes = snoid->features[1];
        nose = snoid->features[2];
        feet = snoid->features[3];
    }
    {
        Snoid *snoid = (Snoid *)&findView(partyViews[waitingSnoids[second]])->body;

        hair2 = snoid->features[0];
        eyes2 = snoid->features[1];
        nose2 = snoid->features[2];
        feet2 = snoid->features[3];
    }
    if (hair == hair2) {
        sharedFeature = 0;
        return 1;
    }
    if (eyes == eyes2) {
        sharedFeature = 1;
        return 1;
    }
    if (nose == nose2) {
        sharedFeature = 2;
        return 1;
    }
    if (feet2 == feet) {
        sharedFeature = 3;
        return 1;
    }
    return 0;
}

/* Picks a random Zoombini of the views crossingViews and copies its features
   into targetFeatures, and from smokeLevel 3 on, the next one's after it; returns
   how many views there were. */
/* @zoombi32 0x00451f4e */
short takeRandomFeatures()
{
    short i;
    short count;
    short ids[22];
    View *view;

    for (i = 0, count = 0; i < crossingCount; i++)
        if (crossingViews[i])
            ids[count++] = crossingViews[i];
    if (count) {
        i = randomBetween(0, count - 1);
        view = findView(ids[i]);
        if (view) {
            Snoid *snoid = viewSnoid(view);
            char *features = snoid->features;

            targetFeatures[0] = features[0];
            targetFeatures[1] = features[1];
            targetFeatures[2] = features[2];
            targetFeatures[3] = features[3];
        }
        if (smokeLevel >= 3 && count > 1) {
            i++;
            if (i == count)
                i = 0;
            view = findView(ids[i]);
            if (view) {
                Snoid *snoid = viewSnoid(view);
                char *features = snoid->features;

                targetFeatures[4] = features[0];
                targetFeatures[5] = features[1];
                targetFeatures[6] = features[2];
                targetFeatures[7] = features[3];
            }
        } else {
            targetFeatures[4] = 0;
            targetFeatures[5] = 0;
            targetFeatures[6] = 0;
            targetFeatures[7] = 0;
        }
    }
    return count;
}

/* The cheat's message: "You have entered the psychedelic ZB Zone!", in a
   box at zoneMessageRect. */
/* @zoombi32 0x0044dcdc */
void showZoneMessage()
{
    ShortRect rect = zoneMessageRect;
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
   smokeGoReady), lit or not, and with `show` shows it. */
/* @zoombi32 0x0044f066 */
void drawSmokeButton(short which, short lit, short show)
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
        if (!smokeGoReady) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        handle = usedResourceHandle(smokeButtonResource);
        lockHandle(handle);
        bank = (ImageBank *)handleData(handle);
        unsigned short *data = (unsigned short *)(swapLong(bank->offsets[image]) + (char *)bank);

        drawImageData(data, smokeButtons[which - 1].rect.left, smokeButtons[which - 1].rect.top, 8);
        unlockHandle(handle);
        if (show)
            showRect(&smokeButtons[which - 1].rect);
    }
}

/* Gives the Zoombinis in the views comparedViews the features set in slots 0-3
   and 7-4 (in that order, so later slots win), and returns 2 if the two
   then differ, else 0. */
/* @zoombi32 0x004513ac */
short applySlotFeatures()
{
    char *first;
    char *second;
    short i;
    View *view;
    short result;

    view = findView(comparedViews[0]);
    if (view) {
        view->body.running = 1;
        Snoid *snoid = (Snoid *)&view->body;

        snoid->action = 4;
        first = snoid->features;
        for (i = 0; i < 4; i++) {
            if (featureSlots[i][0])
                first[0] = featureSlots[i][0];
            if (featureSlots[i][1])
                first[1] = featureSlots[i][1];
            if (featureSlots[i][2])
                first[2] = featureSlots[i][2];
            if (featureSlots[i][3])
                first[3] = featureSlots[i][3];
        }
    }
    view = findView(comparedViews[1]);
    if (view) {
        view->body.running = 1;
        Snoid *snoid = (Snoid *)&view->body;

        snoid->action = 4;
        second = snoid->features;
        for (i = 7; i > 3; i--) {
            if (featureSlots[i][0])
                second[0] = featureSlots[i][0];
            if (featureSlots[i][1])
                second[1] = featureSlots[i][1];
            if (featureSlots[i][2])
                second[2] = featureSlots[i][2];
            if (featureSlots[i][3])
                second[3] = featureSlots[i][3];
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

/* Starts the scene's next move: view moverView1 or moverView2's script, and
   the pair of views pairView1 and pairView2 (scripts from pairScripts), grouped. */
/* @zoombi32 0x004514f6 */
void startNextMove()
{
    View *view;
    View *first;
    View *second;

    movePlace = 0;
    if (!useSecondMover)
        view = findView(moverView1);
    else
        view = findView(moverView2);
    if (view) {
        view->flags = 0x5188000;
        if (smokeLevel == 1 || smokeLevel == 2) {
            setViewScript(view, moverScripts[0], 0);
        } else {
            useSecondMover = !useSecondMover;
            setViewScript(view, moverScripts[1], 0);
        }
    }
    first = findView(pairView1);
    if (first) {
        setViewScript(first, pairScripts[pairMismatch], 1);
        loadViewSounds(first->id, 1);
        first->notify = smokeViewNotify;
    }
    second = findView(pairView2);
    if (second) {
        setViewScript(second, pairScripts[pairMismatch + 1], 1);
        second->notify = smokeViewNotify;
    }
    view = findView(crossingSnoid);
    if (view && second)
        moveView(view->id, 0, second->id);
    if (first && second)
        groupViews(first->id, second->id, 0, 0, 0, 0);
}

/* Gives the eight Zoombinis in the views randomViews random features at their
   places (randomPlaces), except that one of the first `count`, at random, gets
   the features set in targetFeatures. */
/* @zoombi32 0x004520ec */
void dealRandomFeatures(short count)
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
            view = findView(randomViews[i]);
            if (view) {
                view->body.running = 1;
                view->changed = 1;
                snoid = (Snoid *)&view->body;
                snoid->action = 4;
                *(Point *)&snoid->body.x = randomPlaces[i];
                features = snoid->features;
                for (j = 0; j < 4; j++)
                    features[j] = randomBetween(1, 4);
                snoid->angle = 1;
                if (i == chosen) {
                    if (targetFeatures[0])
                        features[0] = targetFeatures[0];
                    else
                        features[0] = randomBetween(1, 4);
                    if (targetFeatures[1])
                        features[1] = targetFeatures[1];
                    else
                        features[1] = randomBetween(1, 4);
                    if (targetFeatures[2])
                        features[2] = targetFeatures[2];
                    else
                        features[2] = randomBetween(1, 4);
                    if (targetFeatures[3])
                        features[3] = targetFeatures[3];
                    else
                        features[3] = randomBetween(1, 4);
                }
            }
        }
    }
}

/* Moves on the one feature (pose) of the Zoombinis in the views
   slotViews[0-2] that change it: each takes the next value after its slot's (or
   an earlier slot's, if its is unset), wrapping 5 round to 1, and records
   it in the next slot. */
/* @zoombi32 0x00450e87 */
void advanceLeftFeatures()
{
    short i;
    View *view;
    Snoid *snoid;

    for (i = 0; i < 3; i++) {
        view = findView(slotViews[i]);
        if (view) {
            snoid = (Snoid *)&view->body;
            snoid->action = 4;
            if (snoid->pose) {
                switch (i) {
                case 0:
                    snoid->features[snoid->pose - 1] = featureSlots[0][snoid->pose - 1] + 1;
                    break;
                case 1:
                    if (featureSlots[1][snoid->pose - 1])
                        snoid->features[snoid->pose - 1] = featureSlots[1][snoid->pose - 1] + 1;
                    else
                        snoid->features[snoid->pose - 1] = featureSlots[0][snoid->pose - 1] + 1;
                    break;
                case 2:
                    if (featureSlots[2][snoid->pose - 1])
                        snoid->features[snoid->pose - 1] = featureSlots[2][snoid->pose - 1] + 1;
                    else if (featureSlots[1][snoid->pose - 1])
                        snoid->features[snoid->pose - 1] = featureSlots[1][snoid->pose - 1] + 1;
                    else
                        snoid->features[snoid->pose - 1] = featureSlots[0][snoid->pose - 1] + 1;
                    break;
                }
                if (snoid->features[snoid->pose - 1] > 5)
                    snoid->features[snoid->pose - 1] = 1;
                featureSlots[i + 1][snoid->pose - 1] = snoid->features[snoid->pose - 1];
                view->tag = snoid->features[snoid->pose - 1];
                snoid->idleTicks = 0;
            }
        }
    }
}

/* The same from the other side: the views slotViews[5-3], from slots 7-5,
   recording in slots 6-4. */
/* @zoombi32 0x00451020 */
void advanceRightFeatures()
{
    short i;
    View *view;
    Snoid *snoid;

    for (i = 5; i > 2; i--) {
        view = findView(slotViews[i]);
        if (view) {
            snoid = (Snoid *)&view->body;
            snoid->action = 4;
            if (snoid->pose) {
                switch (i) {
                case 3:
                    if (featureSlots[5][snoid->pose - 1])
                        snoid->features[snoid->pose - 1] = featureSlots[5][snoid->pose - 1] + 1;
                    else if (featureSlots[6][snoid->pose - 1])
                        snoid->features[snoid->pose - 1] = featureSlots[6][snoid->pose - 1] + 1;
                    else
                        snoid->features[snoid->pose - 1] = featureSlots[7][snoid->pose - 1] + 1;
                    break;
                case 4:
                    if (featureSlots[6][snoid->pose - 1])
                        snoid->features[snoid->pose - 1] = featureSlots[6][snoid->pose - 1] + 1;
                    else
                        snoid->features[snoid->pose - 1] = featureSlots[7][snoid->pose - 1] + 1;
                    break;
                case 5:
                    snoid->features[snoid->pose - 1] = featureSlots[7][snoid->pose - 1] + 1;
                    break;
                }
                if (snoid->features[snoid->pose - 1] > 5)
                    snoid->features[snoid->pose - 1] = 1;
                featureSlots[i + 1][snoid->pose - 1] = snoid->features[snoid->pose - 1];
                view->tag = snoid->features[snoid->pose - 1];
                snoid->idleTicks = 0;
            }
        }
    }
}

/* The scene's keys (with debugMessagesOn, the cheat keys; else only 0x16f): L shows
   the level, 0x172 and 0x173 turn cheating (cheatMode) on and off, 0x16f
   calls replayHint. Returns whether the key was used. */
/* Not exact: the original keeps `key` in esi and `show` in ebx (saving esi
   around the arrays' initial copies); this swaps them. */
/* @zoombi32 0x00450a58 */
short smokeKey(unsigned short key)
{
    Color saved;
    char digits[52] = "01234";
    char level[52] = "Level x ";
    short show = 0;
    ShortRect rect = smokeButtonsRect;
    char text[52];

    if (!debugMessagesOn && key != 0x16f)
        return 0;
    switch (key) {
    case 'L':
        level[6] = digits[smokeLevel];
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
        cheatMode = 1;
        key = 1;
        break;
    case 0x173:
        show = 0;
        cheatMode = 0;
        key = 1;
        break;
    case 0x16f:
        replayHint();
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
   (waitingSnoids) that shares a feature with the one on `cell` (starting from a
   random feature); places it on the far cell (state 507) with the middle
   one showing the feature (state 501, view 510-513), and returns its
   index. -2: no such cells; -1: none shares a feature. (It reuses `cell`
   as the index.) */
/* `direction` is volatile only to leave it on the stack, as the original
   does (BCC would otherwise give it edi until `differ` needs it). */
/* @zoombi32 0x0044d3b8 */
short placeAlike(short cell, volatile short direction)
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
    next = hexCells[cell].links[direction];
    if (next < 0)
        return -2;
    further = hexCells[next].links[direction];
    if (further < 0)
        return -2;
    {
        Snoid *snoid = (Snoid *)&findView(hexCells[cell].snoid)->body;

        hair = snoid->features[0];
        eyes = snoid->features[1];
        nose = snoid->features[2];
        feet = snoid->features[3];
    }
    differ = 1;
    for (cell = 0; cell < partySize; cell++)
        if (waitingSnoids[cell] != -1) {
            Snoid *snoid = (Snoid *)&findView(partyViews[waitingSnoids[cell]])->body;

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
                hexCells[further].state = 507;
                hexCells[further].snoid = partyViews[waitingSnoids[cell]];
                hexCells[next].state = 501;
                hexCells[next].snoid = feature + 510;
                waitingSnoids[cell] = -1;
                return cell;
            }
        }
    return -1;
}

/* The memory statistics line (with showMemoryStats): free, purgeable and the
   least free seen (leastFreeMemory), in thousands; with `clear`, unloads the
   sounds and blanks the line instead. */
/* Not exact: the original gives `port` esi and `freeThousands` ebx (shared
   with `available`); this gives them the other way round. */
/* @zoombi32 0x00455023 */
void drawMemoryStats(short clear)
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
        drawText(Rect(memoryStatsRect), 0x11, text, 0xffff);
        setPort(port);
    } else if (showMemoryStats) {
        available = availableVirtualMemory();
        purgeable = purgeMemory(0, 0);
        if (available < leastFreeMemory)
            leastFreeMemory = available;
        freeUnits = available % 1000;
        freeThousands = available / 1000;
        purgeableUnits = purgeable % 1000;
        purgeableThousands = purgeable / 1000;
        leastUnits = leastFreeMemory % 1000;
        leastThousands = leastFreeMemory / 1000;
        port = getPort();
        setPort(workPort);
        font = getFont();
        setFont(fonts[1]);
        sprintf(text, "Free:%d,%03d  Purg:%d,%03d  Min:%d,%03d", (short)freeThousands, (short)freeUnits,
                (short)purgeableThousands, (short)purgeableUnits, (short)leastThousands, (short)leastUnits);
        fillPortRect(Rect(memoryStatsRect), Color(0xff), 0);
        saved = setForeColor(Color(0));
        drawText(Rect(memoryStatsRect), 0x11, text, 0xffff);
        setForeColor(saved);
        setFont(font);
        setPort(port);
        showRect(&memoryStatsRect);
    }
}

/* A view's drawing: buttons 1 and 2, unlit. */
/* @zoombi32 0x0044f163 */
void drawSmokeButtons(View *)
{
    drawSmokeButton(1, 0, 0);
    drawSmokeButton(2, 0, 0);
}

/* Lets the movie play (cmgr_09; MCIdle?); once it has stopped (flag 0x40
   clear; mcInfoIsPlaying?), closes it and returns 1. 0: still playing;
   -1: no movie. */
/* @zoombi32 0x00455229 */
short idleMovie()
{
    long flags;

    if (!movieController)
        return -1;
    cmgr_05(movieController, &flags);
    cmgr_09(movieController);
    if (flags & 0x40)
        return 0;
    stopMovie(0);
    return 1;
}

/* Sets the Zoombinis' features from the slots (applySlotFeatures; slotsDiffer if the
   two differ) and starts the views view11018 and view11019 (11018, 11019),
   grouped. */
/* @zoombi32 0x00451315 */
void setPairFeatures()
{
    View *first;
    View *second;

    pairMismatch = applySlotFeatures();
    slotsDiffer = 0;
    if (pairMismatch)
        slotsDiffer = 1;
    first = findView(view11018);
    if (first) {
        setViewScript(first, 11018, 1);
        first->notify = smokeViewNotify;
    }
    second = findView(view11019);
    if (second) {
        setViewScript(second, 11019, 1);
        second->notify = smokeViewNotify;
    }
    if (first && second)
        groupViews(first->id, second->id, 0, 0, 0, 0);
}

/* Fills the free cells of a set of 20 with waiting Zoombinis, while any
   are left. */
/* Its loop runs to 22, past the end of the cells, and each Zoombini's view
   is read after its entry in waitingSnoids is cleared (partyViews[-1]); both as
   in the original. */
/* @zoombi32 0x0044d5f5 */
void fillFreeCells()
{
    short cells[20] = {55, 57, 59, 61, 38, 74, 40, 76, 42, 78, 44, 80, 21, 93, 23, 95, 25, 97, 4, 112};
    short i;
    short j;

    for (i = 0; i < 22; i++)
        if (hexCells[cells[i]].state != 507) {
            hexCells[cells[i]].state = 507;
            for (j = 0; j < partySize; j++)
                if (waitingSnoids[j] != -1) {
                    waitingSnoids[j] = -1;
                    hexCells[cells[i]].snoid = partyViews[waitingSnoids[j]];
                    break;
                }
            if (!anyLeftToPlace())
                break;
        }
}

/* Clears cells of a line of three (a cell's `snoid` from 510 is a feature
   marker, below it a Zoombini): all three when both ends hold Zoombinis and
   the middle is in state 501; otherwise, with the middle in state 507, the
   second end if both ends hold Zoombinis or only the second does, the first
   if only the first does. */
/* @zoombi32 0x0044e21a */
void clearLine(short first, short second, short middle)
{
    if (hexCells[first].snoid < 510 && hexCells[second].snoid < 510 && hexCells[middle].state == 501) {
        unlinkCell(first);
        unlinkCell(second);
        unlinkCell(middle);
    } else if (hexCells[first].snoid < 510 && hexCells[second].snoid < 510 && hexCells[middle].state == 507) {
        unlinkCell(second);
    } else if (hexCells[first].snoid < 510 && hexCells[second].snoid >= 510 && hexCells[middle].state == 507) {
        unlinkCell(first);
    } else if (hexCells[first].snoid >= 510 && hexCells[second].snoid < 510 && hexCells[middle].state == 507) {
        unlinkCell(second);
    }
}

/* Plays a QuickTime movie from a file, centred in the window: loads it
   (loadMovie), makes or reuses the movie controller (movieController; qtim_38 is
   NewMovieController?), and starts it. Returns 0, or 1 if it failed. */
/* @zoombi32 0x0045537f */
short playMovie(const char *path)
{
    POINT where;
    Point offset;
    RECT bounds;
    short failed;
    short i;

    failed = 1;
    currentMovie = loadMovie(path);
    if (currentMovie) {
        portBeforeMovie = getPort();
        setPort(screenPort);
        qtim_0f(currentMovie, &bounds);
        OffsetRect(&bounds, -bounds.left, -bounds.top);
        offset.x = ((screenRect.right - screenRect.left) - (gameRect.right - gameRect.left)) / 2;
        offset.y = ((screenRect.bottom - screenRect.top) - (gameRect.bottom - gameRect.top)) / 2;
        OffsetRect(&bounds, offset.x, offset.y);
        if (!movieController) {
            movieController = qtim_38(currentMovie, &bounds, 11, mainWindow);
        } else {
            where.x = offset.x;
            where.y = offset.y;
            cmgr_0d(movieController, currentMovie, mainWindow, where);
        }
        cmgr_0e(movieController, &bounds, 0, 11);
        cmgr_00(movieController, mainWindow, 1);
        cmgr_01(movieController, 0x20, 0);
        qtim_31(currentMovie, 1);
        qtim_62(0, 0);
        qtim_2f(currentMovie, 0, 0x10000);
        for (i = 0; i < 100; i++)
            cmgr_09(movieController);
        cmgr_01(movieController, 8, 0x10000);
        failed = 0;
        introPending = 0;
        movieShowing = 1;
    }
    return failed;
}

/* Recomputes the cells' link bits: a cell in state 502, 504, 505 or 508
   clears its bit for each neighbour in state 501, 506 or 507; one in state
   501, 506 or 507 for each neighbour in state 502, 508 or startState. Then
   starts script 7000 on the views of the cells in state 501, 506 or 507. */
/* @zoombi32 0x0044ddc9 */
void updateCellLinks()
{
    short i;
    View *view;
    short direction;
    short state;

    for (i = 0; i < 117; i++)
        switch (hexCells[i].state) {
        case 502:
        case 504:
        case 505:
        case 508:
            for (direction = 0; direction <= 5; direction++) {
                state = hexCells[hexCells[i].links[direction]].state;
                if (state == 501 || state == 506 || state == 507)
                    switch (direction) {
                    case 0:
                        cellLinkBits[i] |= 1;
                        cellLinkBits[i] ^= 1;
                        break;
                    case 1:
                        cellLinkBits[i] |= 2;
                        cellLinkBits[i] ^= 2;
                        break;
                    case 2:
                        cellLinkBits[i] |= 4;
                        cellLinkBits[i] ^= 4;
                        break;
                    case 3:
                        cellLinkBits[i] |= 8;
                        cellLinkBits[i] ^= 8;
                        break;
                    case 4:
                        cellLinkBits[i] |= 0x10;
                        cellLinkBits[i] ^= 0x10;
                        break;
                    case 5:
                        cellLinkBits[i] |= 0x20;
                        cellLinkBits[i] ^= 0x20;
                        break;
                    }
            }
            break;
        case 501:
        case 506:
        case 507:
            for (direction = 0; direction <= 5; direction++) {
                state = hexCells[hexCells[i].links[direction]].state;
                if (state == 502 || state == 508 || state == startState)
                    switch (direction) {
                    case 0:
                        cellLinkBits[i] |= 1;
                        cellLinkBits[i] ^= 1;
                        break;
                    case 1:
                        cellLinkBits[i] |= 2;
                        cellLinkBits[i] ^= 2;
                        break;
                    case 2:
                        cellLinkBits[i] |= 4;
                        cellLinkBits[i] ^= 4;
                        break;
                    case 3:
                        cellLinkBits[i] |= 8;
                        cellLinkBits[i] ^= 8;
                        break;
                    case 4:
                        cellLinkBits[i] |= 0x10;
                        cellLinkBits[i] ^= 0x10;
                        break;
                    case 5:
                        cellLinkBits[i] |= 0x20;
                        cellLinkBits[i] ^= 0x20;
                        break;
                    }
            }
            break;
        }
    for (i = 0; i < 117; i++)
        if (hexCells[i].state == 501 || hexCells[i].state == 506 || hexCells[i].state == 507) {
            view = findView(hexCells[i].view);
            setViewScript(view, 7000, 1);
            view->placed = placeCellViewImages;
        }
    updateViews();
}

/* Turns cells in state 506 back to 501, lists the placed Zoombinis (state
   507) in placedSnoids (each one's view lands in the next entry, as in the
   original), and for each of a fixed set of cells (or pairs) found in state
   501, resets it and the cells next to it and cuts the neighbours' links
   to them. */
/* @zoombi32 0x0044d127 */
void settleCells()
{
    short count = 0;
    short i;

    for (i = 0; i < 117; i++) {
        if (hexCells[i].state == 506)
            hexCells[i].state = 501;
        if (hexCells[i].state == 507) {
            placedSnoids[count].cell = i;
            count++;
            placedSnoids[count].snoid = hexCells[i].snoid;
        }
    }
    if (hexCells[2].state == 501 && hexCells[19].state == 501) {
        resetCell(2);
        resetCell(19);
        resetCell(10);
        resetCell(11);
        resetCell(28);
        cutLink(38, 0, 1);
        cutLink(21, 0, 1);
    }
    if (hexCells[91].state == 501 && hexCells[110].state == 501) {
        resetCell(91);
        resetCell(110);
        resetCell(100);
        resetCell(82);
        resetCell(101);
        cutLink(74, 2, 4);
        cutLink(93, 2, 4);
    }
    if (hexCells[112].state == 501) {
        resetCell(112);
        resetCell(102);
        resetCell(103);
        cutLink(93, 3, 8);
        cutLink(95, 2, 4);
    }
    if (hexCells[114].state == 501) {
        resetCell(114);
        resetCell(104);
        resetCell(105);
        cutLink(95, 3, 8);
        cutLink(97, 2, 4);
    }
    if (hexCells[4].state == 501) {
        resetCell(4);
        resetCell(12);
        resetCell(13);
        cutLink(21, 5, 0x20);
        cutLink(23, 0, 1);
    }
    if (hexCells[6].state == 501) {
        resetCell(6);
        resetCell(14);
        resetCell(15);
        cutLink(23, 5, 0x20);
        cutLink(25, 0, 1);
    }
    if (hexCells[97].state == 501 && hexCells[80].state == 501) {
        resetCell(97);
        resetCell(80);
        resetCell(88);
        resetCell(87);
        resetCell(70);
        resetCell(105);
        cutLink(78, 3, 8);
        cutLink(61, 3, 8);
        cutLink(114, 5, 0x20);
    }
    if (hexCells[25].state == 501 && hexCells[44].state == 501) {
        resetCell(25);
        resetCell(44);
        resetCell(34);
        resetCell(15);
        resetCell(33);
        resetCell(52);
        cutLink(42, 5, 0x20);
        cutLink(61, 5, 0x20);
        cutLink(6, 3, 8);
    }
}

/*
 * Drags a Zoombini (from `where`), snapping it to the spot it's over: with
 * smokeLevel below 3, the one spot spot4Rect (4) unless featureSlots[7][0]; otherwise
 * one of the three spots in row leftRow of leftRowSpots (0-2) or row rightRow
 * of rightRowSpots (3-5). Returns the spot it was over when released (-1:
 * none).
 */
/* @zoombi32 0x00453e8c */
short dragSnoidToSpot(View *view, Point where)
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
    if (smokeLevel == 3 || smokeLevel == 4)
        startRowViews();
    else if (!randomDragSlot)
        startView11076();
    while (keepDragging()) {
        getCursorPosition(&current);
        spot = -1;
        if (smokeLevel < 3) {
            if (ptInRect(&spot4Rect, current) && !featureSlots[7][0]) {
                spot = 4;
                current.x = madePlaces1[1].x;
                current.y = madePlaces1[1].y;
            }
        } else {
            if (leftRow < 3)
                for (i = 0; i < 3; i++)
                    if (ptInRect(&leftRowSpots[leftRow][i], current)) {
                        spot = i;
                        current.x = leftRowSpots[leftRow][i].left + 25;
                        current.y = leftRowSpots[leftRow][i].top + 31;
                        i = 3;
                    }
            if (spot < 0 && rightRow < 3)
                for (i = 0; i < 3; i++)
                    if (ptInRect(&rightRowSpots[rightRow][i], current)) {
                        spot = i + 3;
                        current.x = rightRowSpots[rightRow][i].left + 25;
                        current.y = rightRowSpots[rightRow][i].top + 31;
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
        snoid->action = 5;
        mainLoopEvents();
        resetViewClock();
    }
    snoid->action = 4;
    rect = dragged->body.bounds;
    unionRgnRect(removedRgn, &rect);
    dragged->id = id;
    dragged->interval = savedInterval;
    return spot;
}

/* Places the party on the 16 starting cells (`cells`: the first
   partySize of them taken, state 507, by the Zoombinis in order; the rest
   state 501), then marks the cells between each placed Zoombini and its
   neighbours with a feature they share (markSharedFeature). */
/* @zoombi32 0x0044d6a3 */
void placePartyOnGrid()
{
    short cells[16] = {19, 25, 91, 97, 57, 59, 40, 76, 78, 38, 74, 42, 21, 95, 23, 93};
    short i;
    short index;

    orderPartyByAlike();
    for (i = 0; i < 16; i++)
        hexCells[cells[i]].state = 501;
    for (i = 0; i < partySize; i++) {
        hexCells[cells[i]].state = 507;
        hexCells[cells[i]].snoid = i;
    }
    for (i = 0; i < partySize; i++)
        if (hexCells[cells[i]].state == 507) {
            index = hexCells[cells[i]].snoid;
            switch (cells[i]) {
            case 19:
                markSharedFeature(38, index, 28);
                markSharedFeature(21, index, 20);
                break;
            case 21:
                markSharedFeature(38, index, 29);
                markSharedFeature(40, index, 30);
                markSharedFeature(23, index, 22);
                break;
            case 23:
                markSharedFeature(40, index, 31);
                markSharedFeature(42, index, 32);
                markSharedFeature(25, index, 24);
                break;
            case 25:
                markSharedFeature(42, index, 33);
                break;
            case 38:
                markSharedFeature(40, index, 39);
                markSharedFeature(57, index, 47);
                break;
            case 40:
                markSharedFeature(57, index, 48);
                markSharedFeature(59, index, 49);
                markSharedFeature(42, index, 41);
                break;
            case 42:
                markSharedFeature(59, index, 50);
                break;
            case 57:
                markSharedFeature(74, index, 65);
                markSharedFeature(76, index, 66);
                markSharedFeature(59, index, 58);
                break;
            case 59:
                markSharedFeature(76, index, 67);
                markSharedFeature(78, index, 68);
                break;
            case 74:
                markSharedFeature(91, index, 82);
                markSharedFeature(93, index, 83);
                markSharedFeature(76, index, 75);
                break;
            case 76:
                markSharedFeature(93, index, 84);
                markSharedFeature(95, index, 85);
                markSharedFeature(78, index, 77);
                break;
            case 78:
                markSharedFeature(95, index, 86);
                markSharedFeature(97, index, 87);
                break;
            case 91:
                markSharedFeature(93, index, 92);
                break;
            case 93:
                markSharedFeature(95, index, 94);
                break;
            case 95:
                markSharedFeature(97, index, 96);
                break;
            case 97:
                break;
            }
        }
}

/* Marks `cell` with a feature (view 510-513) that the Zoombini `index`
   shares with the one on `neighbour` (if that one is placed, state 507),
   checking the features from a random one on. */
/* @zoombi32 0x0044d974 */
void markSharedFeature(short neighbour, short index, short cell)
{
    long other;

    if (hexCells[neighbour].state == 507)
        switch (randomUpTo(3)) {
        case 0:
            other = hexCells[neighbour].snoid;
            if (partyHair[index] == partyHair[other])
                hexCells[cell].snoid = 510;
            else if (partyEyes[index] == partyEyes[other])
                hexCells[cell].snoid = 511;
            else if (partyNoses[index] == partyNoses[other])
                hexCells[cell].snoid = 512;
            else if (partyFeet[index] == partyFeet[other])
                hexCells[cell].snoid = 513;
            break;
        case 1:
            other = hexCells[neighbour].snoid;
            if (partyEyes[index] == partyEyes[other])
                hexCells[cell].snoid = 511;
            else if (partyNoses[index] == partyNoses[other])
                hexCells[cell].snoid = 512;
            else if (partyFeet[index] == partyFeet[other])
                hexCells[cell].snoid = 513;
            else if (partyHair[index] == partyHair[other])
                hexCells[cell].snoid = 510;
            break;
        case 2:
            other = hexCells[neighbour].snoid;
            if (partyNoses[index] == partyNoses[other])
                hexCells[cell].snoid = 512;
            else if (partyFeet[index] == partyFeet[other])
                hexCells[cell].snoid = 513;
            else if (partyHair[index] == partyHair[other])
                hexCells[cell].snoid = 510;
            else if (partyEyes[index] == partyEyes[other])
                hexCells[cell].snoid = 511;
            break;
        case 3:
            other = hexCells[neighbour].snoid;
            if (partyFeet[index] == partyFeet[other])
                hexCells[cell].snoid = 513;
            else if (partyHair[index] == partyHair[other])
                hexCells[cell].snoid = 510;
            else if (partyEyes[index] == partyEyes[other])
                hexCells[cell].snoid = 511;
            else if (partyNoses[index] == partyNoses[other])
                hexCells[cell].snoid = 512;
            break;
        }
}

/*
 * Lays out a Zoombini's cels in this scene (unless it's in state 2, which
 * it's put in unless it's in 3 or 5): a part for its pose (angle), then
 * its face (layer layers[angle], if all four features are set) and
 * its features on top, placed by the hot spots in smokeHotSpotsX/smokeHotSpotsY; then
 * its bounds from the images in smokeImages.
 */
/* @zoombi32 0x00454374 */
void layOutSmokeSnoid(Snoid *snoid)
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
    switch (snoid->action) {
    case 2:
        return;
    case 3:
    case 5:
        break;
    default:
        snoid->action = 2;
        break;
    }
    if (!snoid->features[3] || !snoid->features[2] || !snoid->features[1] || !snoid->features[0])
        face = 0;
    else
        face = snoid->layers[snoid->angle];
    base = snoid->layers[snoid->angle];
    where = *(Point *)&snoid->body.x;
    dy = 0;
    switch (snoid->angle) {
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
        *cel++ = where.x - smokeHotSpotsX[part];
        *cel++ = where.y - smokeHotSpotsY[part];
    }
    if (face) {
        *cel++ = base;
        *cel++ = where.x - smokeHotSpotsX[base];
        *cel++ = where.y - smokeHotSpotsY[base] - dy;
    }
    if (snoid->features[3]) {
        part = snoid->features[3] + base + 15;
        *cel++ = part;
        *cel++ = where.x - smokeHotSpotsX[part];
        *cel++ = where.y - smokeHotSpotsY[part] - dy;
    }
    if (snoid->features[1]) {
        part = snoid->features[1] + base + 5;
        *cel++ = part;
        *cel++ = where.x - smokeHotSpotsX[part];
        *cel++ = where.y - smokeHotSpotsY[part] - dy;
    }
    if (snoid->features[2]) {
        part = snoid->features[2] + base + 10;
        *cel++ = part;
        *cel++ = where.x - smokeHotSpotsX[part];
        *cel++ = where.y - smokeHotSpotsY[part] - dy;
    }
    if (snoid->features[0]) {
        part = snoid->features[0] + base;
        *cel++ = part;
        *cel++ = where.x - smokeHotSpotsX[part];
        *cel++ = where.y - smokeHotSpotsY[part] - dy;
    }
    *cel++ = 0;
    *cel++ = 0;
    *cel = 0;
    cel = (short *)snoid->body.cels;
    bank = smokeImages;
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
 * features (featureOrder) and values (valueOrder) without repeats: for slots 0
 * and 1 values unlike the ones already chosen (leftChosenValues), recorded in slot
 * n + 1; for 2 and 3, mostly the chosen ones (else those set in targetFeatures),
 * recorded in slot n + 2 and rightChosenValues. Then updates targetFeatures, and places
 * the Zoombini at dealtPlaces[n].
 */
/* @zoombi32 0x00452258 */
void giveSlotFeatures(Snoid *snoid, short n)
{
    short *order = featureOrder;
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
        valueOrder[i] = i + 1;
    valuePick = 0;
    valuePickMax = 3;
    featurePick = 0;
    featurePickMax = 4;
    if (!n)
        for (j = 0; j < 4; j++) {
            leftChosenValues[j] = 0;
            rightChosenValues[j] = 0;
        }
    left = randomBetween(1, 2);
    start = left;
    for (i = 0; i < 4 && left > 0; i++) {
        featurePick = randomBetween(0, featurePickMax);
        valuePick = randomBetween(0, valuePickMax);
        switch (n) {
        case 0:
        case 1:
            snoid->angle = 0;
            if (leftChosenValues[order[valuePick]] != valueOrder[featurePick]) {
                snoid->features[order[valuePick]] = valueOrder[featurePick];
                featureSlots[n + 1][order[valuePick]] = valueOrder[featurePick];
                leftChosenValues[order[valuePick]] = valueOrder[featurePick];
                left--;
            }
            break;
        case 2:
            snoid->angle = 2;
            k = randomBetween(0, 3);
            if (leftChosenValues[k]) {
                if ((r = randomBetween(0, 100)) > 65 || start == left && i == 3) {
                    snoid->features[k] = leftChosenValues[k];
                    featureSlots[n + 2][k] = leftChosenValues[k];
                    rightChosenValues[k] = leftChosenValues[k];
                    left--;
                }
            } else {
                snoid->features[k] = targetFeatures[k];
                featureSlots[n + 2][k] = targetFeatures[k];
                rightChosenValues[k] = targetFeatures[k];
                left--;
            }
            break;
        case 3:
            snoid->angle = 2;
            k = randomBetween(0, 3);
            if (leftChosenValues[k]) {
                if ((r = randomBetween(0, 100)) > 65 || start == left && i == 3) {
                    snoid->features[k] = leftChosenValues[k];
                    featureSlots[n + 2][k] = leftChosenValues[k];
                    rightChosenValues[k] = leftChosenValues[k];
                    left--;
                }
            } else {
                snoid->features[k] = targetFeatures[k];
                featureSlots[n + 2][k] = targetFeatures[k];
                rightChosenValues[k] = targetFeatures[k];
                left--;
            }
            break;
        }
        for (j = valuePick; j < valuePickMax + 1; j++)
            order[j] = order[j + 1];
        valuePickMax--;
        for (j = featurePick; j < featurePickMax + 1; j++)
            valueOrder[j] = valueOrder[j + 1];
        featurePickMax--;
    }
    if (leftChosenValues[0])
        targetFeatures[0] = leftChosenValues[0];
    if (leftChosenValues[1])
        targetFeatures[1] = leftChosenValues[1];
    if (leftChosenValues[2])
        targetFeatures[2] = leftChosenValues[2];
    if (leftChosenValues[3])
        targetFeatures[3] = leftChosenValues[3];
    if (n == 3) {
        for (j = 5; j > 3; j--) {
            if (featureSlots[j][0])
                rightChosenValues[0] = featureSlots[j][0];
            if (featureSlots[j][1])
                rightChosenValues[1] = featureSlots[j][1];
            if (featureSlots[j][2])
                rightChosenValues[2] = featureSlots[j][2];
            if (featureSlots[j][3])
                rightChosenValues[3] = featureSlots[j][3];
        }
        if (targetFeatures[0] == rightChosenValues[0])
            targetFeatures[0] = 0;
        if (targetFeatures[1] == rightChosenValues[1])
            targetFeatures[1] = 0;
        if (targetFeatures[2] == rightChosenValues[2])
            targetFeatures[2] = 0;
        if (targetFeatures[3] == rightChosenValues[3])
            targetFeatures[3] = 0;
    }
    *(Point *)&snoid->body.x = dealtPlaces[n];
    snoid->action = 4;
}

/* Deals features to the four Zoombinis in the views dealtViews. */
/* @zoombi32 0x0045222d */
void dealFeatures()
{
    short i;
    View *view;

    for (i = 0; i < 4; i++) {
        view = findView(dealtViews[i]);
        if (view)
            giveSlotFeatures((Snoid *)&view->body, i);
    }
}

/* Adds a view for a Zoombini of this scene (drawn by drawSmokeSnoid, updated by
   updateSmokeSnoid) from `snoid`; returns its id. */
/* @zoombi32 0x00454165 */
short addSmokeSnoidView(Snoid *snoid)
{
    short id;
    View *view;

    id = addView(1, drawSmokeSnoid, updateSmokeSnoid, 0, 6, snoid, 0, 0);
    view = findView(id);
    if (view) {
        layOutSmokeSnoid(snoid);
        view->tag = 0;
        view->nextUpdate = 0;
        view->body.frameOffset = 0;
        view->flags = 0x4000002;
    }
    return id;
}

/* The scene's Zoombini views' update: cycles the feature being changed
   (pose) through its values every 60 ticks while idleTicks is set,
   else flashes it (every 30) between unknown1e and nothing; lays the
   Zoombini out again when it changes. */
/* @zoombi32 0x00454228 */
void updateSmokeSnoid(View *view, short region)
{
    short changed = 0;
    Snoid *snoid;
    char *features;

    if (!dialogFlags && view->body.running && clockTime() >= view->nextUpdate) {
        view->nextUpdate = clockTime() + view->interval;
        snoid = (Snoid *)&view->body;
        if (snoid->idleTicks && clockTime() >= view->body.frameOffset) {
            view->body.frameOffset = clockTime() + 60;
            features = snoid->features;
            snoid->idleTicks++;
            if (snoid->idleTicks > 5)
                snoid->idleTicks = 1;
            if (snoid->pose > 0)
                features[snoid->pose - 1] = snoid->idleTicks;
            snoid->action = 4;
        } else if (view->tag && clockTime() >= view->body.frameOffset) {
            view->body.frameOffset = clockTime() + 30;
            features = snoid->features;
            if (snoid->pose > 0 && !features[snoid->pose - 1])
                features[snoid->pose - 1] = view->tag;
            else if (snoid->pose > 0)
                features[snoid->pose - 1] = 0;
            snoid->action = 4;
        }
        switch (snoid->action) {
        case 2:
            break;
        default:
            changed = 1;
            break;
        }
        if (changed) {
            unionRgnRect(region, &view->body.bounds);
            layOutSmokeSnoid(snoid);
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
void smokeViewNotify(View *view, short event)
{
    View *other;
    Snoid *first;
    Snoid *second;
    short i;

    switch (event) {
    case 0:
        if (view->flags == 1) {
            Snoid *snoid = (Snoid *)&view->body;

            snoid->facingLeft = !snoid->facingLeft;
        }
        break;
    case 251:
        if (view->flags == 1)
            setSnoidFacing((Snoid *)&view->body, 1);
        break;
    case 1:
        view11017Due = 1;
        break;
    case 2:
        setPairFeatures();
        break;
    case 3:
        if (smokeLevel > 0 && smokeLevel < 4)
            dealDimDue = 1;
        break;
    case 4:
        startNextMove();
        break;
    case 10:
    case 11:
    case 13:
    case 14:
        if (crossingSnoid) {
            other = findView(crossingSnoid);
            if (other) {
                Snoid *snoid = (Snoid *)&other->body;

                startSmokeSnoidScript(crossingSnoid, crossScripts[slotsDiffer] + snoid->features[3], view->body.group, smokeViewNotify, 1);
            }
        }
        break;
    case 16:
        other = findView(comparedViews[0]);
        if (other) {
            other->nextUpdate = 0;
            first = (Snoid *)&other->body;
            first->action = 4;
        }
        other = findView(comparedViews[1]);
        if (other) {
            other->nextUpdate = 0;
            second = (Snoid *)&other->body;
            second->action = 4;
        }
        if (slotsDiffer) {
            *(Point *)&first->body.x = movePlaces1[movePlace];
            *(Point *)&second->body.x = movePlaces2[movePlace];
        } else {
            *(Point *)&first->body.x = movePlaces3[movePlace];
            *(Point *)&second->body.x = movePlaces4[movePlace];
        }
        movePlace++;
        break;
    case 17:
        if (crossingSnoid) {
            for (i = 0; i < crossingCount; i++)
                if (crossingViews[i] == crossingSnoid && smokeLevel != 4) {
                    crossingViews[i] = 0;
                    i = crossingCount;
                }
            if (pairMismatch)
                crossingSnoid = 0;
        }
        startNextCrossing(view->body.group);
        break;
    case 30:
        if (crossingViews[nextCrossing] && nextCrossing < crossingCount) {
            other = findView(crossingViews[nextCrossing]);
            if (other) {
                other->body.running = 0;
                crossingSnoid = crossingViews[nextCrossing];
                Snoid *snoid = (Snoid *)&other->body;

                *(Point *)&snoid->body.x = crossingStart2;
                startSmokeSnoidScript(crossingViews[nextCrossing], crossScript1, view->body.group, smokeViewNotify, 0);
                moveView(crossingViews[nextCrossing], 1, view->id);
            }
        }
        break;
    case 31:
        other = findView(moverView2);
        if (other) {
            moveView(moverView2, 1, moverView1);
            other->flags = 0x4108000;
            setViewScript(other, moverScripts[1], 1);
            other->notify = smokeViewNotify;
        }
        break;
    case 35:
        if (crossingViews[nextCrossing + 1] && nextCrossing + 1 < crossingCount) {
            other = findView(crossingViews[nextCrossing + 1]);
            if (other) {
                other->body.running = 0;
                Snoid *snoid = (Snoid *)&other->body;

                *(Point *)&snoid->body.x = crossingStart2;
                startSmokeSnoidScript(crossingViews[nextCrossing + 1], crossScript2, view->body.group, smokeViewNotify, 0);
                moveView(crossingViews[nextCrossing + 1], 1, view->id);
            }
        }
        break;
    case 36:
        if (crossOnceFlag) {
            crossOnceFlag = 0;
            copyToSlotView(crossingSnoid, 0);
            recordSlotFeatures(crossingSnoid, 0);
        }
        break;
    case 37:
        if (crossingViews[nextCrossing] && nextCrossing < crossingCount) {
            crossingSnoid = crossingViews[nextCrossing];
            startSmokeSnoidScript(crossingViews[nextCrossing], crossScript3, view->body.group, smokeViewNotify, 0);
            moveView(crossingViews[nextCrossing], 1, view->id);
        }
        if (useSecondMover) {
            other = findView(moverView1);
            if (other) {
                other->flags = 0x4108000;
                setViewScript(other, moverScripts[1], 1);
                other->notify = smokeViewNotify;
                ViewBody *body = &other->body;

                body->cels[0].image = 0;
            }
        } else {
            other = findView(moverView2);
            if (other) {
                other->flags = 0x4108000;
                setViewScript(other, moverScripts[1], 1);
                other->notify = smokeViewNotify;
                ViewBody *body = &other->body;

                body->cels[0].image = 0;
            }
        }
        break;
    case 38:
        if (smokeLevel == 4) {
            if (level4Stage == 3) {
                if (crossingSnoid) {
                    copyToSlotView(crossingSnoid, 0);
                    recordSlotFeatures(crossingSnoid, 0);
                    recordLeftSlots();
                    advanceLeftFeatures();
                    emptyPairViews();
                    other = findView(rowViews[7]);
                    if (other) {
                        first = (Snoid *)&other->body;
                        ViewBody *body = &first->body;

                        body->cels[0].image = 0;
                    }
                    if (crossingViews[nextCrossing]) {
                        other = findView(dealerView);
                        if (other) {
                            setViewScript(other, dealerScript, 1);
                            other->notify = stepBackNotify;
                            dealerRunning = 0;
                        }
                    }
                }
            } else if (level4Stage == 1 && featuresTaken && crossedCount <= crossingCount) {
                dealButtonState = 1;
                lightDealButton(11005);
            }
        }
        break;
    case 50:
        crossedDue = 1;
        break;
    case 51:
        anchorDue = 1;
        break;
    case 60:
        snoidsOnTheirWay = 0;
        snoidsArrived = 1;
        break;
    }
}

/*
 * Makes the scene's puzzle (with n 1) and gives the Zoombini for row n its
 * features. The rows (rowFeatures, with the second set in rowFeatures2) are built
 * from the features set in targetFeatures: rows 1 and 2 change one or two
 * features at random, rows 3 and 4 (at smokeLevel 3 and 4) follow on from
 * them, row 7 (and 8, for the second set) from rows 3 and 4, and rows 5
 * and 6 differ by level. rowChanges marks the features a row changes; the
 * Zoombini's first such feature is the one it changes (pose).
 */
/* @zoombi32 0x00452d5d */
void makeSmokeRows(Snoid *snoid, short n)
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
            rowFeatures[j][0] = 0;
            rowFeatures[j][1] = 0;
            rowFeatures[j][2] = 0;
            rowFeatures[j][3] = 0;
            rowFeatures2[j][0] = 0;
            rowFeatures2[j][1] = 0;
            rowFeatures2[j][2] = 0;
            rowFeatures2[j][3] = 0;
            rowChanges[j][0] = 0;
            rowChanges[j][1] = 0;
            rowChanges[j][2] = 0;
            rowChanges[j][3] = 0;
        }
        leftFeatureMarks[0] = 0;
        leftFeatureMarks[1] = 0;
        leftFeatureMarks[2] = 0;
        leftFeatureMarks[3] = 0;
        rightFeatureMarks[0] = 0;
        rightFeatureMarks[1] = 0;
        rightFeatureMarks[2] = 0;
        rightFeatureMarks[3] = 0;
        rowFeatures[0][0] = targetFeatures[0];
        rowFeatures[0][1] = targetFeatures[1];
        rowFeatures[0][2] = targetFeatures[2];
        rowFeatures[0][3] = targetFeatures[3];
        leftTargetFeatures[0] = targetFeatures[0];
        leftTargetFeatures[1] = targetFeatures[1];
        leftTargetFeatures[2] = targetFeatures[2];
        leftTargetFeatures[3] = targetFeatures[3];
        rowFeatures2[0][0] = targetFeatures[4];
        rowFeatures2[0][1] = targetFeatures[5];
        rowFeatures2[0][2] = targetFeatures[6];
        rowFeatures2[0][3] = targetFeatures[7];
        rightTargetFeatures[0] = targetFeatures[4];
        rightTargetFeatures[1] = targetFeatures[5];
        rightTargetFeatures[2] = targetFeatures[6];
        rightTargetFeatures[3] = targetFeatures[7];
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
                        if (!leftTargetFeatures[i])
                            rowFeatures[row][i] = targetFeatures[i] + 1;
                        else
                            rowFeatures[row][i] = leftTargetFeatures[i] + 1;
                        if (rowFeatures[row][i] > 5)
                            rowFeatures[row][i] = 1;
                        if (!rightTargetFeatures[i])
                            rowFeatures[row][i] = targetFeatures[i + 4] + 1; /* sic: not rowFeatures2 */
                        else
                            rowFeatures2[row][i] = rightTargetFeatures[i] + 1;
                        if (rowFeatures2[row][i] > 5)
                            rowFeatures2[row][i] = 1;
                        rowChanges[row][i] = rowFeatures[row][i];
                    } else if (randomBetween(0, 100) > 40 || i == 3 && count == 0) {
                        rowFeatures[row][i] = values[pick];
                        rowFeatures2[row][i] = values[pick];
                        rowChanges[row][i] = 0;
                    }
                    if (rowFeatures[row][i]) {
                        leftTargetFeatures[i] = rowFeatures[row][i];
                        rightTargetFeatures[i] = rowFeatures2[row][i];
                        count++;
                        for (j = pick; j < last + 1; j++)
                            values[j] = values[j + 1];
                        last--;
                    }
                }
        }
        limit = 2;
        if (smokeLevel == 4 || smokeLevel == 3) {
            leftFeatureMarks[0] = leftTargetFeatures[0];
            leftFeatureMarks[1] = leftTargetFeatures[1];
            leftFeatureMarks[2] = leftTargetFeatures[2];
            leftFeatureMarks[3] = leftTargetFeatures[3];
            rightFeatureMarks[0] = rightTargetFeatures[0];
            rightFeatureMarks[1] = rightTargetFeatures[1];
            rightFeatureMarks[2] = rightTargetFeatures[2];
            rightFeatureMarks[3] = rightTargetFeatures[3];
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
                                if (leftFeatureMarks[i]) {
                                    rowFeatures[row][i] = leftFeatureMarks[i];
                                    rowFeatures2[row][i] = rightFeatureMarks[i];
                                } else {
                                    rowFeatures[row][i] = targetFeatures[i];
                                    rowFeatures2[row][i] = targetFeatures[i + 4];
                                }
                            } else if (rowChanges[row - 1][i]) {
                                rowFeatures[row][i] = leftFeatureMarks[i] - 1;
                                if (rowFeatures[row][i] < 1)
                                    rowFeatures[row][i] = 5;
                                rowFeatures2[row][i] = rightFeatureMarks[i] - 1;
                                if (rowFeatures2[row][i] < 1)
                                    rowFeatures2[row][i] = 5;
                            } else if (rowFeatures[row - 1][i]) {
                                rowFeatures[row][i] = values[pick];
                                rowFeatures2[row][i] = values[pick];
                            } else if (leftFeatureMarks[i]) {
                                rowFeatures[row][i] = leftFeatureMarks[i];
                                rowFeatures2[row][i] = rightFeatureMarks[i];
                            } else {
                                rowFeatures[row][i] = targetFeatures[i];
                                rowFeatures2[row][i] = targetFeatures[i + 4];
                            }
                            rowChanges[row][i] = rowFeatures[row][i];
                        } else if (row == 3) {
                            if (!rowChanges[2][i] && rowFeatures[2][i]) {
                                rowFeatures[row][i] = leftFeatureMarks[i];
                                rowFeatures2[row][i] = rightFeatureMarks[i];
                            } else if (!rowChanges[1][i] && rowFeatures[1][i]) {
                                rowFeatures[row][i] = leftFeatureMarks[i];
                                rowFeatures2[row][i] = rightFeatureMarks[i];
                            } else {
                                rowFeatures[row][i] = 0;
                                rowFeatures2[row][i] = 0;
                            }
                            rowChanges[row][i] = 0;
                        } else if (rowChanges[row - 1][i]) {
                            if (!once) {
                                rowFeatures[row][i] = leftFeatureMarks[i] - 1;
                                if (rowFeatures[row][i] < 1)
                                    rowFeatures[row][i] = 5;
                                rowFeatures2[row][i] = rightFeatureMarks[i] - 1;
                                if (rowFeatures2[row][i] < 1)
                                    rowFeatures2[row][i] = 5;
                                once = 1;
                                rowChanges[row][i] = rowFeatures[row][i];
                            } else {
                                rowChanges[row][i] = 0;
                            }
                        } else if (rowFeatures[row - 1][i]) {
                            rowFeatures[row][i] = values[pick];
                            rowFeatures2[row][i] = values[pick];
                            rowChanges[row][i] = 0;
                        } else if (!rowChanges[2][i] && rowFeatures[2][i] || !rowChanges[1][i] && rowFeatures[1][i]) {
                            rowFeatures[row][i] = leftFeatureMarks[i];
                            rowFeatures2[row][i] = rightFeatureMarks[i];
                            rowChanges[row][i] = 0;
                        } else {
                            rowFeatures[row][i] = 0;
                            rowFeatures2[row][i] = 0;
                            rowChanges[row][i] = 0;
                        }
                        if (rowFeatures[row][i]) {
                            leftFeatureMarks[i] = rowFeatures[row][i];
                            rightFeatureMarks[i] = rowFeatures2[row][i];
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
            if (rowChanges[4][i]) {
                rowFeatures[row][i] = rowFeatures[4][i] - 1;
                if (rowFeatures[row][i] < 1)
                    rowFeatures[row][i] = 5;
            } else if (rowFeatures[4][i]) {
                rowFeatures[row][i] = randomBetween(1, 5);
            } else if (rowChanges[3][i]) {
                rowFeatures[row][i] = rowFeatures[3][i] - 1;
                if (rowFeatures[row][i] < 1)
                    rowFeatures[row][i] = 5;
            } else if (rowFeatures[3][i]) {
                rowFeatures[row][i] = randomBetween(1, 5);
            } else {
                rowFeatures[row][i] = leftFeatureMarks[i];
            }
        if (targetFeatures[4]) {
            row = 8;
            for (i = 0; i < 4; i++)
                if (rowChanges[4][i]) {
                    rowFeatures2[row][i] = rowFeatures2[4][i] - 1;
                    if (rowFeatures2[row][i] < 1)
                        rowFeatures2[row][i] = 5;
                } else if (rowFeatures2[4][i]) {
                    rowFeatures2[row][i] = randomBetween(1, 5);
                } else if (rowChanges[3][i]) {
                    rowFeatures2[row][i] = rowFeatures2[3][i] - 1;
                    if (rowFeatures2[row][i] < 1)
                        rowFeatures2[row][i] = 5;
                } else if (rowFeatures2[3][i]) {
                    rowFeatures2[row][i] = randomBetween(1, 5);
                } else {
                    rowFeatures2[row][i] = rightFeatureMarks[i];
                }
        }
        if (smokeLevel == 3) {
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
                            rowFeatures[row][i] = values[pick];
                            rowChanges[row][i] = values[pick];
                        } else if (randomBetween(0, 100) > 40 || i == 3 && count == 0) {
                            rowFeatures[row][i] = values[pick];
                            rowChanges[row][i] = 0;
                        }
                        if (rowFeatures[row][i]) {
                            count++;
                            for (j = pick; j < last + 1; j++)
                                values[j] = values[j + 1];
                            last--;
                        }
                    }
            }
        } else if (smokeLevel == 4) {
            if (randomBetween(0, 1)) {
                row2 = 5;
                row = randomBetween(1, 2);
                for (i = 0; i < 4; i++)
                    if (rowChanges[row][i]) {
                        rowFeatures[row2][i] = rowFeatures[row][i];
                        rowChanges[row2][i] = rowFeatures[row][i];
                    } else if (rowFeatures[row][i]) {
                        rowFeatures[row2][i] = rowFeatures[row][i];
                        rowFeatures[row2][i]++;
                        if (rowFeatures[row2][i] > 5)
                            rowFeatures[row2][i] = 1;
                        rowChanges[row2][i] = 0;
                    }
                row2 = 6;
                randomBetween(3, 4);
                chosen = randomBetween(0, 3);
                for (i = 0; i < 4; i++)
                    if (i == chosen) {
                        rowFeatures[row2][i] = randomBetween(1, 5);
                        rowChanges[row2][i] = 0;
                    }
            } else {
                row2 = 6;
                row = randomBetween(3, 4);
                for (i = 0; i < 4; i++)
                    if (rowChanges[row][i]) {
                        rowFeatures[row2][i] = rowFeatures[row][i];
                        rowChanges[row2][i] = rowFeatures[row][i];
                    } else if (rowFeatures[row][i]) {
                        rowFeatures[row2][i] = rowFeatures[row][i];
                        rowFeatures[row2][i]++;
                        if (rowFeatures[row2][i] > 5)
                            rowFeatures[row2][i] = 1;
                        rowChanges[row2][i] = 0;
                    }
                row2 = 5;
                randomBetween(1, 2);
                chosen = randomBetween(0, 3);
                for (i = 0; i < 4; i++)
                    if (i == chosen) {
                        rowFeatures[row2][i] = randomBetween(1, 5);
                        rowChanges[row2][i] = 0;
                    }
            }
        }
    }
    count = 0;
    for (i = 0; i < 4; i++) {
        if (n == 8) {
            if (targetFeatures[4])
                snoid->features[i] = rowFeatures2[n][i];
            else
                snoid->features[i] = 0;
        } else {
            snoid->features[i] = rowFeatures[n][i];
        }
        if (rowChanges[n][i])
            count = i + 1;
    }
    if (count) {
        snoid->idleTicks = 1;
        snoid->pose = count;
    } else {
        snoid->idleTicks = 0;
        snoid->pose = 0;
    }
}

/* Sets out the scene's Zoombinis: shuffles the places of views 1-6 (unless
   cheating), makes the puzzle's rows (makeSmokeRows) and gives each view its
   row's features and place, then fills slot 7 and slot 0. */
/* @zoombi32 0x004508db */
void setOutSmokeSnoids()
{
    short values[8];
    short i;
    short last;
    short pick;
    View *view;
    Snoid *snoid;

    for (i = 0; i < 8; i++) {
        values[i] = i;
        rowPlaceOrder[i] = i;
    }
    if (!cheatMode) {
        last = 6;
        for (i = 1; i < 7; i++) {
            pick = randomBetween(1, last);
            rowPlaceOrder[i] = values[pick];
            for (; pick < last + 1; pick++)
                values[pick] = values[pick + 1];
            last--;
        }
    }
    for (i = 1; i < rowViewCount; i++) {
        view = findView(rowViews[i]);
        if (view) {
            snoid = (Snoid *)&view->body;
            makeSmokeRows(snoid, i);
            if (i < 7) {
                snoid->angle = 7;
                *(Point *)&snoid->body.x = rowPlaces[rowPlaceOrder[i]];
            } else if (i == 7) {
                snoid->angle = 0;
                *(Point *)&snoid->body.x = madePlaces1[1];
            }
            if (i == 8) {
                snoid->angle = 2;
                *(Point *)&snoid->body.x = smokeRowStart;
            }
            snoid->action = 4;
        }
    }
    copyToSlotView(rowViews[7], 7);
    recordSlotFeatures(rowViews[7], 7);
    crossingSnoid = crossingViews[nextCrossing];
    copyToSlotView(crossingSnoid, 0);
    recordSlotFeatures(crossingSnoid, 0);
    rightRow = 0;
    leftRow = 0;
    fillMemory(&slotViews[0], 0, 12);
}

/* Adds `count` Zoombini views of one kind to the scene (1: random ones at
   randomPlaces, one of them with the features set in targetFeatures; 2: dealt ones;
   3: the puzzle's rows; 4 and 5: empty ones), recording them in that
   kind's list. */
/* @zoombi32 0x00452857 */
void addSmokeSnoids(short kind, short count)
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
            made->angle = 1;
            *(Point *)&made->body.x = randomPlaces[i];
            if (i == chosen) {
                if (targetFeatures[0])
                    made->features[0] = targetFeatures[0];
                else
                    made->features[0] = randomBetween(1, 5);
                if (targetFeatures[1])
                    made->features[1] = targetFeatures[1];
                else
                    made->features[1] = randomBetween(1, 5);
                if (targetFeatures[2])
                    made->features[2] = targetFeatures[2];
                else
                    made->features[2] = randomBetween(1, 5);
                if (targetFeatures[3])
                    made->features[3] = targetFeatures[3];
                else
                    made->features[3] = randomBetween(1, 5);
            }
            made->idleTicks = 0;
            made->pose = 0;
            made->action = 4;
            break;
        case 2:
            giveSlotFeatures(made, i);
            made->idleTicks = 0;
            made->pose = 0;
            made->action = 4;
            break;
        case 3:
            makeSmokeRows(made, i + 1);
            if (i + 1 < 7)
                made->angle = 7;
            else if (i + 1 == 7)
                made->angle = 5;
            if (i + 1 == 8)
                made->angle = 3;
            *(Point *)&made->body.x = rowPlaces[i + 1];
            made->action = 4;
            break;
        case 4:
            for (j = 0; j < 4; j++)
                made->features[j] = 0;
            if (i == 0) {
                made->angle = 8;
                *(Point *)&made->body.x = madePlaces1[i];
            }
            if (i == 1) {
                made->angle = 5;
                *(Point *)&made->body.x = madePlaces1[i];
            }
            made->idleTicks = 0;
            made->pose = 0;
            made->action = 4;
            break;
        case 5:
            for (j = 0; j < 4; j++)
                made->features[j] = 0;
            *(Point *)&made->body.x = madePlaces2[i];
            if (!i)
                made->angle = 5;
            else
                made->angle = 3;
            made->idleTicks = 0;
            made->pose = 0;
            made->action = 4;
            break;
        default:
            made->features[1] = 1;
            made->body.x = 10;
            made->body.y = 10;
            made->angle = 0;
            made->idleTicks = 0;
            made->pose = 0;
            break;
        }
        made->layers[0] = 1;
        made->layers[1] = 22;
        made->layers[2] = 43;
        made->layers[3] = 43;
        made->layers[4] = 22;
        made->layers[5] = 1;
        made->layers[6] = 1;
        made->layers[7] = 22;
        made->layers[8] = 43;
        made->facingLeft = 0;
        made->name[0] = 0;
        made->home = *(Point *)&made->body.x;
        *(Point *)&made->body.waypointX = *(Point *)&made->body.x;
        *(Point *)&made->targetX = *(Point *)&made->body.x;
        made->pathIndex = 0;
        made->path = 0;
        made->stepX = 0;
        made->stepY = 0;
        made->pathDirection = 0;
        made->chosen = 0;
        j = addSmokeSnoidView(made);
        if (j) {
            view = findView(j);
            if (view)
                view->flags = 0x4000002;
            switch (kind) {
            case 1:
                randomViews[randomViewCount] = j;
                randomViewCount++;
                moveView(j, 0, pairView2);
                break;
            case 2:
                dealtViews[dealtViewCount] = j;
                dealtViewCount++;
                moveView(j, 1, view11018);
                break;
            case 3:
                rowViews[rowViewCount] = j;
                rowViewCount++;
                moveView(j, 0, pairView2);
                if (rowViewCount == 8 && smokeLevel >= 3) {
                    copyToSlotView(rowViews[7], 7);
                    recordSlotFeatures(rowViews[7], 7);
                    randomDragSlot = 1;
                }
                if (rowViewCount == 9 && smokeLevel >= 3)
                    copyToSlotView(rowViews[8], 8);
                break;
            case 4:
                slotPairViews[slotPairViewCount] = j;
                slotPairViewCount++;
                moveView(j, 1, pairView2);
                break;
            case 5:
                comparedViews[comparedViewCount] = j;
                comparedViewCount++;
                moveView(j, 1, pairView1);
                if (view) {
                    view->body.running = 0;
                    view->changed = 1;
                }
                break;
            }
        }
    }
}

/* Adds the scene's Zoombini views for a level (1-4). */
/* @zoombi32 0x004527be */
void addLevelSnoids(short level)
{
    clearFeatureSlots();
    switch (level) {
    case 1:
        addSmokeSnoids(1, 8);
        addSmokeSnoids(4, 2);
        addSmokeSnoids(5, 2);
        break;
    case 2:
        addSmokeSnoids(2, 4);
        addSmokeSnoids(1, 8);
        addSmokeSnoids(4, 2);
        addSmokeSnoids(5, 2);
        break;
    case 3:
        addSmokeSnoids(3, 7);
        addSmokeSnoids(4, 1);
        addSmokeSnoids(5, 2);
        break;
    case 4:
        addSmokeSnoids(3, 8);
        addSmokeSnoids(4, 1);
        addSmokeSnoids(5, 2);
        break;
    }
}

/* The notify of view dealerView's scripts (level 4): 17 and 18 step view
   rowViews[8]'s Zoombini back along madePlaces1 (18 also empties it and
   moves the features along); 19 sets out the Zoombinis again. */
/* @zoombi32 0x00451e5d */
void stepBackNotify(View *, short event)
{
    View *view;

    switch (event) {
    case 17:
        view = findView(rowViews[8]);
        if (view) {
            Snoid *snoid = (Snoid *)&view->body;

            level4Stage--;
            *(Point *)&snoid->body.x = madePlaces1[level4Stage];
            snoid->angle = 4;
            snoid->action = 4;
        }
        break;
    case 18:
        view = findView(rowViews[8]);
        if (view) {
            Snoid *snoid = (Snoid *)&view->body;
            ViewBody *body = &snoid->body;

            body->cels[0].image = 0;
            level4Stage--;
            *(Point *)&snoid->body.x = madePlaces1[level4Stage];
            snoid->angle = 5;
            snoid->action = 4;
            recordSlotFeatures(rowViews[8], 7);
        }
        recordLeftSlots();
        advanceLeftFeatures();
        recordRightSlots();
        advanceRightFeatures();
        setPairFeatures();
        if (dealerRunning)
            dealerRunning = 0;
        break;
    case 19:
        setOutSmokeSnoids();
        dealLightDue = 1;
        break;
    }
}

/* Starts a round: empties the slot views, and deals new Zoombinis for the
   level (smokeLevel). */
/* Not exact: at level 4 the original keeps `snoid` in edx and `body` in
   eax; this has them the other way round. */
/* @zoombi32 0x004507e0 */
short startRound()
{
    View *view;
    Snoid *snoid;
    ViewBody *body;

    emptySlotView(0);
    if (smokeLevel == 1 || smokeLevel == 2)
        emptySlotView(1);
    else
        emptySlotView(7);
    emptyPairViews();
    clearFeatureSlots();
    switch (smokeLevel) {
    case 1:
        dealRandomFeatures(featuresTaken);
        break;
    case 2:
        dealFeatures();
        dealRandomFeatures(featuresTaken);
        break;
    case 3:
        setOutSmokeSnoids();
        break;
    case 4:
        view = findView(rowViews[7]);
        if (view) {
            snoid = (Snoid *)&view->body;
            body = &snoid->body;

            body->cels[0].image = 0;
            snoid->action = 2;
        }
        view = findView(rowViews[8]);
        if (view) {
            snoid = (Snoid *)&view->body;
            body = &snoid->body;

            body->cels[0].image = 0;
        }
        view = findView(dealerView);
        if (view) {
            setViewScript(view, dealerScript2, 1);
            view->notify = stepBackNotify;
            dealerRunning = 0;
        }
        level4Stage = 3;
        break;
    }
    return 2;
}

/*
 * Opens the scene: resets its state, picks the level (from sceneLevel, 1-4)
 * and its scripts, opens Smoke.MHK, adds its views and the Zoombinis'
 * views, takes the features of the Zoombini (levels 1 and 2: the one picked
 * at random) or two (levels 3 and 4) of the party that the puzzle is built
 * from, sets the last two of the party walking in, and starts the
 * opening scripts and sounds.
 */
/* @zoombi32 0x0044e494 */
void openSmoke()
{
    short *scripts = pairScripts;
    short *others = moverScripts;
    short pick;
    short count;
    short i;
    short j;
    View *view;
    View *other;

    sceneDue = 0;
    smokeOpen = 0;
    smokeGoReady = 0;
    movePlace = 0;
    randomDragSlot = 0;
    crossingSnoid = 0;
    crossOnceFlag = 1;
    dealButtonState = 2;
    crossingCount = 0;
    leftRow = 0;
    rightRow = 0;
    dealtViewCount = 0;
    slotPairViewCount = 0;
    comparedViewCount = 0;
    crossedCount = 0;
    nextCrossing = 0;
    useSecondMover = 0;
    level4Stage = 3;
    unusedSmoke1 = 0;
    crossedChosen = 0;
    pairMismatch = 0;
    crossedDue = 0;
    unusedSmoke2 = 0;
    anchorDue = 0;
    dealLightDue = 0;
    dealDimDue = 0;
    unusedSmoke3 = 0;
    featuresTaken = 0;
    leadersDue = 0;
    hintSound = 0;
    cheatMode = 0;
    unusedSmoke4 = 0;
    lastSmokeFidgetTime = 0;
    smokeFidgetersUsed = 0;
    smokeFidgets = 0;
    smokeFidgeting = 0;
    soundOnBeforeSmoke = soundOn;
    soundOn = 0;
    smokeDragOrigin = smokeDragOriginStart;
    for (i = 0; i < 8; i++)
        rowPlaceOrder[i] = i;
    fillMemory(&slotViews[0], 0, 12);
    fillMemory(&view11009, 0, 42);
    fillMemory(crossedMarkers, 0, 40);
    fillMemory(crossedSnoids, 0, 40);
    fillMemory(crossingViews, 0, 42);
    fillMemory(rowViews, 0, 18);
    smokeLevel = sceneLevel() + 1;
    if (smokeLevel > 4)
        smokeLevel = 4;
    pairScriptIndex = 0;
    slotsDiffer = 0;
    if (smokeLevel == 1 || smokeLevel == 2) {
        scripts[0] = 11024;
        scripts[1] = 11025;
        scripts[2] = 11026;
        scripts[3] = 11027;
        crossScripts[0] = 11999;
        crossScripts[1] = 12004;
        others[0] = 11032;
        others[1] = 0;
        others[2] = 0;
        crossScript1 = 0;
        crossScript2 = 0;
        crossScript3 = 0;
    } else {
        scripts[0] = 11028;
        scripts[1] = 11029;
        scripts[2] = 11030;
        scripts[3] = 11031;
        crossScripts[0] = 12009;
        crossScripts[1] = 12014;
        others[0] = 11033;
        others[1] = 11034;
        others[2] = 11035;
        crossScript1 = 12038;
        crossScript2 = 12039;
        crossScript3 = 12040;
    }
    if (smokeLevel != 4) {
        dealerScript = 11013;
        dealerScript2 = 0;
    } else {
        dealerScript = 11011;
        dealerScript2 = 11012;
    }
    openGameFile(&smokeFile, "Smoke.MHK");
    setCurrentMap(smokeFile);
    loadTerrain(100);
    drawBackdrop(5000);
    loadFeatureGroup(11000, 0, 0);
    loadScripts(11000, 78);
    loadSnoidScripts(11999, 1, 0);
    addSnoidScripts(12000, 50, 0);
    loadShape(&smokeButtonResource, 6000, "Map/Go Buttons");
    dealerView = addView(0x4188000, drawCels, runViewScript, dealerScript, 10, 0, 0, 0);
    if (smokeLevel == 1 || smokeLevel == 2)
        view11076 = addView(0x5188000, drawCels, runViewScript, 11076, 10, 0, 0, 0);
    leftRowView = addView(0x5188000, drawCels, runViewScript, 11006, 10, 0, 0, 0);
    rightRowView = addView(0x5188000, drawCels, runViewScript, 11007, 10, 0, 0, 0);
    pairView1 = addView(0x5188000, drawCels, runViewScript, scripts[pairScriptIndex], 6, 0, 0, 0);
    moverView1 = addView(0x4108000, drawCels, runViewScript, others[0], 6, 0, 0, 0);
    if (smokeLevel == 3 || smokeLevel == 4)
        moverView2 = addView(0x4108000, drawCels, runViewScript, others[1], 6, 0, 0, 0);
    {
        View *added = findView(moverView1);

        if (added)
            added->notify = smokeViewNotify;
    }
    pairView2 = addView(0xd180000, drawCels, runViewScript, scripts[pairScriptIndex + 1], 6, 0, 0, 0);
    view11018 = addView(0x5180000, drawCels, runViewScript, 11018, 6, 0, 0, 0);
    view11019 = addView(0xd180000, drawCels, runViewScript, 11019, 6, 0, 0, 0);
    view11009 = addView(0x4000000, drawCels, runViewScript, 11009, 6, 0, 0, 0);
    view11036 = addView(0x5180000, drawCels, runViewScript, 11036, 6, 0, 0, 0);
    view11008 = addView(0x4100000, drawCels, runViewScript, 11008, 0, 0, 0, 0);
    dealButtonView = addView(0x4180000, drawCels, runViewScript, 11002, 5, 0, 0, 0);
    view11077 = addView(0x4100000, drawCels, runViewScript, 11077, 0, 0, 0, 0);
    addView(0x1000, drawSmokeButtons, updateSmokeButtons, 0, 0, 0, 0, 0);
    smokeImages = loadImageBank(10000, &smokeImagesResource);
    smokeHotSpotsX = loadShortTable(10000, &smokeHotSpotsXResource);
    smokeHotSpotsY = loadShortTable(10001, &smokeHotSpotsYResource);
    setViewPlaces(20, smokePlaces, 1);
    makePartySnoids(0);
    randomViewCount = 0;
    rowViewCount = 1;
    dealtViewCount = 0;
    slotPairViewCount = 0;
    comparedViewCount = 0;
    crossingCount = listChosenSnoids()->count;
    count = 0;
    if (crossingCount > 0) {
        j = 0;
        i = 0;
        pick = randomBetween(0, crossingCount - 1);
        for (view = viewListEnd(1); view; view = view->next)
            if (view->flags == 1) {
                crossingViews[i] = view->id;
                i++;
                Snoid *snoid = (Snoid *)&view->body;

                if (smokeLevel < 3) {
                    if (!pick) {
                        targetFeatures[j * 4] = snoid->features[0];
                        targetFeatures[j * 4 + 1] = snoid->features[1];
                        targetFeatures[j * 4 + 2] = snoid->features[2];
                        targetFeatures[j * 4 + 3] = snoid->features[3];
                    }
                    pick--;
                    count++;
                    if (count == crossingCount - 1 || count == crossingCount) {
                        snoid = (Snoid *)&view->body;
                        snoid->angle = 0;
                        setSnoidAction((Snoid *)&view->body, 7, 0);
                        view->body.y = 79;
                        ((Snoid *)&view->body)->targetY = 79;
                        if (count == crossingCount) {
                            view->body.x = 45;
                            ((Snoid *)&view->body)->targetX = 160;
                        } else {
                            view->body.x = 110;
                            ((Snoid *)&view->body)->targetX = 200;
                        }
                    }
                } else {
                    view->body.running = 0;
                    if (j < 2) {
                        targetFeatures[j * 4] = snoid->features[0];
                        targetFeatures[j * 4 + 1] = snoid->features[1];
                        targetFeatures[j * 4 + 2] = snoid->features[2];
                        targetFeatures[j * 4 + 3] = snoid->features[3];
                        j++;
                    }
                }
            }
        addLevelSnoids(smokeLevel);
    }
    if (smokeLevel < 3)
        placedViews[0] = addView(0x108a000, drawCels, runViewScript, 11001, 7, &smokeSpotPoint, 0, 0);
    fadeOutViews();
    copyPaletteRange(10, 236);
    updateViews();
    copyPaletteRange(10, 236);
    setGroupLists(&smokeGroups, 1, -0x4000);
    drawSmokeButton(1, 0, 0);
    drawSmokeButton(2, 0, 0);
    soundOn = soundOnBeforeSmoke;
    view11017Due = 0;
    view = findView(pairView1);
    setViewScript(view, 11015, 1);
    loadViewSounds(view->id, 1);
    view->notify = smokeViewNotify;
    other = findView(pairView2);
    setViewScript(other, 11016, 1);
    other->notify = smokeViewNotify;
    groupViews(view->id, other->id, 0, 0, 0, 0);
    showRect(&shownGameRect);
    fadeInViews();
    chooseSnoids(0, 0);
    resetViewClock();
    smokeOpen = 1;
    addSoundRange(996, 997, 0);
    addSoundRange(20000, 29999, 1);
    if (smokeLevel != 3) {
        addSoundRange(11008, 11009, 0);
        addSoundRange(11001, 11001, 0);
        addSoundRange(11013, 11013, 0);
        addSoundRange(11016, 11016, 0);
        addSoundRange(125, 149, 0);
        addSoundRange(450, 474, 0);
        addSoundRange(11011, 11012, 0);
        addSoundRange(11014, 11015, 0);
        addSoundRange(11010, 11010, 0);
        addSoundRange(11002, 11004, 0);
        addSoundRange(11007, 11007, 0);
        addSoundRange(11006, 11006, 0);
        addSoundRange(11005, 11005, 0);
        addSoundRange(11000, 11000, 0);
    } else {
        addSoundRange(11017, 11017, 0);
        addSoundRange(11001, 11001, 0);
        addSoundRange(11013, 11013, 0);
        addSoundRange(11016, 11016, 0);
        addSoundRange(125, 149, 0);
        addSoundRange(450, 474, 0);
        addSoundRange(11011, 11012, 0);
        addSoundRange(11014, 11015, 0);
        addSoundRange(11010, 11010, 0);
        addSoundRange(11002, 11004, 0);
        addSoundRange(11007, 11007, 0);
        addSoundRange(11006, 11006, 0);
        addSoundRange(11005, 11005, 0);
        addSoundRange(11000, 11000, 0);
    }
    queueViewSound(sceneLevel() + 30030, 0);
    campHint((short *)(gameState + 0x42));
    hintSound = randomBetween(20066, 20067);
    if (smokeLevel == 3 || smokeLevel == 4) {
        dealLit = 1;
        lightDealButton(11003);
    }
}

/*
 * The scene's clicks: 1 the map button (leave), 2 the go button, 3 a click
 * on the scene: on the spot button (dealButtonRect) it deals a new round or ends
 * one; otherwise it drags the party's Zoombinis to and from the scene's
 * slots and snaps them into place, and (levels 1 and 2) the Zoombinis it
 * deals into the machine's slot, or (levels 3 and 4) the rows' Zoombinis
 * into the two lines of slots (slotViews).
 */
/* @zoombi32 0x0044fa57 */
void smokeClicked(short action)
{
    Point where;
    short spot;
    short k;
    volatile short placed;
    short result;
    View *view;
    Snoid *snoid;
    short ok;
    short best;
    short distance;
    short m;
    View *other;

    if (sceneDue) {
        pendingScene = sceneDue;
        sceneDue = 0;
        setCurrentMap(0);
        closeSmoke();
        return;
    }
    switch (action) {
    case 1:
        queueViewSound(999, 0);
        drawSmokeButton(action, 1, 1);
        waitForEventFor(0, 2, 0, 1);
        drawSmokeButton(action, 0, 1);
        snoidsArrived = 1;
        sceneDue = 1;
        askKeepParty();
        break;
    case 2:
        if (smokeGoReady) {
            queueViewSound(0, 0);
            drawSmokeButton(action, 1, 1);
            waitForEventFor(0, 2, 0, 1);
            drawSmokeButton(action, 0, 1);
            leadersDue = 1;
            snoidsOnTheirWay = 1;
            snoidsArrived = 0;
            sceneDue = 18;
        }
        break;
    case 3:
        if (snoidsOnTheirWay > 0 || crossedCount >= crossingCount)
            break;
        getCursorPosition(&where);
        if (dealButtonState == 1) {
            if (ptInRect(&dealButtonRect, where)) {
                dealButtonState = startRound();
                pressDealButton(11004, 1);
                requestViewSort();
            }
            break;
        }
        if (!dealButtonState)
            break;
        if (ptInRect(&dealButtonRect, where) && dealLit == 1) {
            dealLit = 0;
            dealButtonState = 0;
            pressDealButton(11002, 1);
            placeClaims[0] = 0;
            releaseHeldPlace();
            claimPlacedView(heldPlaceNumber(), 0);
            setViewsLocked(0);
            break;
        }
        view = viewAt(where, 1, 1);
        if (view) {
            ok = 1;
            for (k = 0; k < crossingCount; k++)
                if (crossedSnoids[k] == view->id || smokeLevel >= 3) {
                    ok = 0;
                    k = crossingCount;
                }
            if (ok && view->body.running) {
                smokeDragOrigin = *(long *)&view->body.x;
                if (view->id == crossingSnoid) {
                    placeClaims[0] = 0;
                    releaseHeldPlace();
                    claimPlacedView(heldPlaceNumber(), 0);
                    crossingSnoid = 0;
                    emptySlotView(0);
                    clearFeatureSlot(0);
                    dealLit = 0;
                    dimDealButton(11002);
                    pickFreeSpot(&spot);
                    smokeDragOrigin = *(long *)&smokePlaces[spot];
                }
                dragSnoid(view, where, 0, 0);
                if (!heldPlaceNumber() && !ptInRect(&smokeWaitArea, *(Point *)&view->body.x)) {
                    *(long *)&((Snoid *)&view->body)->targetX = smokeDragOrigin;
                } else if (ptInRect(&smokeWaitArea, *(Point *)&view->body.x)) {
                    best = smokeWaitRows[0];
                    distance = MAGNITUDE(((Snoid *)&view->body)->targetY - smokeWaitRows[0]);
                    for (k = 1; k < 5; k++)
                        if (MAGNITUDE(((Snoid *)&view->body)->targetY - smokeWaitRows[k]) < distance) {
                            best = smokeWaitRows[k];
                            distance = MAGNITUDE(((Snoid *)&view->body)->targetY - smokeWaitRows[k]);
                        }
                    ((Snoid *)&view->body)->targetY = best;
                }
                if (heldPlaceNumber() && !crossingSnoid) {
                    crossingSnoid = view->id;
                    copyToSlotView(crossingSnoid, 0);
                    recordSlotFeatures(crossingSnoid, 0);
                    if (randomDragSlot > 0) {
                        dealLit = 1;
                        lightDealButton(11003);
                    }
                }
            }
        }
        view = viewAt(where, 2, 1);
        if (!view)
            break;
        switch (smokeLevel) {
        case 1:
        case 2:
            if (ptInRect(&spot4Rect, where)) {
                if (randomDragSlot) {
                    dealLit = 0;
                    dimDealButton(11002);
                    clearFeatureSlot(7);
                    emptySlotView(1);
                    view = findView(randomViews[randomDragSlot - 1]);
                    if (randomDragSlot) {
                        view->changed = 1;
                        view->body.running = 1;
                    }
                    action = randomDragSlot;
                    randomDragSlot = 0;
                    if (dragSnoidToSpot(view, where) == 4) {
                        randomDragSlot = action;
                        copyToSlotView(view->id, 1);
                        recordSlotFeatures(view->id, 7);
                    } else {
                        randomDragSlot = action;
                        snoid = (Snoid *)&view->body;
                        *(Point *)&snoid->body.x = randomPlaces[randomDragSlot - 1];
                        snoid->action = 4;
                        randomDragSlot = 0;
                    }
                }
            } else {
                for (k = 0; k < randomViewCount; k++)
                    if (randomViews[k] == view->id) {
                        if (dragSnoidToSpot(view, where) == 4) {
                            if (!randomDragSlot) {
                                randomDragSlot = k + 1;
                                copyToSlotView(view->id, 1);
                                recordSlotFeatures(view->id, 7);
                            } else {
                                snoid = (Snoid *)&view->body;
                                *(Point *)&snoid->body.x = randomPlaces[k];
                                snoid->action = 4;
                            }
                        } else {
                            snoid = (Snoid *)&view->body;
                            *(Point *)&snoid->body.x = randomPlaces[k];
                            snoid->action = 4;
                        }
                        k = randomViewCount;
                    }
            }
            if (randomDragSlot && crossingSnoid) {
                dealLit = 1;
                lightDealButton(11003);
            }
            stopView11076();
            break;
        case 3:
        case 4:
            for (k = 1; k < rowViewCount && k < 7; k++)
                if (rowViews[k] == view->id) {
                    pressDealButton(11002, 0);
                    dealLit = 0;
                    snoid = (Snoid *)&view->body;
                    snoid->angle = 7;
                    if (snoid->pose) {
                        snoid->idleTicks = 1;
                        view->tag = 0;
                    }
                    for (m = 0; m < 6; m++)
                        if (slotViews[m] == view->id) {
                            placed = 0;
                            slotViews[m] = 0;
                            if (m > 2) {
                                for (action = m; action <= 5; action++)
                                    if (action < 5)
                                        slotViews[action] = slotViews[action + 1];
                                    else
                                        slotViews[action] = 0;
                                rightRow -= 2;
                                if (rightRow < 0)
                                    rightRow = 0;
                                for (action = 3; action <= 5; action++) {
                                    other = findView(slotViews[action]);
                                    if (other) {
                                        Snoid *moved = (Snoid *)&other->body;

                                        *(Point *)&moved->body.x = rightRowPlaces[rightRow][placed];
                                        moved->action = 4;
                                        placed++;
                                    }
                                }
                                if (placed)
                                    rightRow++;
                                recordRightSlots();
                                advanceRightFeatures();
                            } else {
                                for (action = m; action <= 2; action++)
                                    if (action < 2)
                                        slotViews[action] = slotViews[action + 1];
                                    else
                                        slotViews[action] = 0;
                                leftRow -= 2;
                                if (leftRow < 0)
                                    leftRow = 0;
                                for (action = 0; action < 3; action++) {
                                    other = findView(slotViews[action]);
                                    if (other) {
                                        Snoid *moved = (Snoid *)&other->body;

                                        *(Point *)&moved->body.x = leftRowPlaces[leftRow][placed];
                                        moved->action = 4;
                                        placed++;
                                    }
                                }
                                if (placed)
                                    leftRow++;
                                recordLeftSlots();
                                advanceLeftFeatures();
                            }
                            m = 6;
                        }
                    action = view->id;
                    result = dragSnoidToSpot(view, where);
                    view = findView(action);
                    snoid = (Snoid *)&view->body;
                    if (result >= 0 && result <= 5) {
                        if (leftRow < 3 && result < 3 || rightRow < 3 && result > 2) {
                            placed = 0;
                            if (result < 3) {
                                snoid->angle = 0;
                                if (slotViews[result])
                                    for (action = leftRow; action >= result; action--)
                                        if (action > 0)
                                            slotViews[action] = slotViews[action - 1];
                                slotViews[result] = view->id;
                                for (action = 0; action <= 2; action++) {
                                    other = findView(slotViews[action]);
                                    if (other) {
                                        Snoid *moved = (Snoid *)&other->body;

                                        *(Point *)&moved->body.x = leftRowPlaces[leftRow][placed];
                                        moved->action = 4;
                                        placed++;
                                    }
                                }
                                leftRow++;
                                slotViews[result] = view->id;
                                recordLeftSlots();
                                advanceLeftFeatures();
                            } else {
                                snoid->angle = 2;
                                if (slotViews[result])
                                    for (action = rightRow + 3; action >= result; action--)
                                        if (action > 3)
                                            slotViews[action] = slotViews[action - 1];
                                slotViews[result] = view->id;
                                for (action = 3; action <= 5; action++) {
                                    other = findView(slotViews[action]);
                                    if (other) {
                                        Snoid *moved = (Snoid *)&other->body;

                                        *(Point *)&moved->body.x = rightRowPlaces[rightRow][placed];
                                        moved->action = 4;
                                        placed++;
                                    }
                                }
                                rightRow++;
                                slotViews[result] = view->id;
                                recordRightSlots();
                                advanceRightFeatures();
                            }
                        }
                    } else {
                        snoid = (Snoid *)&view->body;
                        *(Point *)&snoid->body.x = rowPlaces[rowPlaceOrder[k]];
                        snoid->action = 4;
                        if (snoid->pose) {
                            snoid->idleTicks = 1;
                            view->tag = 0;
                        }
                    }
                    k = rowViewCount;
                }
            stopRowViews();
            dealLit = 1;
            lightDealButton(11003);
            break;
        }
        break;
    }
}
