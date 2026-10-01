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
#include "debug.h"
#include "mainloop.h"
#include "net.h"
#include "roster.h"
#include "snoids.h"
#include "lilly.h"
#include "town.h"
#include "view.h"

#include <algorithm>
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

/* miniwin's scripted click and screenshot (port/miniwin/screen.cpp); with `at` 0 a click happens now. */
namespace miniwin {
void scriptClick(DWORD at, int x, int y, int action);
bool saveScreenshot(const char *name);
}

/* The game's integer globals by name (build/port/generated/debug_globals.inc, from
   zbtools.debug_globals): the address, the size of an element and whether it is signed, and the
   array's dimensions (none for a scalar) with their lengths. */
struct DebugGlobal
{
    const char *name;
    void *address;
    int size;
    int isSigned;
    int dimensions;
    int first;
    int second;
};

#if defined(__has_include) && __has_include("debug_globals.inc")
#include "debug_globals.inc"
#else
static const DebugGlobal debugGlobals[] = {{0, 0, 0, 0, 0, 0, 0}}; /* (not generated) */
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

/* A global (or an element of an array one: name[3], name[1][2]) found in the table. */
struct Located
{
    void *at;
    int size;
    bool isSigned;
};

bool locate(const std::string &text, Located *found)
{
    size_t bracket = text.find('[');
    std::string base = text.substr(0, bracket);
    long index[2] = {0, 0};
    int count = 0;

    while (bracket != std::string::npos) {
        size_t close = text.find(']', bracket);
        if (close == std::string::npos || count == 2
            || !number(text.substr(bracket + 1, close - bracket - 1), &index[count]))
            return false;
        count++;
        bracket = close + 1 == text.size() ? std::string::npos : close + 1;
        if (bracket != std::string::npos && text[bracket] != '[')
            return false;
    }
    for (const DebugGlobal &g : debugGlobals) {
        if (!g.name || base != g.name)
            continue;
        if (count != g.dimensions || (count >= 1 && (index[0] < 0 || index[0] >= g.first))
            || (count == 2 && (index[1] < 0 || index[1] >= g.second)))
            return false;
        long element = count == 2 ? index[0] * g.second + index[1] : index[0];
        found->at = (char *)g.address + element * g.size;
        found->size = g.size;
        found->isSigned = g.isSigned != 0;
        return true;
    }
    return false;
}

long readAt(const Located &where)
{
    switch (where.size) {
    case 1:
        return where.isSigned ? *(signed char *)where.at : *(unsigned char *)where.at;
    case 2:
        return where.isSigned ? *(short *)where.at : *(unsigned short *)where.at;
    default:
        return *(long *)where.at;
    }
}

void writeAt(const Located &where, long value)
{
    switch (where.size) {
    case 1:
        *(char *)where.at = (char)value;
        break;
    case 2:
        *(short *)where.at = (short)value;
        break;
    default:
        *(long *)where.at = value;
        break;
    }
}

bool setGlobal(const std::string &name, long value)
{
    Located where;

    if (!locate(name, &where))
        return false;
    writeAt(where, value);
    return true;
}

/* The party's Zoombini views in the order they were made (the party's order). */
std::vector<View *> zoombiniViews()
{
    std::vector<View *> found;

    for (View *view = nextActorView(1); view; view = nextActorView(0))
        found.push_back(view);
    std::sort(found.begin(), found.end(), [](View *a, View *b) { return a->id < b->id; });
    return found;
}

/* The middle of Zoombini n's picture on the screen (where it stands, if it hasn't been drawn). */
bool zoombiniPoint(long n, long *x, long *y)
{
    std::vector<View *> views = zoombiniViews();

    if (n < 0 || n >= (long)views.size())
        return false;
    const ShortRect &bounds = views[n]->body.bounds;
    if (bounds.right > bounds.left) {
        *x = (bounds.left + bounds.right) / 2;
        *y = (bounds.top + bounds.bottom) / 2;
    } else {
        *x = viewSnoid(views[n])->body.x;
        *y = viewSnoid(views[n])->body.y;
    }
    return true;
}

/* The point a Zoombini stands on (its feet), and the middle of its picture, which a drag grabs. */
bool zoombiniStands(long n, long *standX, long *standY)
{
    std::vector<View *> views = zoombiniViews();

    if (n < 0 || n >= (long)views.size())
        return false;
    *standX = viewSnoid(views[n])->body.x;
    *standY = viewSnoid(views[n])->body.y;
    return true;
}

/* Whether a command may run while a game dialog is open (the options, the saved games, "keep the
   party?"): those that look, wait or send input, which is how a dialog is operated. The others
   (changing the scene or the state) wait for it to close. */
bool worksInDialog(const std::string &line)
{
    std::vector<std::string> w = words(line);

    if (w.empty())
        return true;
    static const char *const verbs[] = {"screenshot", "get", "assert", "dump", "help", "quit", "wait",
                                         "click", "key", "zoombinis", "places", "monuments", "toads"};
    for (const char *verb : verbs)
        if (w[0] == verb)
            return !(w[0] == "click" && w.size() >= 2 && w[1] == "zoombini");
    return false;
}

/* Puts commands at the front of the queue, to run next. */
void runNext(const std::vector<std::string> &commands)
{
    for (size_t i = commands.size(); i > 0; i--)
        pending.push_front(commands[i - 1]);
}

/* A drag as the steps `click` makes: press, three moves with a pause each, release. */
std::vector<std::string> dragSteps(long x1, long y1, long x2, long y2)
{
    std::vector<std::string> steps = {"click " + std::to_string(x1) + " " + std::to_string(y1) + " press",
                                      "wait 300"};

    for (int i = 1; i <= 3; i++) {
        steps.push_back("click " + std::to_string(x1 + (x2 - x1) * i / 3) + " "
                        + std::to_string(y1 + (y2 - y1) * i / 3) + " move");
        steps.push_back("wait 250");
    }
    /* (The last pause matters: the game's drag loop must see the pointer there before the release.) */
    steps.push_back("click " + std::to_string(x2) + " " + std::to_string(y2) + " release");
    return steps;
}

struct Condition
{
    bool active;
    std::string name;
    std::string op;
    long value;
} condition; /* `wait until`: the commands after it wait for this */

bool compares(long a, const std::string &op, long b)
{
    return op == "==" ? a == b : op == "!=" ? a != b : op == "<" ? a < b : op == ">" ? a > b
           : op == "<=" ? a <= b : a >= b;
}

/* Names a value `assert`, `get` and `wait until` can read: a field of the game below, state:<offset>[:<size>], or any
   global in the table (`bridgeLevel`, `acrossSpots`-like `name[3]`). */
bool lookup(const std::string &name, long *value)
{
    if (name == "scene")
        *value = currentScene;
    else if (name == "pending")
        *value = pendingScene;
    else if (name == "practice")
        *value = practiceLevel;
    else if (name == "due")
        *value = sceneDue;
    else if (name == "clock")
        *value = (long)clockTime();
    else if (name == "viewclock")
        *value = (long)viewClock();
    else if (name == "transitions")
        *value = transitionsOn;
    else if (name == "party")
        *value = party()->count;
    else if (name == "debug")
        *value = debugMessagesOn;
    else if (name == "dialog")
        *value = dialogFlags;
    else if (name.compare(0, 5, "level") == 0 && name.size() == 6 && name[5] >= '1' && name[5] <= '4')
        *value = puzzleLevels()[name[5] - '0'] + 1;
#ifdef ZB_PERFECT_CLEARS_PER_LEVEL
    else if (name.compare(0, 6, "clears") == 0 && name.size() == 7 && name[6] >= '1' && name[6] <= '4')
        *value = perfectClears(name[6] - '0');
#endif
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
    } else {
        Located where;

        if (!locate(name, &where))
            return false;
        *value = readAt(where);
    }
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
        makeName(traveller->name, sizeof traveller->name);
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
    resetViewClock(); /* (the journey scene leaves once the clock, idle time, passes 300 ticks) */
    pendingScene = scene;
    enterNextScene();
    /* The scene entered: `scene` itself, or with `map` the journey scene (2) on the way, which
       lasts as long as its narration; `wait scene N` waits for the destination. */
    waitingForScene = currentScene;
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
    say("commands: debug on|off | transitions on|off | scene N [map] | level G L | clears G [N] | practice L (0 off) | party N"
        " | unlock | records N | state get|set OFFSET [VALUE] [SIZE] | cheatcode HASH CODE"
        " | paldiff | key CODE | roster save | roster load | wait MS | wait scene N | get NAME"
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
    } else if (command == "transitions" && n == 2 && (w[1] == "on" || w[1] == "off")) {
        /* The options' "transitions" (Ctrl-T): off shows the journey screen between scenes. */
        transitionsOn = w[1] == "on";
    } else if (command == "scene" && n >= 2 && number(w[1], &a) && a >= 0 && a <= 21) {
        changeScene((short)a, n >= 3 && w[2] == "map");
    } else if (command == "level" && n == 3 && number(w[1], &a) && number(w[2], &b) && a >= 1 && a <= 4
               && b >= 1 && b <= 4) {
        puzzleLevels()[a] = (short)(b - 1);
    } else if (command == "clears" && (n == 2 || n == 3) && number(w[1], &a) && a >= 1 && a <= 4) {
#ifdef ZB_PERFECT_CLEARS_PER_LEVEL
        if (n == 3 && number(w[2], &b) && b >= 0 && b <= 3)
            setPerfectClears((short)a, (short)b);
        else if (n == 3)
            return fail(line, "bad count");
        say("clears[%ld] = %d", a, perfectClears((short)a));
#else
        return fail(line, "built without ZB_PERFECT_CLEARS_PER_LEVEL");
#endif
    } else if (command == "practice" && n == 2 && number(w[1], &a) && a >= 0 && a <= 4) {
        practiceLevel = (short)a;
    } else if (command == "party" && n == 2 && number(w[1], &a)) {
        debugPartyCount = a < 0 ? 0 : a > 16 ? 16 : a;
        makeParty(a);
    } else if (command == "unlock" && n == 1) {
        /* The lowest bit of each group's nibble, as the game's own debug key '@' sets (the
           map takes a nibble as a level number: all four bits set would index past the
           four levels' views). */
        gameState[0x50] |= 1;
        gameState[0x51] |= 1;
        *(short *)(gameState + 0x52) |= 0x11;
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
    } else if (command == "set" && n == 3 && number(w[2], &a)) {
        if (!setGlobal(w[1], a))
            return fail(line, "unknown global");
    } else if (command == "seed" && n == 2 && number(w[1], &a)) {
        /* The game's random numbers, from a seed (it seeds them from the time otherwise). */
        if (!setGlobal("randomSeed", a) || !setGlobal("seedPending", 0))
            return fail(line, "no random seed in the table");
    } else if (command == "wait" && n == 5 && w[1] == "until" && number(w[4], &a)
               && (w[3] == "==" || w[3] == "!=" || w[3] == "<" || w[3] == ">" || w[3] == "<="
                   || w[3] == ">=")) {
        condition = {true, w[2], w[3], a};
    } else if (command == "screenshot" && n == 2) {
        if (!miniwin::saveScreenshot(w[1].c_str()))
            return fail(line, "no --screenshot path to save beside");
    } else if (command == "zoombinis" && n == 1) {
        std::vector<View *> views = zoombiniViews();

        for (size_t i = 0; i < views.size(); i++) {
            long x = 0, y = 0;
            Snoid *snoid = viewSnoid(views[i]);

            zoombiniPoint((long)i, &x, &y);
            say("zoombini %d: at %ld,%ld  hair %d eyes %d nose %d feet %d  %s  chosen %d action %d", (int)i, x,
                y, snoid->features[0], snoid->features[1], snoid->features[2], snoid->features[3],
                snoid->name, snoid->chosen, snoid->action);
        }
    } else if (command == "click" && n == 3 && w[1] == "zoombini" && number(w[2], &a)) {
        if (!zoombiniPoint(a, &b, &c))
            return fail(line, "no such Zoombini");
        runNext({"click " + std::to_string(b) + " " + std::to_string(c)});
    } else if (command == "drag" && n == 5 && w[1] == "zoombini" && number(w[2], &a) && number(w[3], &b)
               && number(w[4], &c)) {
        /* Brings the Zoombini's feet to (b, c): the grab is at its middle, and the game puts the
           feet where the pointer is, less that offset. */
        long x, y, standX, standY;

        if (!zoombiniPoint(a, &x, &y) || !zoombiniStands(a, &standX, &standY))
            return fail(line, "no such Zoombini");
        runNext(dragSteps(x, y, b + x - standX, c + y - standY));
    } else if (command == "drag" && n == 5 && w[1] == "zoombini" && number(w[2], &a) && w[3] == "place"
               && number(w[4], &b)) {
        long x, y, standX, standY;

        if (b < 1 || b > placedViewCount)
            return fail(line, "no such place (see `places`)");
        if (!zoombiniPoint(a, &x, &y) || !zoombiniStands(a, &standX, &standY))
            return fail(line, "no such Zoombini");
        runNext(dragSteps(x, y, placedViewPoints[b - 1].x + x - standX, placedViewPoints[b - 1].y + y - standY));
    } else if (command == "toads" && n == 1) {
        /* Toads' pieces (kind 0) and the rows they can go to, for scripting scene 11: a piece goes
           into a row whose left cell has the same attribute value (the piece's attribute 1-3
           picks which of the cell's attributes); the layout is lilly.cpp's LillyActor. */
        for (View *view = viewListEnd(1); view; view = view->next) {
            const unsigned char *body = (const unsigned char *)&view->body;

            if ((view->flags & 0x980002) != 0x980002)
                continue;
            say("piece: view %d at %d,%d kind %d attribute %d value %d onboard %d", view->id,
                (view->body.bounds.left + view->body.bounds.right) / 2,
                (view->body.bounds.top + view->body.bounds.bottom) / 2, *(short *)(body + 0xc0), body[0xde],
                body[0xdf], body[0xc2]);
        }
        for (int row = 0; row < 12; row++)
            say("row %d: entry %d,%d taken %d attributes %d %d %d", row, (rowEntryRects[row].left + rowEntryRects[row].right) / 2,
                (rowEntryRects[row].top + rowEntryRects[row].bottom) / 2, lillyBoard[row][0].attributes[0],
                lillyBoard[row][0].attributes[1], lillyBoard[row][0].attributes[2],
                lillyBoard[row][0].attributes[3]);
    } else if (command == "monuments" && n == 1) {
        /* Zoombiniville's record hotspots on the screen shown (scene 6): a click needs the pointer
           moved onto one first (`click X Y move`, a short `wait`), as the town notices it in its frame. */
        for (short i = 0; i < recordHotspotCount; i++)
            say("hotspot %d: %d,%d to %d,%d (record %d)", i, recordHotspots[i].left, recordHotspots[i].top,
                recordHotspots[i].right, recordHotspots[i].bottom, recordHotspotNumbers[i] + 1);
    } else if (command == "places" && n == 1) {
        for (short i = 0; i < placedViewCount; i++)
            say("place %d: %d,%d%s", i + 1, placedViewPoints[i].x, placedViewPoints[i].y,
                placeClaims[i] ? " (claimed)" : "");
        for (short i = 0; i < viewPlaceCount; i++)
            say("spot %d: %d,%d", i + 1, viewPlaces[i].x, viewPlaces[i].y);
    } else if (command == "drag" && n == 5) {
        long x2, y2;

        if (!number(w[1], &a) || !number(w[2], &b) || !number(w[3], &x2) || !number(w[4], &y2))
            return fail(line, "bad drag");
        runNext(dragSteps(a, b, x2, y2));
    } else if (command == "click" && n >= 3 && n <= 4 && number(w[1], &a) && number(w[2], &b)
               && (n == 3 || w[3] == "press" || w[3] == "move" || w[3] == "release")) {
        /* 0: a click, 1: press, 2: move, 3: release (as --click's actions) */
        int action = n == 3 ? 0 : w[3] == "press" ? 1 : w[3] == "move" ? 2 : 3;
        miniwin::scriptClick(0, (int)a, (int)b, action);
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
    } else if (command == "paldiff") {
        /* The entries of loadedPalette (what the scene's images were made for) that differ
           from targetPalette (what fadeInViews shows), as ranges. */
        std::string ranges;
        long start = -1, total = 0;
        for (long i = 0; i <= 256; i++) {
            bool differs = i < 256
                           && (loadedPalette[i].peRed != targetPalette[i].peRed
                               || loadedPalette[i].peGreen != targetPalette[i].peGreen
                               || loadedPalette[i].peBlue != targetPalette[i].peBlue);
            if (differs) {
                total++;
                if (start < 0)
                    start = i;
            } else if (start >= 0) {
                ranges += " " + std::to_string(start) + "-" + std::to_string(i - 1);
                start = -1;
            }
        }
        say("palette: %ld entries of loadedPalette differ from targetPalette:%s", total,
            ranges.empty() ? " none" : ranges.c_str());
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
        if (busy || !gameActive || currentScene == -1 || pendingScene != -1)
            return;
        if (dialogFlags && !pending.empty() && !worksInDialog(pending.front()))
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
        if (condition.active) {
            long now;

            if (!lookup(condition.name, &now)) {
                say("error: wait until: unknown name %s", condition.name.c_str());
                assertionsFailed++;
                condition.active = false;
            } else if (!compares(now, condition.op, condition.value))
                return;
            else
                condition.active = false;
        }
        if (pending.empty() || sceneChanged)
            return; /* (a new scene gets a pass of its own before the next command) */
        std::string line = pending.front();
        pending.pop_front();
        run(line);
    }
}
