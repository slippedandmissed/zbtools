# The camps: Shelter Rock and Shade Tree

Between the puzzle groups the party rests at a camp, where Zoombinis that have been left behind are kept and the player chooses who sets out next.

## Shelter Rock (scene 4)

> 📷 **Screenshot: `camp1-overview`**
> *Shelter Rock: the scrolling rows of camp slots, the Zoombinis standing in them, and the buttons at the right edge.*
> *Capture:* finish Pizza Pass (or use practice mode off with a saved game that has).

> 📷 **Screenshot: `camp1-drag`**
> *A Zoombini picked up from its slot and being dragged toward the "party" area.*
> *Capture:* click and hold on a Zoombini in the camp.

| | |
| --- | --- |
| Scene / module / archive | 4 / `decomp/basecamp.cpp` / `BASECAMP` (`BaseCamp.MHK`) |
| Input | `campGroupLists[2]`: buttons → `campButtonClicked`; the camp area → `campMouse` |
| State | `Camp` (625 `CampSlot`s: Zoombini, rectangle, name) from `gameState`, `campEnoughChosen`, `campPopulationFull` |

| Function | Address | Role |
| --- | --- | --- |
| `enterCamp` | `0x416789` | loads the slots from `gameState` and `BaseCamp.MHK`, returns the party from the journey (`returnToCamp`) and picks a greeting by how the journey went |
| `campIdle` | `0x417000` | per-frame: leaves once asked and sound 996 is done; tracks which of the scroll buttons (3-6) the cursor is over |
| `campButtonClicked` | `0x417108` | buttons 1 and 2 **set out** (button 1 to scene 10, group 2; button 2 to scene 13, group 3) *if enough Zoombinis are chosen*, else say why (a random one of three remarks); 3 leaves (to the map); 4-7 scroll while held |
| `campMouse` | `0x417350` | action 1 (down) / 2 (up): pick a Zoombini up from its slot, drop one in a slot or back, click the other things |
| `scrollCamp`, `drawCamp`, `compactCamp`, `insertCampRow` | `0x417cf1`… | the scrolling grid of slots |
| `leaveCamp` | `0x416ede` | saves the party that set out and frees everything |
| `returnToCamp` | `0x4184cd` | puts the Zoombinis back from the journey into the camp's slots |

Setting out sends the Zoombinis walking off (`sendSnoids(0x2a8, y, 0x2d)`, `markPlacedSnoids`) and plays sound 996.

## Shade Tree (scene 5)

> 📷 **Screenshot: `camp2-book`**
> *Shade Tree: the "book" of Zoombinis waiting there, with its scroll arrows.*
> *Capture:* finish Stone Rise or Mudball Wall.

| | |
| --- | --- |
| Scene / module / archive | 5 / `decomp/bctwo.cpp` / `BCTWO` (`bctwo.mhk`) |
| Input | `campGroups[2]`: buttons → `camp2Clicked`; the book area → `campDragged` |

| Function | Address | Role |
| --- | --- | --- |
| `openCamp2` | `0x4186dc` | the book of Zoombinis waiting here (kept at `gameState+0x3688`), the camp's Zoombinis, the party (which joins the book when it doesn't carry on), and a hint |
| `camp2Frame` | `0x418e62` | per-frame |
| `camp2Clicked` | `0x418fa7` | button 1 sets out for **scene 16** (group 4) if enough are chosen, else a random one of three remarks (sounds 20084…) |
| `campDragged` | `0x41914d` | dragging Zoombinis in the book; clicking elsewhere starts the camp's "thing" there (`campThingRects`: ambient animations) |
| `drawBook`, `scrollBook`, `bookEntryAt`, `makeBookRoom`, `refreshBook`, `dropEmptyBookRows` | `0x419c3a`… | the book |
| `addPartyToBook` | `0x41a23b` | adds the party after the last taken entry, else into free ones |

## Things to know

- **The population cap.** A camp can hold 625 Zoombinis, the whole population; `campPopulationFull` changes the remarks.
- **Group order.** Shelter Rock offers group 2 *or* group 3; both end at Shade Tree, which leads to group 4.
- **Unlocking.** Each camp's map hotspot is open only when the "left" bits in `gameState` say its group was finished (`gameState[0x50] & 0xf`, `+0x52`, `+0x51`).
- The camps share their input items with the isle's style: two button items and a whole-screen item, plus `campAreaItems`/`bookAreaItems` for the drag areas.

- **Palette.** Every scene copies the palette its images set (`loadedPalette`) into the one the screen fades in to (`targetPalette`) with `copyPaletteRange(10, 236)`, and Shelter Rock's own palette (`BaseCamp.MHK`'s shape list 1000, entries 10-228) is unlike the journey's. `enterCamp` once passed the two numbers the wrong way round, `(0xec, 10)`, copying only entries 236-245: the backdrop then came out in whatever palette the scene before had left (`uv run port run --headless --cmd "debug on; party 8; scene 9; wait 3000; scene 4 map; wait 20000"` showed it), and the debug command `paldiff` shows such a mismatch.
