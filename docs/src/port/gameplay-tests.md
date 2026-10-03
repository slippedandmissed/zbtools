# Instrumented gameplay tests

`uv run gameplay` plays scripted flows on the headless port, checks the game's own state as it goes (`assert`: the scene reached, a puzzle's counters, the party, the level) and compares the few screenshots it takes with baselines checked into the repository, so a change that breaks a scene, a puzzle's mechanics or a dialog, or alters what the game draws, is caught. The aim is confidence that the port is playable from start to end: the cases are *instrumented* (they read the game's globals through the [debug tools](debug-tools.md)), and together should cover every scene and every major path through the game; pictures are the exception, kept for what only a picture can show.

```sh
uv run port build headless_wasm      # once, and after changing the port or decomp/
uv run gameplay                        # play every case and compare
uv run gameplay cliffs-right-and-wrong # one case (or several)
uv run gameplay --rebaseline           # write the pictures as the new baselines
```

## What a case is

`tests/gameplay/cases.toml` has one table per case: a `description`, and `commands`, a script of [debug-tool](debug-tools.md) commands that takes the game somewhere and plays something. `screenshot NAME` takes a picture to compare; `assert NAME VALUE` checks the game's own state (a failed assertion fails the case); `wait until` waits for the game rather than the clock. Every case starts by turning "sticky mouse" off and seeding the game's random numbers (`seed`, 1 unless the case sets `seed`), and ends by itself: leave out `quit`. A case must check something, an `assert` or a `screenshot` (one that does neither can't fail); a case about where the game goes needs no picture.

```toml
[cliffs-right-and-wrong]
description = "Zoombinis are sent over both bridges of Allergic Cliffs: the cliff sneezes at those it dislikes"
seed = 7
commands = "party 8; scene 7; wait 6000; screenshot start; drag zoombini 0 place 1; wait 1500; ...; assert sentBackCount 2; screenshot after"
```

The baselines are `tests/gameplay/baselines/<case>/<NAME>.png`: 640×480 screens as 256-colour PNGs (about 170 KB each; the screen has at most 256 colours, so nothing is lost). Each case runs on a fresh C: drive, so none sees what another saved, and several run at once (`--jobs`, 4 by default).

### What the cases cover

| Cases | What they do |
| --- | --- |
| `open-*` | **one case per scene** (`open-map`, `open-isle`, `open-shelter-rock`, ..., `open-hidden-catch-21`): the scene opens, with the state it should start in asserted (the level of its group, the party's Zoombinis on screen, its counters at zero, go-ready off) and a picture, so one that opens into the wrong state or draws wrongly fails by name |
| `transition-*` | **leaving a scene by the player's own click and arriving where the code says**: the intro (a new game, and each kind of saved scene it resumes), the isle, the map's locked and unlocked hotspots, both camps' set-out buttons (with a full party, and with too few), their map buttons, the journey scene's click, practice mode, the hidden games' exits; most assert state alone and take no picture |
| `cliffs-*` | **Allergic Cliffs played to the end with an [oracle](debug-tools.md#oracles-solving-a-puzzle-by-its-rule)**: sixteen Zoombinis sent over the right bridges at each of the four levels and the go button leaving for Stone Cold Caves with all of them; a wrong send that sneezes and comes back; the six-sent-back limit; leaving with the one that crossed |
| `tunnels-*` | **Stone Cold Caves played to the end with its oracle**: Zoombinis (8 or 16) sent to doors that let them in at each of the four levels and the go button leaving for Pizza Pass with all of them; a wrong door that turns one back (one turn-back used); running out of turn-backs (the count is *set* to two or one for the test, to avoid sixteen real turn-backs); leaving with the one that got in |
| `pizza-win-*`, `pizza-wrong-and-partial-then-right`, `pizza-out-of-pizzas` | **Pizza Pass played to the end with its oracle**: the pizza each troll wants served at each of the four levels (1, 2, 3 and 3 trolls), every troll satisfied, the go button leaving for Shelter Rock with the party and group 1's bit set; a refused pizza and one short of what the troll wants; running out of pizzas, which does not end the puzzle |
| `ferry-win-*`, `ferry-wrong-then-right`, `ferry-leave-with-three` | **Captain Cajun's ferry played to the end with its oracle**: the Zoombinis (8 or 16) seated at each of the four levels by a seating the oracle searches for, none sent back, the go button crossing to the toads with all of them; a misfit sent back; going with only three aboard |
| `toads-win-level-1..3`, `toads-dead-end-strands-one` | **Titanic Tattooed Toads played to the end with its oracle**: eight toads put on rows they can cross from (as they come back for the next trip), all eight Zoombinis arrive, the go button leaves for Stone Rise; a toad put where it can't cross is stranded and the other seven still cross. Level 4 is not covered (it needs the swapping wand) |
| `stone-rise-win-level-1..2`, `stone-rise-misfit-stays-dark` | **Stone Rise (levels 1 and 2) played to the end with its oracle**: the Zoombinis (16 or 8) arranged so every stone lights, all cells lit, the go button leaving for Shade Tree with them and group 2's bit set; a misfit that stays dark and a departure with only the lit one. Levels 3 and 4 are not covered |
| `intro-logo`, `high-levels`, `map-practice`, `journeys` | the intro, the highest level of three puzzles, the map's practice mode, the journey screens between places |
| `isle-make-party-and-send`, `camp-drag` | sixteen Zoombinis made by clicking the isle's panel and sent to the ship; a Zoombini dragged in Shelter Rock |
| `cliffs-right-and-wrong`, `pizza-wrong-pizza`, `pizza-wants`, `toads-wrong-piece` | **wrong moves**: the cliff sneezing, trolls refusing a pizza, a toad put in a row it doesn't match |
| `ferry-load-and-go`, `toads-place-and-go`, `stonerise-place`, `fleens-pick`, `hotel-rooms`, `mudball-codes`, `lion-places`, `caves-door` | the puzzles' mechanics: loading, placing, picking, sending, with the pictures along the way |
| `town-monuments` | a monument's plaque and the clock tower |
| `dialog-*` | the options (a toggle and OK), saving (cancel), and "keep the party?" both ways, each asserting the dialog closed and the scene it left to |

These play the **mechanics** of each puzzle, not whole solutions: a puzzle is won by working out its rule from what the Zoombinis do, which a script can't, so no case plays one to its end. Adding the final steps (every Zoombini through, the way on to the next scene) is the natural next case once a rule can be read from the game's own state (`get`/`wait until` on its globals, [Debug tools](debug-tools.md)).

What is covered and what is still to cover is in the [coverage map](gameplay-coverage.md).

## Watching a run

The run prints as it goes (it is the longest job of the pull request pipeline, so its page should show what it is doing): a `start` line when a case begins, a line when it finishes, in the order they finish, with how far the run is (`[12/41  05:31]`: cases done of all, minutes and seconds since the start) and how long the case took, a `retry` line when a case's pictures differed and it is played again, and, after a minute of nothing else, a `...` line naming the cases still running and for how long. A case that hangs is therefore visible while it hangs, not when its time runs out. (Output is line-buffered, so it reaches a pipe or CI's log at once.)

## Comparing

A picture fails when more than the case's `tolerance` (0.5% by default) of its pixels differ, a pixel differing when some channel is more than 24 (of 255) off. A case also fails if it crashes (a trap in the game's code), an `assert` fails, it doesn't finish within `seconds` (a wait that never ends), a picture has no baseline, or a baseline has no case (`STALE`).

The game runs in real time, but with the seed and the scripts' waits its pictures are mostly repeatable: in the full runs made while writing these, most cases were identical to the pixel every time, and a few needed a second try now and then (`open-hidden-catch`, whose Zoombinis cross at timed moments (it has a larger `tolerance`), and `ferry-load-and-go`, whose boat is mid-crossing in one picture: that case has a larger `tolerance` for it). A case whose pictures differ is therefore played once more before it counts as failed (`--retries`), and a case that passed only the second time is reported as such: one that needs it often should wait on the game's state (`wait until`) instead of the clock. A virtual clock for the headless build (time advancing by the game's own frames, not the machine's speed) would remove the dependence on timing altogether; it is not done.

A case whose scene animates (the isle's sea cycles its colours; Zoombinis, toads and machines idle) has a larger `tolerance`, set from what differed when the cases were run on a machine kept busy on purpose (eight busy loops beside six cases at once, harsher than CI): the isle 10%, Mudball Wall 3%, the toads 4%, the targets 2%. A looser picture check is why those cases also assert state: what is on the screen is checked by the picture only roughly, what the game believes exactly.

## Rebaselining

When a change is meant to alter what the game draws, run `uv run gameplay --rebaseline` (or name the cases) and commit `tests/gameplay/baselines/`. It writes only the pictures that changed, so git sees only real differences, removes baselines no case takes any more, and says what it wrote; the report then shows the **before and after** of every picture it changed, which is what to review in the pull request. The baselines are binary files, so each change adds to the repository's size: keep cases few and meaningful rather than a picture of everything.

A new case: add it to `cases.toml`, run `uv run gameplay NAME --rebaseline`, look at the pictures (they are in `tests/gameplay/baselines/NAME/`), and commit them. A test (`tests/test_gameplay.py`, part of `uv run lint`) checks that every picture a case takes has a baseline, that none is left over, and that they are whole screens, so a forgotten file is caught without playing anything.

## The report

Every run writes `build/gameplay/report.html`: **one self-contained file** (its pictures are inside it, so it can be opened anywhere, sent, or attached) with the counts, every failing case first (its commands, the game's own output if it crashed, and for each failing picture the **baseline, the new picture and the differences in red** side by side), and a table of all cases. The failing pictures are also files in `build/gameplay/<case>/` (`NAME.actual.png`, and `NAME.diff.png`, the three side by side). If a change breaks so many pictures that the report would be too big, the ones past the limit are listed without their images (the files are in `build/gameplay/`).

## In the pull request pipeline

The `gameplay` job in `.github/workflows/pr.yml` builds the headless port, runs `uv run gameplay` and fails the pull request if any case fails. What it leaves for a failed run:

- **The run's page** shows a summary of the failures (`--summary "$GITHUB_STEP_SUMMARY"`: a table of cases and what was wrong, as Markdown, which GitHub renders on the page itself, with a link to the artifacts). It carries no pictures: they are in the report.
- **`gameplay-report.html`**, in the run's artifacts: the report, uploaded unzipped (`actions/upload-artifact` with `archive: false`, which names the artifact after its file), so it is one click to download (a login is needed) and opens straight in a browser. Whether GitHub shows an HTML artifact in its own page rather than downloading it is not something this relies on; showing the pictures in the page itself would need them hosted somewhere (a preview deployment of the report, say), which this does not set up.
- **`gameplay-pictures`**: the failing pictures as files (zipped).

Locally the report is `build/gameplay/report.html`; open it in a browser.

## Limits

- The baselines are made by the headless build, whose pictures should be the same on any machine (the game is software-rendered and integer-only), but they were first made on one environment and checked in another: if the pipeline shows pictures differing on a machine that should agree, look at the report's differences before anything else (a font, a library, timing) and say so.
- Timing: slower machines move animations by a frame at worst; pictures taken mid-animation (`ferry-load-and-go`'s moving ferry, `cliffs-right-and-wrong`'s sneeze) are the likeliest to differ, and `--retries` absorbs a one-off.
- A baseline records what the game does, not what it should do: it can only say that something changed, and a person decides whether that is a bug.
