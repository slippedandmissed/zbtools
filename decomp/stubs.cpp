/*
 * Functions that do nothing, return a constant or return their argument:
 * probably placeholder callbacks. Unused parameters' types can't be recovered;
 * each is 4 bytes. Most use the game's Pascal convention; the __cdecl ones pop
 * no arguments (plain `ret`).
 */

/* @zoombi32 0x0041d9e4 */
void fn_41d9e4(long)
{
}

/* @zoombi32 0x0041d9eb */
void fn_41d9eb(long, long)
{
}

/* @zoombi32 0x0042c10b */
void fn_42c10b(long)
{
}

/* @zoombi32 0x0042c112 */
void fn_42c112(long, long)
{
}

/* @zoombi32 0x0043595f */
void fn_43595f(long)
{
}

/* @zoombi32 0x00435966 */
void fn_435966(long, long)
{
}

/* @zoombi32 0x00446962 */
void fn_446962(long, long)
{
}

/* @zoombi32 0x00455e26 */
void fn_455e26(long)
{
}

/* @zoombi32 0x00455e2d */
void fn_455e2d(long)
{
}

/* @zoombi32 0x00417906 */
long fn_417906(long)
{
    return 0;
}

/* @zoombi32 0x004196a8 */
long fn_4196a8(long)
{
    return 0;
}

/* @zoombi32 0x004320da */
long fn_4320da(long)
{
    return 0;
}

/* @zoombi32 0x0046b07b */
long fn_46b07b(long)
{
    return 0;
}

/* @zoombi32 0x0046e1f8 */
long fn_46e1f8(long value)
{
    return value;
}

/* @zoombi32 0x0046e28e */
char __cdecl fn_46e28e(char value)
{
    return value;
}

/* @zoombi32 0x0046f43a */
long __cdecl fn_46f43a(long value)
{
    return value;
}

/* @zoombi32 0x0044027b */
short fn_44027b(long, long)
{
    return 1;
}

extern char g_4aa4c9;

/* @zoombi32 0x00455013 */
long fn_455013(long, long)
{
    g_4aa4c9 = 1;
    return 0;
}

/*
 * The original adds one with `sub eax, -1`; BCC32 turns every way of writing
 * it tried so far (+ 1, - -1, enums, consts, unsigned, compound assignment,
 * locals, other -O options, and Borland C++ 4.52 as well as 4.5) into `inc eax`.
 */
/* @zoombi32-nonmatching 0x0041d3e6 */
int fn_41d3e6(long, short value)
{
    return value + 1;
}

/*
 * Only an unsigned constant (or `-=` on a local) gives the original's
 * `sub eax, 50`; a signed one becomes `add eax, -50`. Perhaps a sizeof or an
 * unsigned #define.
 */
/* @zoombi32 0x0043691d */
int fn_43691d(long, short value)
{
    return value - 50u;
}
