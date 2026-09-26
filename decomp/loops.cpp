/*
 * Small call-free functions with loops, used to pin down the compiler
 * release and options (see docs/findings.md). Functions are named by address
 * until their purpose is known; structures are guessed from their field offsets.
 *
 * The game uses the Pascal calling convention (-p): arguments are pushed left
 * to right, so the first parameter is the one furthest from the stack frame.
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
short indexOfLargestExcept(short exclude)
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

/* Finds the entry with a key whose tag matches ('SND' matches any). */
/* @zoombi32 0x004115f5 */
Entry *fn_4115f5(short key, long tag)
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

/* Splices a list in after another. */
/* @zoombi32 0x0043a772 */
void spliceList(Link *other, Link *list)
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

/* @zoombi32 0x00437acb */
short fn_437acb(short i)
{
    return g_4aff9a[i];
}

extern short g_4aa79a;
extern short g_4aa79c;

/* How many entries are queued in a 32-entry ring buffer (g_4aa79a is where
   reading starts, g_4aa79c where writing does). */
/* @zoombi32 0x00413dc0 */
short fn_413dc0()
{
    short count = g_4aa79c - g_4aa79a;
    if (count < 0)
        count += 32;
    return count;
}

/* Advances an index into the 32-entry ring buffer, wrapping to 0. */
/* @zoombi32 0x0041416f */
void __cdecl nextRingIndex(short *index)
{
    if (++*index > 31)
        *index = 0;
}

/* How many of g_4aff9a[1..20] are non-zero. */
/* @zoombi32 0x004381bb */
short fn_4381bb()
{
    short i, count;
    for (i = 1, count = 0; i < 0x15; i++)
        if (g_4aff9a[i])
            count++;
    return count;
}
