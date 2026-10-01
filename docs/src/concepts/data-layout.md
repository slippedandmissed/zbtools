# How data is laid out, and matched

`uv run match-data` and `uv run define-data` rest on how BCC32 and TLINK32 lay a module's data out. (The code is in `src/zbtools/match_data.py`; its docstring has the exact placement rules.)

## The rules

- **Code reaches its own module's data through the segment** (`_DATA` or `_BSS`), with the offset in the instruction itself and a zero fixup displacement. A matching function's data reference therefore tells you where that global lives in the original: the instruction holds the absolute address. `match` masks those addresses, so `match-data` separately checks that every data reference in a decompiled function points where the original's does. A function can match and still read the wrong global; this check catches it.
- **Each object's `_DATA`** holds initialised globals and statics in definition order, then its string literals, padded to 4 bytes. In the original the modules follow each other without that padding where literals end, so padding isn't compared.
- **`_BSS` follows the same module order** as code and `_DATA`. It starts at `0x4aa410`, inside the zero padding of the `DATA` section's file data (TLINK32 pads to 0x200), not at the end of it.
- **Zero-initialised globals** may be in `_DATA` if the original wrote `= 0`; if the decompilation doesn't, BCC32 puts it in `_BSS`. Add the initialiser.
- **A hidden copy of an initialised local array** is data like any other and shows the original's initialiser.
- **String literals** are pooled per module in order of first use (see [Matching BCC32](bcc32-quirks.md)).

## Defining globals

Headers declare globals as they're found, by address: `extern short primes[5]; /* @data 0x4a0800 */`. `uv run define-data` defines the ones no source defines, in the module whose code uses them in the original (else the module of the globals around them), with initialisers rendered from the original's bytes as typed C++: numbers, text, pointers as the function, global or string they point to, structs and arrays in braces. Types are checked against BCC32's own `sizeof` through a probe file. It leaves a global for a person, saying why, when it can't be exact.

## Pitfalls it found

- **Arrays indexed from 1.** The original's address is an element *before* the array, so a declaration "starts an element early" and overlaps its neighbour. Declare the array at its real start and shift the uses (`a[i - 1]` compiles to the same instruction).
- **Second names for elements.** Code that uses one element by itself looks like a separate global (`view6000` is `campThingViews[9]`; `inputFlagsHigh` is the high byte of `inputFlags`). Fold them into the array or field.
- **Input groups list their items**: each scene's `Group` points at its buttons plus an item for the whole screen (`{0, 0, 640, 480}`), so button arrays have one more entry than there are buttons.
- **Genuine overlaps exist.** The tail view's drag cursor sits inside its body; `splitIntoGroups` can write more of `netGroups` than it clears, over its neighbours. The decompilation keeps the original's lengths and notes the overrun in the declaration.

`uv run report` shows the totals: 69% of the original's initialised data is placed and identical byte for byte, and every checked reference points where the original's does.
