# Matching BCC32: a field guide

Byte-matching means reproducing not just what the code does but how BCC32 4.5 compiled it. Almost all the effort goes into a handful of recurring effects. This is the distilled list; the original's quirks are in the source comments next to each function (search `decomp/` for "Not exact").

## Register allocation

BCC32 gives its three saved registers (`ebx`, `esi`, `edi`, in that order) to the most-used locals, and on a tie to the one used first. Everything else follows from that:

- **Declaration order matters.** Registers go to variables in declaration order among equals, so declare locals in the order the original's registers suggest.
- **Lifetimes can share a register.** Variables whose lives don't overlap can share one, even of different types. When one register holds several unrelated values in the original, try *one reused variable* as well as separate ones (`placeDialogList`, `runViewScript`).
- **Block scope decides sharing.** Declaring a loop index inside the block that uses it is what lets it share a register with a flag used elsewhere.
- **A local that lives across no call** goes in `eax`/`edx`/`ecx`, not a saved register.
- **`volatile`** keeps a local on the stack where BCC32 would give it a register, and keeps stores to otherwise-unused locals. It is the standard tool for "the original leaves this in memory"; the prototype must say `volatile` too when it's a parameter.
- Assigning inside a condition (`if ((p = find(id)) && p->x)`) keeps `p` in its stack slot; assigning first and testing `p` keeps it in `eax`.
- **Unused locals:** an unused scalar takes no space, but an unused array or struct does. Frame gaps in the original are reproduced with dummy arrays or structs (structs sit below arrays).

## Globals, structs and addresses

- **BCC32 hoists a global's address into a register by itself** when a function uses it often (`mov esi, 0x4b9b6c`). That only happens when the source writes the global directly (`heap.error`); a local pointer to it (`MemoryState *state = &heap`) becomes a register variable in declaration order and gets a different register. The reverse also occurs: a few functions only matched with a pointer local. Try both.
- The hoisted address is one *symbol's*: two separate globals are always addressed absolutely. So where the original addresses several places through one base register (`mov esi, 0x4af8ac` then `[esi+0x24]`), they are fields of one struct.
- **Arrays indexed from 1.** The original's `[array - size + i*size]` is `a[i - 1]` in the decompilation; BCC32 folds the `- 1` into the displacement. Declare the array where the data starts, not an element early.
- **String literals are pooled per module**, in order of first use, and a function with several literals addresses them from the pool's start (`lea eax, [edi+offset]`). The offsets are code, so a function's literals only match once every earlier literal-using function in its module is decompiled, in address order. Functions with few literals push each address instead. **Named strings** (`char msgFoo[]`) are pushed by address and stored in an unrelated order; the game's startup messages are like that.
- `DIBPort` is declared inside `#pragma pack(push, 4)` to reproduce both its size (`0xc8`) and its derived class's (`0xc6`).
- A structure copied through its address is the second half of a chained assignment (`a = b = c`).
- An initialised local array is copied from a hidden copy in `DATA` (`rep movsd`) where it's declared.

## Expressions

- A comparison's operand order follows the source (`exclude != i` and `i != exclude` compile differently). Between two register variables BCC32 puts the right operand first, so `j < done` becomes `cmp done, j`.
- `x = x * -1` compiles to `mov/neg/mov`, `x = -x` to `neg [x]`. `++x > N` loads, increments and compares in a register; `x++; if (x > N)` works in memory.
- `if (c < 1)` on a `char`, `c <= 0` and `c - 1 < 0` all differ. A one-case `switch` on an `unsigned short` compiles to 16-bit compares, on a `short` to `movsx`. `x + 200` on a register variable adds in the full register, `x += 200` on a `short` adds `bx`.
- Masks written with `~` differ from their positive forms (`op & ~3` vs `op & 0xfc`).
- Initialising several variables in a `for` header versus in declarations changes the setup order; `while (p && !found) p = p->next;` and a `for` with `break` lay out differently.
- Dead code is compiled (`else if (0) {...}`), and a `jmp` over a lone instruction is an `if` BCC32 resolved at compile time (an `unsigned short` tested `< 0`).
- **Switches**: the jump table lives inside the function and is relocated. BCC32 lays case bodies out in source order but numbers the table its own way, so write the `case` labels in the original's code order. A `switch` on a `char` whose cases start above 0 compiles as byte decrements; the original's table from 0 needs explicit empty `case 0:`/`case 1:` labels. `match` requires every absolute address into the function itself to point where the original's does, so a switch with its cases swapped cannot match by accident.
- Pascal order evaluates arguments left to right. `f(*cel++, *cel++, *cel++)` reproduces one original, but C++ leaves the order unspecified, so it's decompiled with indexing and marked `@zoombi32-functional`.
- Loads whose values go unused can come from an inline function called for nothing: BCC32 drops the body but still evaluates the arguments.

## Types the game shares with the engine

- The engine takes rectangles as `const Rect &`; the game passes its `ShortRect`s, each converted into a temporary by an inline constructor (`memcpy(this, &r, 8)`). With two in one call each gets its own register. Some engine modules call the same constructor out of line (`RECT_OUT_OF_LINE`).
- `Color` is constructed out of line (`__cdecl`) and returned through a hidden pointer; passing one by value copies a dword then a word. `RGBColor` has an inline three-argument constructor for the game's uses, while the engine calls the out-of-line one.
- The game has inline byte-swapping helpers for big-endian (Mac) values (`swapShort`, `swapLong` in `decomp/zoombinis.h`). Their inlined shape (copy to a stack temporary, reassemble bytes in reverse) only reproduces with the helper taking its parameter *by value*.
- Resource types are built Mac-style: `RESOURCE_TYPE('C','U','R','S')`, because Borland's multi-character constants have the opposite byte order.

## Where matching stops

A few functions can't match by construction: those the original wrote in assembly (see [Functions written in assembly](assembly.md)), and a few dozen near-misses whose register choice no source form reproduces. They are measured (they are simply absent from `decomp/matching.txt`) and reviewed with `uv run near-misses`.
