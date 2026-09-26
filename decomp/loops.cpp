/*
 * Small call-free functions with loops, used to pin down the compiler
 * release and options (see docs/findings.md). Names are by address until
 * their purpose is known; structures are guessed from their field offsets.
 */

extern char *g_4a4ba0;

/* @zoombi32 0x004572bf */
int fn_4572bf()
{
    int count = 0;
    for (short i = 0; i < *(short *)(g_4a4ba0 + 0xa92e); i++)
        if (*(g_4a4ba0 + 0xa93c + i * 0x13) != 0)
            count++;
    return count;
}

extern short g_4aff9a[];

/* The index (1-20) of the largest value, ignoring `exclude`. */
/* @zoombi32 0x00437390 */
short __stdcall fn_437390(short exclude)
{
    short best, bestValue, i;
    for (i = 1, best = 0, bestValue = 0; i < 0x15; i++) {
        if (g_4aff9a[i] > bestValue && exclude != i) {
            bestValue = g_4aff9a[i];
            best = i;
        }
    }
    return best;
}

/* List entry: an index at +0, a key at +4 and the next entry at +0xe. */
struct Entry
{
    short index;
    short unknown2;
    short key;
    char unknown6[8];
    Entry *next;
};

extern Entry *g_4a00a0;
extern long g_4a00dc[];

/*
 * Finds the entry with a key whose tag matches ('SND' matches any).
 * Near miss: the original loads `tag` before `key` in the prologue.
 */
/* @zoombi32-nonmatching 0x004115f5 */
Entry *__stdcall fn_4115f5(long tag, short key)
{
    Entry *entry = g_4a00a0;
    while (entry && (key != entry->key || (tag != 0x534e44 && tag != g_4a00dc[entry->index])))
        entry = entry->next;
    return entry;
}

/* Doubly-linked list node: fields at +0 and +4. */
struct Link
{
    Link *prev;
    Link *next;
};

/*
 * Splices a list in after another.
 * Near miss: the original keeps `list` in ecx and `other` in edx.
 */
/* @zoombi32-nonmatching 0x0043a772 */
void __stdcall fn_43a772(Link *list, Link *other)
{
    if (other && list) {
        Link *last = list;
        while (last->next)
            last = last->next;
        list->prev = other;
        last->next = other->next;
        other->next = list;
        last->next->prev = last;
    }
}
