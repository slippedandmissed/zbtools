/* Oracles for the instrumented gameplay tests: see oracles.h. */

#include "zoombinis.h"
#include "bridge.h"
#include "tunnels.h"
#include "pizza.h"
#include "snoids.h"
#include "view.h"
#include "oracles.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace {

/* ---- Allergic Cliffs (scene 7) ---------------------------------------------------------------------
 *
 * Dropping a Zoombini on a bridge (place 1 the upper, 2 the lower) queues it to cross; the cliff's rule
 * (bridgeRules) decides, by turnedBack, whether it is turned back (it sneezes and walks back, and
 * counts in sentBackCount, six at most) or crosses (and counts across). The right bridge for a
 * Zoombini is the one where it isn't turned back.
 */

bool cliffsOpen()
{
    return currentScene == 7;
}

bool inList(const short *views, short count, short id)
{
    for (short i = 0; i < count; i++)
        if (views[i] == id)
            return true;
    return false;
}

/* Whether the Zoombini is standing among those waiting to be sent: not across, not queued, not walking. */
bool cliffsWaiting(View *view)
{
    Snoid *snoid = viewSnoid(view);

    return snoid->chosen == 0 && snoid->action != 8 && snoid->action != 9
           && !inList(queueViews, queuedCount, view->id) && !inList(upperViews, upperCount, view->id)
           && !inList(lowerViews, lowerCount, view->id);
}

/* The bridge (1 or 2) where the Zoombini is let across. */
short cliffsRightBridge(View *view)
{
    return turnedBack(&bridgeRules, 1, viewSnoid(view)) ? 2 : 1;
}

bool cliffsCommand(const std::vector<std::string> &w, OracleResult *result)
{
    if (!cliffsOpen()) {
        result->error = "cliffs: Allergic Cliffs (scene 7) isn't open";
        return true;
    }
    std::vector<View *> views = zbDebugZoombiniViews();

    if (w.size() == 1) {
        /* `cliffs`: what the rule makes of each Zoombini */
        for (size_t i = 0; i < views.size(); i++) {
            Snoid *snoid = viewSnoid(views[i]);
            char line[160];

            snprintf(line, sizeof line, "zoombini %d: hair %d eyes %d nose %d feet %d  right bridge %d  %s",
                     (int)i, snoid->features[0], snoid->features[1], snoid->features[2], snoid->features[3],
                     cliffsRightBridge(views[i]), cliffsWaiting(views[i]) ? "waiting" : "not waiting");
            result->said.push_back(line);
        }
        return true;
    }
    if (w.size() >= 3 && w[1] == "pick" && (w[2] == "right" || w[2] == "wrong")) {
        /* `cliffs pick right|wrong`: drags the first waiting Zoombini to its right (or the other) bridge */
        if (sentBackCount >= 6) {
            result->error = "cliffs pick: six have been sent back, the cliff takes no more";
            return true;
        }
        for (size_t i = 0; i < views.size(); i++)
            if (cliffsWaiting(views[i])) {
                short right = cliffsRightBridge(views[i]);
                short place = w[2] == "right" ? right : 3 - right;

                result->commands.push_back("drag zoombini " + std::to_string(i) + " place " + std::to_string(place));
                result->commands.push_back("wait 800"); /* (the game takes the drop a moment after the release) */
                result->said.push_back("cliffs: zoombini " + std::to_string(i) + " to bridge " + std::to_string(place)
                                       + " (" + w[2] + ")");
                return true;
            }
        result->error = "cliffs pick: no Zoombini is waiting";
        return true;
    }
    if (w.size() >= 3 && w[1] == "send" && (w[2] == "right" || w[2] == "wrong")) {
        /* `cliffs send right|wrong [N]`: N times (once), when the queue and the crossing are clear,
           the first waiting Zoombini to its right (or wrong) bridge */
        int count = w.size() >= 4 ? atoi(w[3].c_str()) : 1;

        for (int i = 0; i < count; i++) {
            result->commands.push_back("wait until queuedCount == 0");
            result->commands.push_back("wait until crossingUnderway == 0");
            result->commands.push_back("cliffs pick " + w[2]);
        }
        return true;
    }
    result->error = "cliffs: expected `cliffs`, `cliffs pick right|wrong` or `cliffs send right|wrong [N]`";
    return true;
}


/* ---- Stone Cold Caves (scene 8) -------------------------------------------------------------------
 *
 * A Zoombini dropped at one of four doors (places 1-4) is let in or turned back by the doors' rule
 * (turnedBackAtDoor over tunnelRules; at level 0 also one pair of doors is shut, closedDoorPair). One
 * turned back walks back and uses up one of the turn-backs allowed (turnBacksLeft: 16-22); at none,
 * drops do nothing. The doors that let a Zoombini in are the right ones.
 */

bool tunnelsOpen_()
{
    return currentScene == 8;
}

/* Whether the doors turn the Zoombini back at `door` (1-4), as tunnelsClicked works it out. */
bool doorTurnsBack(Snoid *snoid, short door)
{
    unsigned short first;
    short back = turnedBackAtDoor(&tunnelRules, door, snoid, &first);

    if (!back && !tunnelsLevel) {
        if (closedDoorPair) {
            if (door == 1 || door == 4)
                back = 1;
        } else if (door == 2 || door == 3) {
            back = 1;
        }
    }
    return back != 0;
}

bool inQueue(short id)
{
    for (short i = 0; i < tunnelQueue.count; i++)
        if (tunnelQueue.entries[i].view == id)
            return true;
    return false;
}

/* Standing among those waiting to be sent: not in a door, not queued, not walking (tunnelsClicked
   takes a drag only when its action is 0 or 6). */
bool tunnelsWaiting(View *view)
{
    Snoid *snoid = viewSnoid(view);

    return snoid->chosen == 0 && (snoid->action == 0 || snoid->action == 6) && !inQueue(view->id);
}

/* The first door (1-4) that turns the Zoombini back (`back`) or lets it in (!back); 0 if none. */
short tunnelsDoor(View *view, bool back)
{
    for (short door = 1; door <= 4; door++)
        if (doorTurnsBack(viewSnoid(view), door) == back)
            return door;
    return 0;
}

bool tunnelsCommand(const std::vector<std::string> &w, OracleResult *result)
{
    if (!tunnelsOpen_()) {
        result->error = "tunnels: Stone Cold Caves (scene 8) isn't open";
        return true;
    }
    std::vector<View *> views = zbDebugZoombiniViews();

    if (w.size() == 1) {
        /* `tunnels`: which doors let each Zoombini in */
        for (size_t i = 0; i < views.size(); i++) {
            Snoid *snoid = viewSnoid(views[i]);
            std::string doors;

            for (short door = 1; door <= 4; door++)
                if (!doorTurnsBack(snoid, door))
                    doors += " " + std::to_string(door);
            char line[200];

            snprintf(line, sizeof line, "zoombini %d: hair %d eyes %d nose %d feet %d  let in at door(s)%s  %s",
                     (int)i, snoid->features[0], snoid->features[1], snoid->features[2], snoid->features[3],
                     doors.empty() ? " none" : doors.c_str(), tunnelsWaiting(views[i]) ? "waiting" : "not waiting");
            result->said.push_back(line);
        }
        return true;
    }
    if (w.size() >= 3 && w[1] == "pick" && (w[2] == "right" || w[2] == "wrong")) {
        /* `tunnels pick right|wrong`: drags the first waiting Zoombini to a door that lets it in (or turns it back) */
        bool back = w[2] == "wrong";

        if (back && turnBacksLeft == 0) {
            result->error = "tunnels pick: no turn-backs are left, the doors take no more drops";
            return true;
        }
        for (size_t i = 0; i < views.size(); i++)
            if (tunnelsWaiting(views[i])) {
                short door = tunnelsDoor(views[i], back);

                if (!door) {
                    result->error = "tunnels pick: zoombini " + std::to_string(i) + " has no "
                                    + (back ? "door that turns it back" : "door that lets it in");
                    return true;
                }
                result->commands.push_back("drag zoombini " + std::to_string(i) + " place " + std::to_string(door));
                result->commands.push_back("wait 800"); /* (the game takes the drop a moment after the release) */
                result->said.push_back("tunnels: zoombini " + std::to_string(i) + " to door " + std::to_string(door)
                                       + " (" + w[2] + ")");
                return true;
            }
        result->error = "tunnels pick: no Zoombini is waiting";
        return true;
    }
    if (w.size() >= 3 && w[1] == "send" && (w[2] == "right" || w[2] == "wrong")) {
        /* `tunnels send right|wrong [N]`: N times, when the last one is through (or back) and the guards are done */
        int count = w.size() >= 4 ? atoi(w[3].c_str()) : 1;

        for (int i = 0; i < count; i++) {
            result->commands.push_back("wait until tunnelsQueued == 0");
            result->commands.push_back("wait until entryUnderway == 0");
            result->commands.push_back("tunnels pick " + w[2]);
        }
        return true;
    }
    result->error = "tunnels: expected `tunnels`, `tunnels pick right|wrong` or `tunnels send right|wrong [N]`";
    return true;
}


/* ---- Pizza Pass (scene 9) --------------------------------------------------------------------------
 *
 * The trolls (Arno, Willa and Shyler: one, two or three by level) each want a set of toppings
 * (arnoWants, willaWants, shylerWants). A pizza is made by toggling topping buttons 4-11 (topping
 * n-4) and served by clicking the pizza (button 3) once the next Zoombini has come up with it; the
 * trolls judge it in turn (judgePizza): the one whose wants it is exactly is satisfied, one with a
 * topping it doesn't want rejects it, one that wants more says so. Pizzas are limited (pizzasLeft).
 * The puzzle is solved when every troll at the level is satisfied.
 */

bool pizzaOpen_()
{
    return currentScene == 9;
}

short *trollWants(int troll)
{
    return troll == 0 ? arnoWants : troll == 1 ? willaWants : shylerWants;
}

short trollState(int troll)
{
    return troll == 0 ? arnoState : troll == 1 ? willaState : shylerState;
}

/* The first troll that hasn't been satisfied yet (-1: all are). */
int firstUnsatisfiedTroll()
{
    for (int troll = 0; troll < 3; troll++)
        if (trollState(troll) == 1)
            return troll;
    return -1;
}

/* Whether the next pizza can be made and served: the serve conditions of pizzaButtonClicked (or the
   puzzle is over, so that waiting for it ends). */
bool pizzaReady()
{
    return pizzaSolved
           || (!zoombiniComing && !levelTrollStarted && !partyThrough && !arnoGroup && !willaGroup && !shylerGroup
               && !pileGroup && !pizzaView7000Group && placeClaims[0]);
}

/* The centre of a button of the scene (1-13). */
std::string pizzaButtonPoint(int button)
{
    const ShortRect &rect = pizzaButtons[button - 1].rect;

    return std::to_string((rect.left + rect.right) / 2) + " " + std::to_string((rect.top + rect.bottom) / 2);
}

/* The toppings of the pizza that suits the first unsatisfied troll `how`: "right" (exactly what it
   wants), "wrong" (and one it doesn't want: it rejects the pizza) or "partial" (one short of what it
   wants: it asks for more). Fails if there is no such pizza. */
bool pizzaFor(const std::string &how, std::vector<int> *toppings, std::string *error)
{
    int troll = firstUnsatisfiedTroll();

    if (troll < 0) {
        *error = "every troll is satisfied";
        return false;
    }
    short *wants = trollWants(troll);

    for (int i = 0; i < toppingCount; i++)
        if (wants[i])
            toppings->push_back(i);
    if (how == "wrong") {
        for (int i = 0; i < toppingCount; i++)
            if (!wants[i] && !(pizzaLevel == 1 && i == 4)) { /* (level 1 has no topping 4) */
                toppings->push_back(i);
                return true;
            }
        *error = "the troll wants every topping";
        return false;
    }
    if (how == "partial") {
        if (toppings->size() < 2) {
            *error = "the troll wants only one topping";
            return false;
        }
        toppings->pop_back();
    }
    return true;
}

bool pizzaCommand(const std::vector<std::string> &w, OracleResult *result)
{
    if (!pizzaOpen_()) {
        result->error = "pizza: Pizza Pass (scene 9) isn't open";
        return true;
    }
    if (w.size() == 1) {
        /* `pizza`: what the trolls want */
        const char *names[3] = {"Arno", "Willa", "Shyler"};

        for (int troll = 0; troll < 3; troll++) {
            if (!trollState(troll))
                continue;
            std::string line = std::string(names[troll]) + " (state " + std::to_string(trollState(troll)) + ") wants:";

            for (int i = 0; i < toppingCount; i++)
                if (trollWants(troll)[i])
                    line += " " + std::to_string(i);
            result->said.push_back(line);
        }
        result->said.push_back("pizzas left " + std::to_string(pizzasLeft) + ", " + (pizzaReady() ? "ready" : "not ready"));
        return true;
    }
    if (w.size() >= 3 && w[1] == "make" && (w[2] == "right" || w[2] == "wrong" || w[2] == "partial")) {
        /* `pizza make right|wrong|partial`: toggles the toppings and serves the pizza, now */
        std::vector<int> toppings;
        std::string error;

        if (pizzaSolved) {
            result->said.push_back("pizza: already solved, nothing to serve");
            return true;
        }
        if (!pizzaFor(w[2], &toppings, &error)) {
            result->error = "pizza make: " + error;
            return true;
        }
        std::string made;

        for (int topping : toppings) {
            result->commands.push_back("click " + pizzaButtonPoint(topping + 4));
            result->commands.push_back("wait 300");
            made += " " + std::to_string(topping);
        }
        result->commands.push_back("click " + pizzaButtonPoint(3));
        result->commands.push_back("wait 800");
        result->said.push_back("pizza: served a " + w[2] + " pizza with toppings" + made);
        return true;
    }
    if (w.size() >= 3 && w[1] == "serve" && (w[2] == "right" || w[2] == "wrong" || w[2] == "partial")) {
        /* `pizza serve right|wrong|partial [N]`: N times (once), when the pizza can be served, make one */
        int count = w.size() >= 4 ? atoi(w[3].c_str()) : 1;

        for (int i = 0; i < count; i++) {
            result->commands.push_back("wait until pizzaReady == 1");
            result->commands.push_back("pizza make " + w[2]);
        }
        return true;
    }
    result->error = "pizza: expected `pizza`, `pizza make right|wrong|partial` or `pizza serve right|wrong|partial [N]`";
    return true;
}

} /* namespace */

bool zbOracleCommand(const std::vector<std::string> &w, OracleResult *result)
{
    if (!w.empty() && w[0] == "cliffs")
        return cliffsCommand(w, result);
    if (!w.empty() && w[0] == "tunnels")
        return tunnelsCommand(w, result);
    if (!w.empty() && w[0] == "pizza")
        return pizzaCommand(w, result);
    return false;
}

bool zbOracleValue(const std::string &name, long *value)
{
    if (name == "cliffsAcross") {
        *value = upperCount + lowerCount; /* the Zoombinis that have crossed */
        return true;
    }
    if (name == "cliffsWaiting") {
        long waiting = 0;

        for (View *view : zbDebugZoombiniViews())
            waiting += cliffsWaiting(view) ? 1 : 0;
        *value = waiting;
        return true;
    }
    if (name == "tunnelsQueued") {
        *value = tunnelQueue.count; /* the entries the guards have yet to deal with */
        return true;
    }
    if (name == "tunnelsIn") {
        long in = 0;

        for (View *view : zbDebugZoombiniViews())
            in += viewSnoid(view)->chosen != 0 ? 1 : 0; /* the Zoombinis let in */
        *value = in;
        return true;
    }
    if (name == "tunnelsWaiting") {
        long waiting = 0;

        for (View *view : zbDebugZoombiniViews())
            waiting += tunnelsWaiting(view) ? 1 : 0;
        *value = waiting;
        return true;
    }
    if (name == "pizzaReady") {
        *value = pizzaReady() ? 1 : 0;
        return true;
    }
    return false;
}
