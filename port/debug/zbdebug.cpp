/*
 * The port's debug tooling (built when ZB_DEBUG is defined; see
 * docs/src/port/debug-tools.md): a queue of text commands, run from the game's
 * frame hook (gameFrame calls zbDebugFrame), to get into any state quickly:
 * jump to a scene, set the levels, fill the party, edit the saved state.
 *
 * Commands are separated by ';' or newlines; `#` starts a comment. They come
 * from the command line (--cmd, --script), the URL (?cmd=) and the page's
 * console (zbDebug("..."), which only queues the text in JavaScript: zbDebugFrame
 * collects it, so nothing calls into the program while Asyncify has it
 * suspended). A command runs when the game is at rest (active, no dialog, no
 * scene change pending); one that changes the scene holds up the ones after it
 * until the scene has opened.
 *
 * This is port code, not decompiled code: it uses the game's globals and
 * functions as the decompilation declares them, and nothing here is part of
 * the original.
 */

#include "zoombinis.h"
#include "basecamp.h"
#include "mainloop.h"
#include "net.h"
#include "roster.h"
#include "snoids.h"

#include <deque>
#include <string>
#include <utility>
#include <vector>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

/* The next text the page's zbDebug() queued (web/pre.js), copied to buffer: 0 if none. */
EM_JS_DEPS(zbDebugDeps, "$stringToUTF8,$lengthBytesUTF8");
EM_JS(int, takePageCommands, (char *buffer, int size), {
    var queue = Module.zbDebugQueue;
    if (!queue || !queue.length)
        return 0;
    var text = queue.shift();
    if (lengthBytesUTF8(text) + 1 > size) {
        console.warn('zbDebug: too long, ignored');
        return 0;
    }
    stringToUTF8(text, buffer, size);
    return 1;
});
#endif

namespace {

std::deque<std::string> pending;
DWORD waitUntil;
bool waitingForTime;
short waitingForScene = -2; /* -2: not waiting */
short assertionsFailed;
long debugPartyCount = -1; /* the last `party N`: -1 if none */
int busy; /* how many scene callbacks (open, close, frame, key) are running */
bool sceneChanged; /* a command changed the scene this pass */

std::vector<std::string> words(const std::string &line)
{
    std::vector<std::string> result;
    size_t i = 0;

    while (i < line.size()) {
        while (i < line.size() && isspace((unsigned char)line[i]))
            i++;
        size_t start = i;
        while (i < line.size() && !isspace((unsigned char)line[i]))
            i++;
        if (i > start)
            result.push_back(line.substr(start, i - start));
    }
    return result;
}

bool number(const std::string &text, long *value)
{
    char *end;

    if (text.empty())
        return false;
    *value = strtol(text.c_str(), &end, 0);
    return *end == 0;
}

void say(const char *format, ...)
{
    va_list arguments;

    printf("[zbdebug] ");
    va_start(arguments, format);
    vprintf(format, arguments);
    va_end(arguments);
    printf("\n");
    fflush(stdout);
}

/* Reads or writes 1, 2 or 4 bytes of the saved state at an offset. */
bool stateAccess(long offset, long size, bool write, long *value)
{
    if (offset < 0 || offset + size > 0xae05 || (size != 1 && size != 2 && size != 4))
        return false;
    unsigned char *at = (unsigned char *)gameState + offset;
    if (write) {
        for (long i = 0; i < size; i++)
            at[i] = (unsigned char)((unsigned long)*value >> (8 * i));
    } else {
        unsigned long result = 0;
        for (long i = 0; i < size; i++)
            result |= (unsigned long)at[i] << (8 * i);
        if (size == 1)
            result = (unsigned char)result;
        *value = (long)result;
    }
    return true;
}

/* Names a value `assert` and `get` can read: a field of the game, or state:<offset>[:<size>]. */
bool lookup(const std::string &name, long *value)
{
    if (name == "scene")
        *value = currentScene;
    else if (name == "pending")
        *value = pendingScene;
    else if (name == "practice")
        *value = practiceLevel;
    else if (name == "party")
        *value = party()->count;
    else if (name == "debug")
        *value = debugMessagesOn;
    else if (name == "dialog")
        *value = dialogFlags;
    else if (name.compare(0, 5, "level") == 0 && name.size() == 6 && name[5] >= '1' && name[5] <= '4')
        *value = puzzleLevels()[name[5] - '0'] + 1;
    else if (name.compare(0, 6, "state:") == 0) {
        long offset, size = 1;
        std::string rest = name.substr(6);
        size_t colon = rest.find(':');
        if (colon != std::string::npos) {
            if (!number(rest.substr(colon + 1), &size))
                return false;
            rest = rest.substr(0, colon);
        }
        return number(rest, &offset) && stateAccess(offset, size, false, value);
    } else
        return false;
    return true;
}


/*
 * A scene's open, close, frame and key callbacks can run the main loop
 * themselves while they wait (for a sound, an animation, a click), which calls
 * the frame hook again from inside them. A command run there would change the
 * scene under the callback that's running (closing a scene half-way through
 * opening it). So each scene's callbacks are wrapped to count themselves, and
 * commands wait until none is running.
 */
Scene original[22];

template <int N> void openScene()
{
    busy++;
    original[N].open();
    busy--;
}
template <int N> void closeScene()
{
    busy++;
    original[N].close();
    busy--;
}
template <int N> void frameScene()
{
    busy++;
    original[N].frame();
    busy--;
}
template <int N> short keyScene(unsigned short key)
{
    busy++;
    short handled = original[N].key(key);
    busy--;
    return handled;
}

template <int N> void wrapScene()
{
    Scene *scene = scenes[N];

    for (int i = 0; i < N; i++)
        if (scenes[i] == scene)
            return; /* (scenes 19 and 21 are one) */
    original[N] = *scene;
    if (scene->open)
        scene->open = openScene<N>;
    if (scene->close)
        scene->close = closeScene<N>;
    if (scene->frame)
        scene->frame = frameScene<N>;
    if (scene->key)
        scene->key = keyScene<N>;
}

template <int... N> void wrapScenes(std::integer_sequence<int, N...>)
{
    (wrapScene<N>(), ...);
}

/* (Done before the game starts, so no callback is already running.) */
struct SceneWrapper
{
    SceneWrapper() { wrapScenes(std::make_integer_sequence<int, 22>()); }
} sceneWrapper;

/* A party of n Zoombinis, all on board, each of a different kind (hair, eyes, nose and feet in turn). */
void makeParty(long n)
{
    if (n < 0)
        n = 0;
    if (n > 16)
        n = 16;
    party()->count = (short)n;
    party()->unknown2 = party()->unknown4 = 0;
    for (long i = 0; i < n; i++) {
        Traveller *traveller = &travellers()[i];
        long kind = i * 37 % 625; /* spread over the 625 kinds */

        memset(traveller, 0, sizeof *traveller);
        /* Features are 1-5 in a Traveller, 0-4 as the counts' indexes. */
        char hair = (char)(kind / 125 % 5), eyes = (char)(kind / 25 % 5), nose = (char)(kind / 5 % 5),
             feet = (char)(kind % 5);
        traveller->features[0] = hair + 1;
        traveller->features[1] = eyes + 1;
        traveller->features[2] = nose + 1;
        traveller->features[3] = feet + 1;
        traveller->onboard = 1;
        sprintf(traveller->name, "Debug%ld", i);
        zoombiniCounts()[hair][eyes][nose][feet] = 1;
    }
}

/* Leaves the current scene as the game does (each scene closes itself before it
   sets pendingScene: enterNextScene only opens the next, so without this the
   old scene's views, sounds and resources stay), makes the party again (a
   scene's closing can empty it), then opens scene N. */
void changeScene(short scene, bool viaMap)
{
    if (currentScene != -1 && scenes[currentScene]->close)
        scenes[currentScene]->close();
    if (debugPartyCount >= 0)
        makeParty(debugPartyCount);
    skipJourneyMap = !viaMap;
    pendingScene = scene;
    waitingForScene = scene;
    enterNextScene();
    sceneChanged = true;
}

void fillRecords(long count)
{
    for (long i = 0; i < 16; i++) {
        bool used = i < count;

        recordGroups()[i] = used ? (char)(((i / 4) & 3) + 1) : 0;
        recordLevels()[i] = used ? (char)((i & 3) + 1) : 0;
        recordYears()[i] = used ? 1996 : 0;
        recordMonths()[i] = used ? 1 : 0;
        recordDays()[i] = used ? 1 : 0;
    }
}

void help()
{
    say("commands: debug on|off | scene N [map] | level G L | practice L (0 off) | party N"
        " | unlock | records N | state get|set OFFSET [VALUE] [SIZE] | cheatcode HASH CODE"
        " | key CODE | roster save | roster load | wait MS | wait scene N | get NAME"
        " | assert NAME VALUE | dump | quit | help");
}

void dump()
{
    say("scene=%d pending=%d practice=%d party=%d debug=%d levels=%d,%d,%d,%d dialog=%d", currentScene,
        pendingScene, practiceLevel, party()->count, debugMessagesOn, puzzleLevels()[1] + 1,
        puzzleLevels()[2] + 1, puzzleLevels()[3] + 1, puzzleLevels()[4] + 1, dialogFlags);
}

void fail(const std::string &line, const char *why)
{
    say("error: %s: %s", why, line.c_str());
    assertionsFailed++;
}

/* Runs one command. */
void run(const std::string &line)
{
    std::vector<std::string> w = words(line);
    long a, b, c;

    if (w.empty())
        return;
    const std::string &command = w[0];
    size_t n = w.size();

    if (command == "debug" && n == 2) {
        debugMessagesOn = w[1] == "on";
        debugMode = debugMessagesOn;
    } else if (command == "scene" && n >= 2 && number(w[1], &a) && a >= 0 && a <= 21) {
        changeScene((short)a, n >= 3 && w[2] == "map");
    } else if (command == "level" && n == 3 && number(w[1], &a) && number(w[2], &b) && a >= 1 && a <= 4
               && b >= 1 && b <= 4) {
        puzzleLevels()[a] = (short)(b - 1);
    } else if (command == "practice" && n == 2 && number(w[1], &a) && a >= 0 && a <= 4) {
        practiceLevel = (short)a;
    } else if (command == "party" && n == 2 && number(w[1], &a)) {
        debugPartyCount = a < 0 ? 0 : a > 16 ? 16 : a;
        makeParty(a);
    } else if (command == "unlock" && n == 1) {
        gameState[0x50] |= 0xf;
        gameState[0x51] |= 0xf;
        *(short *)(gameState + 0x52) |= 0xff;
    } else if (command == "records" && n == 2 && number(w[1], &a)) {
        fillRecords(a);
    } else if (command == "state" && n >= 3 && w[1] == "get" && number(w[2], &a)) {
        b = 1;
        if (n >= 4 && !number(w[3], &b))
            return fail(line, "bad size");
        if (!stateAccess(a, b, false, &c))
            return fail(line, "bad state access");
        say("state[0x%lx] = %ld (0x%lx)", a, c, c);
    } else if (command == "state" && n >= 4 && w[1] == "set" && number(w[2], &a) && number(w[3], &c)) {
        b = 1;
        if (n >= 5 && !number(w[4], &b))
            return fail(line, "bad size");
        if (!stateAccess(a, b, true, &c))
            return fail(line, "bad state access");
    } else if (command == "cheatcode" && n == 3 && number(w[1], &a) && number(w[2], &b)) {
        cheatHash = a;
        cheatCode = b;
    } else if (command == "key" && n == 2 && number(w[1], &a)) {
        gameKey((unsigned short)a);
    } else if (command == "roster" && n == 2 && w[1] == "save") {
        saveRoster();
    } else if (command == "roster" && n == 2 && w[1] == "load") {
        readRoster();
    } else if (command == "wait" && n == 2 && number(w[1], &a)) {
        waitUntil = GetTickCount() + (DWORD)a;
        waitingForTime = true;
    } else if (command == "wait" && n == 3 && w[1] == "scene" && number(w[2], &a)) {
        waitingForScene = (short)a;
    } else if (command == "get" && n == 2) {
        if (!lookup(w[1], &a))
            return fail(line, "unknown name");
        say("%s = %ld", w[1].c_str(), a);
    } else if (command == "assert" && n == 3 && number(w[2], &b)) {
        if (!lookup(w[1], &a))
            return fail(line, "unknown name");
        if (a != b) {
            say("assertion failed: %s is %ld, not %ld", w[1].c_str(), a, b);
            assertionsFailed++;
        }
    } else if (command == "dump") {
        dump();
    } else if (command == "quit") {
        say("quitting%s", assertionsFailed ? " (assertions failed)" : "");
        exit(assertionsFailed ? 1 : 0);
    } else if (command == "help") {
        help();
    } else {
        fail(line, "bad command (try help)");
    }
}

} /* namespace */

/* Queues commands (separated by ';' or newlines) to run as the game gets to them. */
extern "C" void zbDebugRun(const char *text)
{
    std::string line;

    for (const char *p = text;; p++) {
        if (*p == ';' || *p == '\n' || *p == 0) {
            size_t hash = line.find('#');
            if (hash != std::string::npos)
                line.resize(hash);
            if (!words(line).empty())
                pending.push_back(line);
            line.clear();
            if (!*p)
                break;
        } else
            line += *p;
    }
}

/* Called at the start of every pass of the game's frame hook. */
void zbDebugFrame()
{
#ifdef __EMSCRIPTEN__
    static char text[65536];

    while (takePageCommands(text, sizeof text))
        zbDebugRun(text);
#endif
    if (pending.empty())
        return;
    sceneChanged = false;
    for (;;) {
        if (busy || !gameActive || dialogFlags || currentScene == -1 || pendingScene != -1)
            return;
        if (waitingForTime) {
            if ((LONG)(GetTickCount() - waitUntil) < 0)
                return;
            waitingForTime = false;
        }
        if (waitingForScene != -2) {
            if (currentScene != waitingForScene)
                return;
            waitingForScene = -2;
        }
        if (pending.empty() || sceneChanged)
            return; /* (a new scene gets a pass of its own before the next command) */
        std::string line = pending.front();
        pending.pop_front();
        run(line);
    }
}
