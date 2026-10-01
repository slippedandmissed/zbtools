# Near-misses

A function that compiles to *almost* the original's bytes is a near-miss. Of about 2,100 functions, 1,915 match exactly, 176 are near-misses (nearly always only register allocation), and 37 are written portably in place of the original's assembly.

`uv run near-misses [files…]` sorts them by comparing what the two versions compute: operations, branch conditions, calls and constants, counted and normalised so equivalent forms look alike, plus which stack slots have their addresses taken.

| Class | Meaning | Action |
| --- | --- | --- |
| *allocation only* | the same computations; different registers or frame slots | usually fine |
| *frame layout* | a local whose address is taken sits elsewhere | check sizes: a too-small buffer would overrun |
| *needs a look* | the tool lists what each version computes that the other doesn't | review by hand |

Most "needs a look" entries are equivalent forms the tool doesn't recognise (16-bit arithmetic, `lea` for `add`, a view's body computed in place at `+0x30`, offsets from a cached base). The review pays off in real bugs: a decompiled function that sets `midi->looping` where the original sets `audioObj::looping`, a 1-byte local where the original's DOS IOCTL block was 2 bytes (which overwrote the saved frame pointer in the rebuilt game), and `switch` bodies swapped between cases all showed up as near-misses first. So review any near-miss with more than allocation differences before trusting it.

The reason for matching at all is the same: a function that matches byte for byte cannot hide a mistake.
