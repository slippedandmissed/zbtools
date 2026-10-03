/* Oracles for the instrumented gameplay tests: see oracles.h. */

#include "zoombinis.h"
#include "bridge.h"
#include "tunnels.h"
#include "pizza.h"
#include "ferry.h"
#include "lilly.h"
#include "slides.h"
#include "snoids.h"
#include "view.h"
#include "oracles.h"

#include <cstdio>
#include <utility>
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


/* ---- Captain Cajun's Ferryboat (scene 10) ---------------------------------------------------------
 *
 * The Zoombinis are seated on the ferry's places (placed views 1..N; ferryLinks lists, for each, the
 * places it touches). A Zoombini dropped on a place stays only if it shares a feature with each
 * occupied place the place touches (else it is sent back), so a seating where every touching pair
 * shares a feature can be made in any order. The oracle finds one by search and seats the Zoombinis
 * in place order, as the game checks them.
 */

bool ferryOpen_()
{
    return currentScene == 10;
}

/* The places (from 1) that place `place` touches. */
std::vector<int> ferryTouching(int place)
{
    std::vector<int> found;

    for (int k = 0; k < 8; k++)
        if (ferryLinks[place - 1][k])
            found.push_back(ferryLinks[place - 1][k]);
    return found;
}

bool shareFeature(Snoid *a, Snoid *b)
{
    for (int f = 0; f < 4; f++)
        if (a->features[f] == b->features[f])
            return true;
    return false;
}

struct FerryPlan
{
    std::vector<View *> views;
    std::vector<int> seatOf; /* per Zoombini: its place (from 1), 0 if it stays behind */
    std::vector<int> occupant; /* per place (from 1): the Zoombini's index, or -1 (empty or not decided) */
    std::vector<bool> decided; /* per place (from 1): has a Zoombini or is left empty */
    int places;
    long nodes;
};

/* Whether Zoombini `z` may take `place` given the places already decided (the game checks a drop
   against the occupied places the place touches; the lists are symmetric, so the order is free). */
bool ferryFits(const FerryPlan &plan, size_t z, int place)
{
    for (int other : ferryTouching(place))
        if (plan.decided[other] && plan.occupant[other] >= 0
            && !shareFeature(viewSnoid(plan.views[z]), viewSnoid(plan.views[plan.occupant[other]])))
            return false;
    return true;
}

/* Seats the Zoombinis by search: the undecided place with the fewest Zoombinis that fit goes next
   (a place nobody fits ends that branch), a place may be left empty while places outnumber the
   Zoombinis. Gives up after a budget of nodes. */
bool ferrySeat(FerryPlan &plan, int empties)
{
    if (++plan.nodes > 3000000)
        return false;
    int best = 0;
    size_t bestCount = 1000;

    for (int place = 1; place <= plan.places; place++) {
        if (plan.decided[place])
            continue;
        size_t count = empties > 0 ? 1 : 0;

        for (size_t z = 0; z < plan.views.size(); z++)
            if (!plan.seatOf[z] && ferryFits(plan, z, place))
                count++;
        if (count < bestCount) {
            best = place;
            bestCount = count;
            if (count == 0)
                return false;
        }
    }
    if (!best)
        return true; /* (every place decided; every Zoombini seated, as the empties were counted) */
    plan.decided[best] = true;
    for (size_t z = 0; z < plan.views.size(); z++) {
        if (plan.seatOf[z] || !ferryFits(plan, z, best))
            continue;
        plan.seatOf[z] = best;
        plan.occupant[best] = (int)z;
        if (ferrySeat(plan, empties))
            return true;
        plan.seatOf[z] = 0;
        plan.occupant[best] = -1;
    }
    if (empties > 0 && ferrySeat(plan, empties - 1))
        return true;
    plan.decided[best] = false;
    return false;
}

/* A seating of every Zoombini on the ferry, if the search finds one. */
bool ferryPlanFor(FerryPlan *plan)
{
    plan->views = zbDebugZoombiniViews();
    plan->seatOf.assign(plan->views.size(), 0);
    plan->places = placedViewCount;
    plan->occupant.assign(plan->places + 1, -1);
    plan->decided.assign(plan->places + 1, false);
    plan->nodes = 0;
    int empties = plan->places - (int)plan->views.size();

    return empties >= 0 && ferrySeat(*plan, empties);
}

/* Whether the Zoombini is standing among those not yet seated. */
bool ferryWaiting(View *view)
{
    return viewSnoid(view)->chosen == 0 && viewSnoid(view)->action != 4;
}

bool ferryBusy()
{
    return returnUnderway || returnDue || ferryLeaving || snoidsOnTheirWay > 0;
}

bool ferryCommand(const std::vector<std::string> &w, OracleResult *result)
{
    if (!ferryOpen_()) {
        result->error = "ferry: Captain Cajun's Ferryboat (scene 10) isn't open";
        return true;
    }
    std::vector<View *> views = zbDebugZoombiniViews();

    if (w.size() == 1) {
        /* `ferry`: a seating that works, if there is one */
        FerryPlan plan;

        if (!ferryPlanFor(&plan)) {
            result->said.push_back("ferry: no seating of every Zoombini works");
            return true;
        }
        for (size_t z = 0; z < views.size(); z++)
            result->said.push_back("zoombini " + std::to_string(z) + " -> place " + std::to_string(plan.seatOf[z]));
        result->said.push_back("(found in " + std::to_string(plan.nodes) + " nodes)");
        return true;
    }
    if (w.size() == 2 && w[1] == "links") {
        /* `ferry links`: which places each place touches */
        for (int place = 1; place <= placedViewCount; place++) {
            std::string line = "place " + std::to_string(place) + " touches";

            for (int other : ferryTouching(place))
                line += " " + std::to_string(other);
            result->said.push_back(line);
        }
        return true;
    }
    if (w.size() >= 3 && w[1] == "seat" && w[2] == "right") {
        /* `ferry seat right`: the next Zoombini of the seating to its place */
        FerryPlan plan;

        if (!ferryPlanFor(&plan)) {
            result->error = "ferry seat: no seating of every Zoombini works";
            return true;
        }
        for (int place = 1; place <= plan.places; place++) {
            int z = plan.occupant[place];

            if (z >= 0 && placeClaims[place - 1] != plan.views[z]->id && ferryWaiting(plan.views[z])) {
                result->commands.push_back("drag zoombini " + std::to_string(z) + " place " + std::to_string(place));
                result->commands.push_back("wait 800"); /* (the game takes the drop a moment after the release) */
                result->said.push_back("ferry: zoombini " + std::to_string(z) + " to place " + std::to_string(place));
                return true;
            }
        }
        result->error = "ferry seat: everyone is seated";
        return true;
    }
    if (w.size() >= 3 && w[1] == "seat" && w[2] == "wrong") {
        /* `ferry seat wrong`: a waiting Zoombini to a free place it doesn't fit (it is sent back) */
        for (size_t z = 0; z < views.size(); z++) {
            if (!ferryWaiting(views[z]))
                continue;
            for (int place = 1; place <= placedViewCount; place++) {
                if (placeClaims[place - 1])
                    continue;
                bool occupiedNeighbour = false, shares = true;

                for (int other : ferryTouching(place)) {
                    View *there = findView(placeClaims[other - 1]);

                    if (there) {
                        occupiedNeighbour = true;
                        shares = shares && shareFeature(viewSnoid(views[z]), viewSnoid(there));
                    }
                }
                if (occupiedNeighbour && !shares) {
                    result->commands.push_back("drag zoombini " + std::to_string(z) + " place " + std::to_string(place));
                    result->commands.push_back("wait 800");
                    result->said.push_back("ferry: zoombini " + std::to_string(z) + " to place " + std::to_string(place)
                                           + " (wrong)");
                    return true;
                }
            }
        }
        result->error = "ferry seat wrong: no waiting Zoombini misfits a free place";
        return true;
    }
    if (w.size() >= 3 && w[1] == "send" && (w[2] == "right" || w[2] == "wrong")) {
        /* `ferry send right [N]` / `ferry send wrong`: N times, when nothing is moving, seat the next one */
        int count = w.size() >= 4 ? atoi(w[3].c_str()) : 1;

        for (int i = 0; i < count; i++) {
            result->commands.push_back("wait until ferryIdle == 1");
            result->commands.push_back("ferry seat " + w[2]);
        }
        return true;
    }
    result->error = "ferry: expected `ferry`, `ferry seat right|wrong` or `ferry send right|wrong [N]`";
    return true;
}


/* ---- Titanic Tattooed Toads (scene 11) ------------------------------------------------------------
 *
 * A toad (a piece, kind 0) with an attribute (1-3) and a value is dropped on a row of the lily-pad
 * board whose first square has that value for that attribute; it then hops over squares with that value,
 * square by square (up, right, down, left), to the far edge (the 12th column), carrying a Zoombini
 * across. One that can't get across fails to. The right rows for the toads are a matching of toads
 * to rows by such paths, as many as there are Zoombinis.
 */

bool toadsOpen_()
{
    return currentScene == 11;
}

struct ToadPiece
{
    View *view;
    int x, y;
    int attribute, value;
};

/* The toads still to be placed (kind 0, not on the board), as `toads` lists them. */
std::vector<ToadPiece> toadPieces()
{
    std::vector<ToadPiece> found;

    for (View *view = viewListEnd(1); view; view = view->next) {
        const unsigned char *body = (const unsigned char *)&view->body;

        if ((view->flags & 0x980002) != 0x980002 || *(const short *)(body + 0xc0) != 0 || body[0xc2])
            continue;
        ToadPiece piece;

        piece.view = view;
        piece.x = (view->body.bounds.left + view->body.bounds.right) / 2;
        piece.y = (view->body.bounds.top + view->body.bounds.bottom) / 2;
        piece.attribute = body[0xde];
        piece.value = body[0xdf];
        found.push_back(piece);
    }
    return found;
}

/* Whether a toad with the attribute and value, put on `row`, can hop to the far column: its first
   square has the value and squares with it join up to column 11. */
bool toadCanCross(int attribute, int value, int row)
{
    bool reached[12][12] = {};
    std::vector<std::pair<int, int>> open;

    if (attribute < 1 || attribute > 3 || lillyBoard[row][0].attributes[attribute] != value)
        return false;
    reached[row][0] = true;
    open.push_back({row, 0});
    for (size_t i = 0; i < open.size(); i++) {
        int r = open[i].first, c = open[i].second;
        const int step[4][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};

        if (c == 11)
            return true;
        for (const auto &d : step) {
            int nr = r + d[0], nc = c + d[1];

            if (nr >= 0 && nr < 12 && nc >= 0 && nc < 12 && !reached[nr][nc]
                && lillyBoard[nr][nc].attributes[attribute] == value) {
                reached[nr][nc] = true;
                open.push_back({nr, nc});
            }
        }
    }
    return false;
}

bool rowFree(int row)
{
    return !lillyBoard[row][0].attributes[0];
}

/* Augmenting-path matching of toads to rows: `rowOf[toad]` for the matched ones. */
bool toadAugment(size_t toad, const std::vector<std::vector<int>> &rows, std::vector<int> &toadAtRow, std::vector<bool> &seen)
{
    for (int row : rows[toad]) {
        if (seen[row])
            continue;
        seen[row] = true;
        if (toadAtRow[row] < 0 || toadAugment(toadAtRow[row], rows, toadAtRow, seen)) {
            toadAtRow[row] = (int)toad;
            return true;
        }
    }
    return false;
}

/* The most toads that can be put on distinct free rows from which they cross (`how` "right"), or
   that fit the first square but can't cross ("dead"); as (toad index, row) pairs. */
std::vector<std::pair<size_t, int>> toadMatching(const std::vector<ToadPiece> &pieces, bool crossing)
{
    std::vector<std::vector<int>> rows(pieces.size());
    std::vector<int> toadAtRow(12, -1);
    std::vector<std::pair<size_t, int>> pairs;

    for (size_t t = 0; t < pieces.size(); t++)
        for (int row = 0; row < 12; row++)
            if (rowFree(row) && lillyBoard[row][0].attributes[pieces[t].attribute] == pieces[t].value
                && toadCanCross(pieces[t].attribute, pieces[t].value, row) == crossing)
                rows[t].push_back(row);
    for (size_t t = 0; t < pieces.size(); t++) {
        std::vector<bool> seen(12, false);

        toadAugment(t, rows, toadAtRow, seen);
    }
    for (int row = 0; row < 12; row++)
        if (toadAtRow[row] >= 0)
            pairs.push_back({(size_t)toadAtRow[row], row});
    return pairs;
}

std::string toadDrag(const ToadPiece &piece, int row)
{
    const ShortRect &entry = rowEntryRects[row];

    return "drag " + std::to_string(piece.x) + " " + std::to_string(piece.y) + " "
           + std::to_string((entry.left + entry.right) / 2) + " " + std::to_string((entry.top + entry.bottom) / 2);
}

bool toadsCommand(const std::vector<std::string> &w, OracleResult *result)
{
    if (!toadsOpen_()) {
        result->error = "toads: Titanic Tattooed Toads (scene 11) isn't open";
        return true;
    }
    std::vector<ToadPiece> pieces = toadPieces();

    if (w.size() == 2 && w[1] == "match") {
        /* `toads match`: the toads that can cross, and the rows they go to */
        for (const auto &pair : toadMatching(pieces, true))
            result->said.push_back("toad at " + std::to_string(pieces[pair.first].x) + "," + std::to_string(pieces[pair.first].y)
                                   + " -> row " + std::to_string(pair.second));
        return true;
    }
    if (w.size() >= 3 && w[1] == "place" && (w[2] == "right" || w[2] == "dead" || w[2] == "wrong")) {
        /* `toads place right|dead|wrong`: the next toad to a row it crosses from (`right`), one whose first
           square fits but can't cross (`dead`), or one it doesn't fit at all (`wrong`: it goes back) */
        if (w[2] == "wrong") {
            for (const ToadPiece &piece : pieces)
                for (int row = 0; row < 12; row++)
                    if (rowFree(row) && lillyBoard[row][0].attributes[piece.attribute] != piece.value) {
                        result->commands.push_back(toadDrag(piece, row));
                        result->commands.push_back("wait 1200");
                        result->said.push_back("toads: toad at " + std::to_string(piece.x) + "," + std::to_string(piece.y)
                                               + " to row " + std::to_string(row) + " (wrong)");
                        return true;
                    }
            result->error = "toads place wrong: no toad misfits a free row";
            return true;
        }
        std::vector<std::pair<size_t, int>> pairs = toadMatching(pieces, w[2] == "right");

        if (pairs.empty()) {
            result->error = "toads place " + w[2] + ": no such toad and row";
            return true;
        }
        result->commands.push_back(toadDrag(pieces[pairs[0].first], pairs[0].second));
        result->commands.push_back("wait 1200");
        result->said.push_back("toads: toad at " + std::to_string(pieces[pairs[0].first].x) + ","
                               + std::to_string(pieces[pairs[0].first].y) + " to row " + std::to_string(pairs[0].second)
                               + " (" + w[2] + ")");
        return true;
    }
    if (w.size() >= 3 && w[1] == "send" && w[2] == "right") {
        /* `toads send right [N]`: N times, when a toad that can cross is free, it to its row */
        int count = w.size() >= 4 ? atoi(w[3].c_str()) : 1;

        for (int i = 0; i < count; i++) {
            result->commands.push_back("wait until toadsAvailable > 0"); /* (the toads come back for the next trip) */
            result->commands.push_back("toads place right");
        }
        return true;
    }
    result->error = "toads: expected `toads match`, `toads place right|dead|wrong` or `toads send right [N]`";
    return true;
}


/* ---- Stone Rise (scene 12) -------------------------------------------------------------------------
 *
 * A staircase of hexagonal cells (hexCells, 13 rows of 9) from the bottom to the top. Zoombinis are
 * put on the cells listed in listedCells; the stones between them light when the Zoombinis either
 * side of a feature stone (a cell whose snoid is 510 hair, 511 eyes, 512 nose, 513 feet) share that
 * feature. (First, a dump of the board; the solver follows.)
 */

bool stoneOpen_()
{
    return currentScene == 12;
}

/* The feature (0-3) a stone cell tests, or -1 if it is not a feature stone. */
int stoneFeature(const HexCell &cell)
{
    return cell.state != 500 && cell.snoid >= 510 && cell.snoid <= 513 ? cell.snoid - 510 : -1;
}

/* Where a stone's two neighbours are, the way the path lights them (lightPath: back and ahead). */
bool stoneNeighbours(const HexCell &cell, int *back, int *ahead)
{
    *back = cell.links[5] != -1 ? cell.links[5] : cell.links[4] != -1 ? cell.links[4] : cell.links[3];
    *ahead = cell.links[0] != -1 ? cell.links[0] : cell.links[1] != -1 ? cell.links[1] : cell.links[2];
    return *back != -1 && *ahead != -1;
}

struct StoneEdge
{
    int a, b; /* listed cell indexes (from 1) either side of a feature stone */
    int feature;
};

/* The stones that hold two listed cells apart: each wants its two Zoombinis to share its feature. */
std::vector<StoneEdge> stoneEdges()
{
    std::vector<StoneEdge> edges;
    std::vector<int> listedAt(117, 0);

    for (int i = 1; i <= listedCount; i++)
        listedAt[listedCells[i]] = i;
    for (int cell = 0; cell < 117; cell++) {
        int feature = stoneFeature(hexCells[cell]), back, ahead;

        if (feature >= 0 && stoneNeighbours(hexCells[cell], &back, &ahead) && listedAt[back] && listedAt[ahead])
            edges.push_back({listedAt[ahead], listedAt[back], feature});
    }
    return edges;
}

struct StonePlan
{
    std::vector<View *> views;
    std::vector<int> zoombiniAt; /* per listed cell (from 1): the Zoombini's index, or -1 */
    std::vector<bool> used;
    std::vector<StoneEdge> edges;
    long nodes = 0;
};

bool stoneAssign(StonePlan &plan, int i)
{
    if (i > listedCount)
        return true;
    if (++plan.nodes > 2000000)
        return false;
    for (size_t z = 0; z < plan.views.size(); z++) {
        if (plan.used[z])
            continue;
        bool fits = true;

        for (const StoneEdge &e : plan.edges) {
            int other = e.a == i ? e.b : e.b == i ? e.a : 0;

            if (other && other < i && plan.zoombiniAt[other] >= 0
                && viewSnoid(plan.views[z])->features[e.feature]
                       != viewSnoid(plan.views[plan.zoombiniAt[other]])->features[e.feature])
                fits = false;
        }
        if (!fits)
            continue;
        plan.used[z] = true;
        plan.zoombiniAt[i] = (int)z;
        if (stoneAssign(plan, i + 1))
            return true;
        plan.used[z] = false;
        plan.zoombiniAt[i] = -1;
    }
    return false;
}

bool stonePlanFor(StonePlan *plan)
{
    plan->views = zbDebugZoombiniViews();
    plan->zoombiniAt.assign(listedCount + 1, -1);
    plan->used.assign(plan->views.size(), false);
    plan->edges = stoneEdges();
    return (int)plan->views.size() >= listedCount && stoneAssign(*plan, 1);
}

long stoneLit()
{
    long lit = 0;

    for (int i = 1; i <= listedCount; i++)
        lit += hexCells[listedCells[i]].state == 508 ? 1 : 0;
    return lit;
}

bool stoneCommand(const std::vector<std::string> &w, OracleResult *result)
{
    if (!stoneOpen_()) {
        result->error = "stone: Stone Rise (scene 12) isn't open";
        return true;
    }
    if (w.size() == 2 && w[1] == "dump") {
        result->said.push_back("level " + std::to_string(stoneRiseLevel) + ", listed " + std::to_string(listedCount)
                               + ", startState " + std::to_string(startState));
        for (int i = 1; i <= listedCount; i++)
            result->said.push_back("listed " + std::to_string(i) + " = cell " + std::to_string(listedCells[i]));
        for (int cell = 0; cell < 117; cell++) {
            const HexCell &c = hexCells[cell];

            if (!c.state || c.state == 500)
                continue;
            std::string line = "cell " + std::to_string(cell) + " state " + std::to_string(c.state) + " snoid "
                               + std::to_string(c.snoid) + " links";

            for (int k = 0; k < 6; k++)
                line += " " + std::to_string(c.links[k]);
            result->said.push_back(line);
        }
        return true;
    }
    if (w.size() == 1) {
        /* `stone`: an arrangement of the Zoombinis on the listed cells where every feature stone has its two
           neighbours sharing its feature */
        StonePlan plan;

        if (!stonePlanFor(&plan)) {
            result->said.push_back("stone: no arrangement works");
            return true;
        }
        for (int i = 1; i <= listedCount; i++)
            result->said.push_back("cell " + std::to_string(listedCells[i]) + " (place " + std::to_string(i) + ") <- zoombini "
                                   + std::to_string(plan.zoombiniAt[i]));
        result->said.push_back("(found in " + std::to_string(plan.nodes) + " nodes)");
        return true;
    }
    if (w.size() >= 3 && w[1] == "seat" && w[2] == "right") {
        /* `stone seat right`: the next Zoombini of the arrangement onto its cell (in cell order) */
        StonePlan plan;

        if (!stonePlanFor(&plan)) {
            result->error = "stone seat: no arrangement works";
            return true;
        }
        for (int i = 1; i <= listedCount; i++) {
            View *view = plan.views[plan.zoombiniAt[i]];

            if (hexCells[listedCells[i]].snoid != view->id) {
                result->commands.push_back("drag zoombini " + std::to_string(plan.zoombiniAt[i]) + " place " + std::to_string(i));
                result->commands.push_back("wait 1000"); /* (the game takes the drop a moment after the release) */
                result->said.push_back("stone: zoombini " + std::to_string(plan.zoombiniAt[i]) + " to place " + std::to_string(i));
                return true;
            }
        }
        result->error = "stone seat: everyone is on their cell";
        return true;
    }
    if (w.size() >= 3 && w[1] == "seat" && w[2] == "wrong") {
        /* `stone seat wrong`: a Zoombini that shares no feature with a seated one, on the free cell the other side of their stone */
        std::vector<View *> views = zbDebugZoombiniViews();
        std::vector<StoneEdge> edges = stoneEdges();

        for (const StoneEdge &e : edges)
            for (int pair = 0; pair < 2; pair++) {
                int seated = pair ? e.b : e.a, free = pair ? e.a : e.b;
                View *other = findView(hexCells[listedCells[seated]].snoid);

                if (!other || hexCells[listedCells[free]].snoid > 0 || hexCells[listedCells[free]].snoid == -1)
                    continue;
                for (size_t z = 0; z < views.size(); z++) {
                    bool onACell = false;

                    for (int i = 1; i <= listedCount; i++)
                        onACell = onACell || hexCells[listedCells[i]].snoid == views[z]->id;
                    if (!onACell && viewSnoid(views[z])->features[e.feature] != viewSnoid(other)->features[e.feature]) {
                        result->commands.push_back("drag zoombini " + std::to_string(z) + " place " + std::to_string(free));
                        result->commands.push_back("wait 1000");
                        result->said.push_back("stone: zoombini " + std::to_string(z) + " to place " + std::to_string(free) + " (wrong)");
                        return true;
                    }
                }
            }
        result->error = "stone seat wrong: no free cell beside a seated Zoombini has one that misfits";
        return true;
    }
    if (w.size() >= 3 && w[1] == "send" && w[2] == "right") {
        /* `stone send right [N]`: N times (all of them), the next Zoombini to its cell */
        int count = w.size() >= 4 ? atoi(w[3].c_str()) : listedCount;

        for (int i = 0; i < count; i++)
            result->commands.push_back("stone seat right");
        return true;
    }
    result->error = "stone: expected `stone`, `stone dump`, `stone seat right` or `stone send right [N]`";
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
    if (!w.empty() && w[0] == "ferry")
        return ferryCommand(w, result);
    if (!w.empty() && w[0] == "toads" && w.size() > 1)
        return toadsCommand(w, result);
    if (!w.empty() && w[0] == "stone")
        return stoneCommand(w, result);
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
    if (name == "ferryIdle") {
        *value = ferryBusy() ? 0 : 1; /* nothing is walking back, sailing or arriving */
        return true;
    }
    if (name == "ferrySeated") {
        long seated = 0;

        for (View *view : zbDebugZoombiniViews())
            seated += viewSnoid(view)->chosen != 0 ? 1 : 0;
        *value = seated;
        return true;
    }
    if (name == "toadsAvailable") {
        *value = toadsOpen_() ? (long)toadMatching(toadPieces(), true).size() : 0; /* the toads that could cross from a free row now */
        return true;
    }
    if (name == "toadsOnBoard") {
        long onBoard = 0;

        if (toadsOpen_())
            for (View *view = viewListEnd(1); view; view = view->next) {
                const unsigned char *body = (const unsigned char *)&view->body;

                if ((view->flags & 0x980002) == 0x980002 && *(const short *)(body + 0xc0) == 0 && body[0xc2])
                    onBoard++; /* the toads on the board: set down and hopping */
            }
        *value = onBoard;
        return true;
    }
    if (name == "stoneLit") {
        *value = stoneOpen_() ? stoneLit() : 0; /* the Zoombinis on lit cells */
        return true;
    }
    return false;
}
