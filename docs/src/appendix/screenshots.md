# Screenshots

The gameplay chapters (and a few others) mark where an image of the running game belongs with a **placeholder**: a block quote that begins

```text
> 📷 **Screenshot: `isle-feature-panel`**
> *What the image shows.*
> *Capture:* how to reach that moment.
```

Ids are lower-case words with hyphens, unique across the book. The table below is generated from them.

## Capturing

| From | How |
| --- | --- |
| the port in a browser | `uv run port package && uv run port serve`, play, and use the browser's screenshot tool on the canvas (the game's area is 640×480 and scales in whole multiples) or open the page with `?screenshot` and read `/screenshot.bmp` from the page's file system |
| the headless port | `uv run port run --headless --seconds 60 --click 12000:320,240 --screenshot build/port/shot.bmp`: scripted clicks (`--click MS:X,Y`, or `:press`/`:move`/`:release` to drag) reach a scene without a window; the BMP is the 640×480 screen. Convert to PNG with any tool |
| the original, in the VM | `uv run vm run`, play, `uv run vm screenshot` (a PNG of the VM's screen): useful for comparing the port with the original |
| movies | `uv run assets frames LOGO025 1 701` draws frames of the converted intro as PNGs in `build/movie-frames/` |
| backdrops and art | the images in `assets/<ARCHIVE>/tBMP/` are the game's own pictures (scene backdrops are the 640×480 ones, `5000.png` and nearby); use them for a clean, UI-free view |

Reaching a given scene quickly: from Zoombini Isle, make a party and set out; the map's practice mode (Ctrl-P, then `1`-`4`) opens every place at the chosen level without touching a real journey; and `uv run port run --headless` with `--click` can script the whole route. Hidden scenes need the cheat codes described in [Hidden scenes](../gameplay/hidden.md).

Use PNG, at the game's native 640×480 where you can, named `<id>.png`.

## Adding one

1. Save the image as `docs/src/images/<id>.png`.
2. `uv run book screenshots --embed` inserts `![…](…/images/<id>.png)` above the placeholder and leaves the placeholder as the caption.
3. `uv run book screenshots --update` refreshes the index below (a test checks it is current), and `uv run book screenshots` lists what is still missing.

## Index

<!-- screenshots:start -->
| Id | Chapter | What it shows | How to capture it |
| --- | --- | --- | --- |
| `camp1-overview` | [gameplay/camps](../gameplay/camps.md) | Shelter Rock: the scrolling rows of camp slots, the Zoombinis standing in them, and the buttons at the right edge. | finish Pizza Pass (or use practice mode off with a saved game that has). |
| `camp1-drag` | [gameplay/camps](../gameplay/camps.md) | A Zoombini picked up from its slot and being dragged toward the "party" area. | click and hold on a Zoombini in the camp. |
| `camp2-book` | [gameplay/camps](../gameplay/camps.md) | Shade Tree: the "book" of Zoombinis waiting there, with its scroll arrows. | finish Stone Rise or Mudball Wall. |
| `cliffs-overview` | [gameplay/group1](../gameplay/group1.md) | The cliffs: two bridges, upper and lower, with Zoombinis waiting at the left and the buttons at lower right. | start a new game, make a party, click button 6, wait for the journey. |
| `cliffs-sneeze` | [gameplay/group1](../gameplay/group1.md) | A Zoombini turned back by a sneezing cliff. | send a Zoombini across the wrong bridge. |
| `caves-overview` | [gameplay/group1](../gameplay/group1.md) | The caves: four doors in the rock face, the characters at them, and Zoombinis queuing. | complete Allergic Cliffs. |
| `caves-remark` | [gameplay/group1](../gameplay/group1.md) | A guard speaking one of its remarks. | wait on the screen. |
| `pizza-overview` | [gameplay/group1](../gameplay/group1.md) | Pizza Pass: the pizza being assembled in the middle with the topping buttons to its left, and the trolls waiting. | complete Stone Cold Caves. |
| `pizza-trolls` | [gameplay/group1](../gameplay/group1.md) | The trolls Arno, Willa and Shyler, each with a thought bubble of what they want. | higher levels have more trolls. |
| `pizza-yuck` | [gameplay/group1](../gameplay/group1.md) | A troll reacting to a pizza it dislikes. | serve a pizza with a topping the troll doesn't want. |
| `ferry-overview` | [gameplay/group2](../gameplay/group2.md) | The river with Captain Cajun's ferry, the landing places and the Zoombinis waiting on the bank. | from Shelter Rock, set out with button 1. |
| `ferry-crossing` | [gameplay/group2](../gameplay/group2.md) | The ferry mid-river carrying Zoombinis, with Captain Cajun at the helm. | place some Zoombinis on the ferry's seats and let it cross. |
| `toads-overview` | [gameplay/group2](../gameplay/group2.md) | The river with the grid of lily pads and toads. | complete the ferry. |
| `toads-hop` | [gameplay/group2](../gameplay/group2.md) | A Zoombini hopping across lily pads. | start the crossing once the board is set. |
| `stonerise-overview` | [gameplay/group2](../gameplay/group2.md) | The cliff of stones with the Zoombinis waiting at the bottom and the cells above. | complete the toads. |
| `stonerise-lit-path` | [gameplay/group2](../gameplay/group2.md) | A path of lit stones between Zoombinis that share a feature. | place Zoombinis in adjacent cells. |
| `fleens-overview` | [gameplay/group3](../gameplay/group3.md) | The Fleens scene: a row of Fleens (small creatures) beside a line of Zoombinis. | from Shelter Rock, set out with button 2. |
| `fleens-pick` | [gameplay/group3](../gameplay/group3.md) | A Zoombini dragged beside a fleen; the pair walking on together. | drag a Zoombini to a fleen. |
| `hotel-overview` | [gameplay/group3](../gameplay/group3.md) | The hotel front: rows and columns of rooms, with labels along the top and left, and Zoombinis arriving. | complete the Fleens. |
| `hotel-rooms` | [gameplay/group3](../gameplay/group3.md) | Zoombinis sent into rooms; a room that doesn't fit stays dark. | send a Zoombini to a room. |
| `mudball-overview` | [gameplay/group3](../gameplay/group3.md) | The wall of 5×5 stones with a rope along its top and the pond below; a Zoombini on the rocks. | complete the hotel. |
| `mudball-codes` | [gameplay/group3](../gameplay/group3.md) | The codes box showing the chosen row and column values. | click the controls to set a code. |
| `lion-overview` | [gameplay/group4](../gameplay/group4.md) | The lair: the lion's paw over the golden stepping stones across the chasm, with Zoombinis at the left. | from Shade Tree, set out. |
| `lion-places` | [gameplay/group4](../gameplay/group4.md) | Zoombinis standing on stones that match the feature the lion wants. | drag Zoombinis onto the stones. |
| `mirror-overview` | [gameplay/group4](../gameplay/group4.md) | The mine: a boulder wedged overhead, wooden trestles and a rail track, with two rows of Zoombinis facing each other. | complete the Lion's Lair. |
| `mirror-grid` | [gameplay/group4](../gameplay/group4.md) | The hex grid of Zoombinis filling in, neighbours sharing features. | at higher levels. |
| `bubble-overview` | [gameplay/group4](../gameplay/group4.md) | The dark chasm with the Zoombinis' bubbles rising over it. | complete the Mirror Machine. |
| `bubble-lines` | [gameplay/group4](../gameplay/group4.md) | The squares and lines the sequence is set on. | at higher levels. |
| `hidden-catch` | [gameplay/hidden](../gameplay/hidden.md) | The hidden catching game. | on the map, type the cheat code that `isCheat(0x469110d3, 0x1e1c32f2)` tests for, then click hotspot 9. |
| `hidden-targets` | [gameplay/hidden](../gameplay/hidden.md) | The hidden targets game. | on the map, enter the code that `isCheat(0xc07a877d, 0xedfa7273)` tests for, then click hotspot 8. |
| `journey-overview` | [gameplay/index](../gameplay/index.md) | The map screen with all sixteen hotspots visible and the map's text box showing "choose a level". | from the map (scene 1), in practice mode. |
| `isle-overview` | [gameplay/isle](../gameplay/isle.md) | Zoombini Isle: the panel of feature buttons (four rows of five) at lower left, the Zoombini being made in the middle, and the queue of finished Zoombinis waiting along the shore. | start a new game; you arrive here after the logo. |
| `isle-feature-panel` | [gameplay/isle](../gameplay/isle.md) | Close-up of the feature panel: hair, eyes, nose and feet choices (5 each) and the seven panel buttons below it. | same scene, crop to the panel (`isleButtons`, x 3-198, y 304-478). |
| `isle-sending-off` | [gameplay/isle](../gameplay/isle.md) | A party of Zoombinis walking off toward the map after the player clicks the "send off" button. | make at least the minimum party, click button 6. |
| `map-overview` | [gameplay/map-and-journey](../gameplay/map-and-journey.md) | The map with its sixteen hotspots drawn on the terrain and the text box at upper left. | from Zoombini Isle click the map button (panel button 5). |
| `map-practice-levels` | [gameplay/map-and-journey](../gameplay/map-and-journey.md) | The map in practice mode: the level list (1-4) and the "snoids to practice with" count in the text box. | open the map with no saved journey so every hotspot is available. |
| `journey-travel` | [gameplay/map-and-journey](../gameplay/map-and-journey.md) | A map screen mid-journey: Zoombinis walking along a path, with the map's name and the grid of places visited. | leave a puzzle for the next without "transitions off" (Ctrl-T). |
| `journey-population-sign` | [gameplay/map-and-journey](../gameplay/map-and-journey.md) | The "zoombiniville population N" sign. | travel to or from Zoombiniville. |
| `start-logo` | [gameplay/start](../gameplay/start.md) | The intro logo movie playing full screen (a frame from `Logo025.MOV`), at 640×480. | the first seconds after pressing Play in the port; or `uv run assets frames LOGO025 1 701` for stills from the converted movie. |
| `dialog-keep-party` | [gameplay/start](../gameplay/start.md) | The dialog "the current party of zoombinis will be lost if you go to the map" with its LOSE 'EM / KEEP 'EM buttons. | from a puzzle, click the map button while Zoombinis are in the party. |
| `dialog-games` | [gameplay/start](../gameplay/start.md) | The saved-games dialog (LOAD / SAVE) listing games. | press Ctrl-L (or Ctrl-S) at any time. |
| `dialog-options` | [gameplay/start](../gameplay/start.md) | The options/help dialog with the ON/OFF toggles (music, sound, less/more action, hide cursor, sticky mouse…). | press `?` or `/`. |
| `town-overview` | [gameplay/zoombiniville](../gameplay/zoombiniville.md) | Zoombiniville: one of the six 320-pixel-wide screens of the town, with townsfolk walking and the settled Zoombinis around. | finish Bubblewonder Abyss, or open the town from the map (hotspot 16). |
| `town-monument` | [gameplay/zoombiniville](../gameplay/zoombiniville.md) | A monument's plaque open: "this monument was made to honor the zoombinis who:" and the journey it records. | click a building in the town. |
| `town-clock` | [gameplay/zoombiniville](../gameplay/zoombiniville.md) | The clock tower, with hands showing the real time; clicking it winds them. | click the clock (`townsfolkViews[0]`). |
<!-- screenshots:end -->
