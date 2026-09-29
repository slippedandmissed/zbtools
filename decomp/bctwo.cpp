/*
 * bctwo (0x418698-0x41a404): 'bctwo.mhk'
 */

#include "zoombinis.h"
#include "bctwo.h"
#include "view.h"

/* Resets scene 5's state. */
/* @zoombi32 0x00418698 */
void resetScene5()
{
    g_4ab650 = g_4ab652 = g_4b0d52 = 0;
    g_4ab662 = g_4ab668 = g_4ab664 = 0;
    g_4ab64a = 0;
    g_4ab666 = 1;
    g_4ab67e = 0;
}

/* @zoombi32 0x004196a8 */
long fn_4196a8(long)
{
    return 0;
}

/* A view update: redraws g_4a0adc whenever g_4ab65c changes. */
/* @zoombi32 0x00419867 */
void fn_419867(View *, short region)
{
    if (g_4ab65c) {
        if (!g_4ab65e) {
            g_4ab65e = 1;
            unionRgnRect(region, &g_4a0adc);
        }
    } else if (g_4ab65e) {
        g_4ab65e = 0;
        unionRgnRect(region, &g_4a0adc);
    }
}

/* Counts entry `n` (0-624) in: g_4ab646 of them, g_4ab648 the highest;
   g_4ab644 is then that rounded up past a multiple of 5 (50-625), and
   g_4ab642 a fifth of it; g_4ab640 stays within 5 of the end. The first
   entry keeps g_4ab640 and the count. */
/* @zoombi32 0x00419e49 */
void fn_419e49(short n)
{
    if (n >= 0 && n < 625 && g_4ab646 < 625) {
        g_4ab646++;
        if (n > g_4ab648)
            g_4ab648 = n;
    }
    g_4ab644 = (g_4ab648 + 10) / 5 * 5;
    if (g_4ab644 > 625)
        g_4ab644 = 625;
    if (g_4ab644 < 50)
        g_4ab644 = 50;
    g_4ab642 = g_4ab644 / 5;
    if (g_4ab640 > g_4ab642 - 5)
        g_4ab640 = g_4ab642 - 5;
    ((short *)g_4ab64c)[0] = g_4ab640;
    ((short *)g_4ab64c)[1] = g_4ab646;
}

/* The index of the last of 625 entries with a value, or 0. */
/* @zoombi32 0x00419f1a */
short fn_419f1a()
{
    for (short i = 0x270; i >= 0; i--)
        if (g_4ab64c[i].value)
            return i;
    return 0;
}

/* Has view g_4ab650 update at once. */
/* @zoombi32 0x0041a225 */
void fn_41a225()
{
    View *view = findView(g_4ab650);

    if (view)
        view->nextUpdate = 0;
}
