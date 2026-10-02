/*
 * Oracles for the instrumented gameplay tests (see docs/src/port/gameplay-tests.md): debug commands that
 * read a puzzle's hidden rule from the game's own state and play a correct (or deliberately wrong) move
 * through real mouse input, so a case can solve a puzzle without knowing the answer in advance (the
 * answer depends on the random numbers, and a recorded move list would break whenever they shift).
 *
 * An oracle command is turned into plain debug commands (`drag ...`, `wait until ...`) at the moment
 * it runs, so it reads the state as it is then; a command that has to wait for the game expands into
 * itself again after the wait. This is port code, not decompiled code.
 */
#pragma once

#include <string>
#include <vector>

struct View;

/* The party's Zoombini views in the order `drag zoombini N` counts them (zbdebug.cpp). */
std::vector<View *> zbDebugZoombiniViews();

struct OracleResult
{
    std::vector<std::string> commands; /* plain debug commands to run next, in order */
    std::vector<std::string> said;     /* lines to print */
    std::string error;                 /* why the command failed, if it did */
};

/* Whether `words` is an oracle's command; if so, what it comes to. */
bool zbOracleCommand(const std::vector<std::string> &words, OracleResult *result);

/* Values the oracles compute for `get`, `assert` and `wait until` (`cliffsAcross`, ...). */
bool zbOracleValue(const std::string &name, long *value);
