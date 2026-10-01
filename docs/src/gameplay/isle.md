# Zoombini Isle

Where the game begins: the player builds the Zoombinis who will make the journey.

## On screen

> 📷 **Screenshot: `isle-overview`**
> *Zoombini Isle: the panel of feature buttons (four rows of five) at lower left, the Zoombini being made in the middle, and the queue of finished Zoombinis waiting along the shore.*
> *Capture:* start a new game; you arrive here after the logo.

> 📷 **Screenshot: `isle-feature-panel`**
> *Close-up of the feature panel: hair, eyes, nose and feet choices (5 each) and the seven panel buttons below it.*
> *Capture:* same scene, crop to the panel (`isleButtons`, x 3-198, y 304-478).

> 📷 **Screenshot: `isle-sending-off`**
> *A party of Zoombinis walking off toward the map after the player clicks the "send off" button.*
> *Capture:* make at least the minimum party, click button 6.

## Scene facts

| | |
| --- | --- |
| Scene | 3 |
| Module / archive | `decomp/isle.cpp` / `PICKER` (`Picker.MHK`) |
| Input | `isleGroups[2]`: feature buttons → `featureButtonClicked`, panel buttons → `isleButtonClicked` |
| Roles | makes Zoombinis; holds a waiting party (`waitingParties()`); starting point of the journey |

## Entry points

| Function | Address | Role |
| --- | --- | --- |
| `openIsle` | `0x43e6b6` | loads `Picker.MHK`'s backdrop, images, scripts and sounds; adds views and the queue's places; brings the waiting party back; sets up the Zoombini being made; offers to load a saved game first if asked; plays a hint (or, from the camp, a remark about how many Zoombinis are left to make) |
| `isleFrame` | `0x43ebf4` | leaves once asked (`sceneDue`) and sound 996 has finished |
| `featureButtonClicked` | `0x43ecbb` | feature buttons 1-20 (four groups of five): pick or drop that feature for the Zoombini being made |
| `isleButtonClicked` | `0x43ee5d` | panel buttons 1-7 (below) |
| `isleKey` | | keyboard shortcuts |
| `closeIsle` | | frees everything |

## The panel's seven buttons

| Button | Does |
| --- | --- |
| 1 | makes the chosen Zoombini and sends it to the queue's first free place (while fewer than 625 exist) |
| 2 | the Zoombini says something |
| 3 | renames it |
| 4 | picks another at random (with Ctrl, fills the queue with random ones) |
| 5 | goes to the map |
| 6 | with a full enough party, sends it off and leaves; otherwise may remark on Zoombinis left to make |
| 7 | takes a Zoombini from the queue back to be remade |

## Where the rules live

- **How many are enough:** `checkEnoughChosen` sets `enoughToLeaveChosen` when the chosen count reaches `enoughToLeave`, or the whole population (625) has been made.
- **Which kinds exist:** `zoombiniCounts()` in `gameState` counts how many of each of 5⁴ kinds have been made; `pickZoombiniMade` picks random features until it hits a kind with fewer than two.
- **Drawing the Zoombini being made:** `drawZoombiniParts` composes it from the feature images in the isle's image bank (`drawIsleImage`, by hot spot); `drawFeatureButtons` and `drawIsleButtons` draw the panel from `isleButtonImages`.
- **The queue:** `isleQueue` (`0x43ffd5`) gives each place the nearest spot no earlier place has taken (`isleQueuePlaces`).

## Things to know

- `isleAllowsFeature` allows every feature, and the "refused" sound (1008) is never played: a hook for something the shipped game doesn't do.
- With Ctrl held and debugging on, button 1 first sets the count to 624, to test the "population full" ending.
- Names are built from syllable tables in `snoids.cpp` (`vowelSounds`, `consonants`, `nameEndings`, `consonantPairs`).
