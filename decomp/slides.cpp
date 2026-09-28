/*
 * slides (0x446bf8-0x44b550): Stone Rise (scene 12), 'Slides.MHK'
 */

#include "zoombinis.h"
#include "e2memory.h"
#include "features.h"
#include "graphics.h"
#include "module_4623b8.h"
#include "slides.h"
#include "snoids.h"
#include "sound.h"
#include "view.h"

/* Closes scene 12. */
/* @zoombi32 0x00447124 */
void closeScene12()
{
    if (g_4b1930) {
        g_4b1930 = 0;
        short saved = fn_46bee9(1);

        clearViews();
        unloadSounds();
        fn_46c602(&g_4a3fc8);
        fn_46bee9(saved);
        fn_46ca9c(&g_4b1928);
        fadeOutViews();
        fn_4624fc();
    }
}

/* The buttons' view update: redraws button 2 when g_4b1932 changes, and
   button 1 the first time. */
/* @zoombi32 0x004470b2 */
void fn_4470b2(View *, short region)
{
    if (g_4b1932) {
        if (!g_4a41e0) {
            g_4a41e0 = 1;
            unionRgnRect(region, &slidesButtons[1].rect);
        }
    } else if (g_4a41e0) {
        g_4a41e0 = 0;
        unionRgnRect(region, &slidesButtons[1].rect);
    }
    if (!g_4a41e2) {
        g_4a41e2 = 1;
        unionRgnRect(region, &slidesButtons[0].rect);
    }
}

/* Marks the Zoombini on each cell in state 508 (unknownF7). */
/* @zoombi32 0x0044943b */
void fn_44943b()
{
    short i;

    for (i = 0; i < 117; i++)
        if (g_4b1aea[i].state == 508)
            ((Snoid *)&findView(g_4b1aea[i].snoid)->body)->unknownF7 = 1;
}

/* Sets g_4b1932 if any of the cells g_4b1ab4 lists (1 to g_4b240e) is in
   state 508. */
/* @zoombi32 0x00449475 */
void fn_449475()
{
    short i;

    g_4b1932 = 0;
    for (i = 1; i <= g_4b240e; i++)
        if (g_4b1aea[g_4b1ab4[i]].state == 508) {
            g_4b1932 = 1;
            return;
        }
}

/* Counts the cells in state 502 or 508, and sums their numbers into
   g_4b1a44. */
/* @zoombi32 0x0044b261 */
short fn_44b261()
{
    short n;
    short i;

    n = g_4b1a44 = 0;
    for (i = 0; i < 117; i++)
        if (g_4b1aea[i].state == 502 || g_4b1aea[i].state == 508) {
            n++;
            g_4b1a44 += i;
        }
    return n;
}

/* Remarks on the count of cells in state 502 or 508 against the last
   (g_4b1a42): up (more than four: 8505, else 8504, and sets g_4b1932), down
   (8501 or 8500), or the same count on other cells (8502). A long remark
   already playing is stopped first. */
/* @zoombi32 0x0044b2a4 */
void fn_44b2a4()
{
    short n = fn_44b261();

    if (n > g_4b1a42) {
        g_4b1932 = 1;
        if (n - g_4b1a42 > 4) {
            if (g_4b87fe && lastViewSound == 8505) {
                stopSounds(8505, RESOURCE_TYPE(0, 'S', 'N', 'D'));
                lastViewSound = 0;
            }
            queueViewSound(8505, 0);
        } else {
            if (g_4b87fe && lastViewSound == 8504) {
                stopSounds(8504, RESOURCE_TYPE(0, 'S', 'N', 'D'));
                lastViewSound = 0;
            }
            queueViewSound(8504, 0);
        }
    } else if (n < g_4b1a42) {
        if (g_4b1a42 - n > 4) {
            if (g_4b87fe && lastViewSound == 8501) {
                stopSounds(8501, RESOURCE_TYPE(0, 'S', 'N', 'D'));
                lastViewSound = 0;
            }
            queueViewSound(8501, 0);
        } else {
            if (g_4b87fe && lastViewSound == 8500) {
                stopSounds(8500, RESOURCE_TYPE(0, 'S', 'N', 'D'));
                lastViewSound = 0;
            }
            queueViewSound(8500, 0);
        }
    } else if (g_4b1a44 != g_4b1a46) {
        queueViewSound(8502, 0);
    }
}

/* Draws button `which` (1 or 2; 2 is dim unless g_4b1932), lit or not,
   showing it on screen if `show`. */
/* @zoombi32 0x00446ffc */
void drawSlidesButton(short which, short lit, short show)
{
    short image = 0;

    switch (which) {
    case 1:
        image = 5;
        break;
    case 2:
        image = 2;
        if (!g_4b1932) {
            lit = 0;
            image = 1;
        }
        break;
    }
    if (image) {
        if (lit)
            image++;
        drawImageData((unsigned short *)(g_4a3fc4->offsets[image] + (char *)g_4a3fc4), slidesButtons[which - 1].rect.left,
                      slidesButtons[which - 1].rect.top, 8);
        if (show)
            showRect(&slidesButtons[which - 1].rect);
    }
}

/* Counts, for each feature, how many of its values the chosen Zoombinis
   (g_4b192c, g_4b2414 of them) show, into g_4b251c. */
/* @zoombi32 0x004488e8 */
void fn_4488e8()
{
    short counts[4][6];
    short i;
    short j;

    g_4b251c[0] = 0;
    g_4b251c[1] = 0;
    g_4b251c[2] = 0;
    g_4b251c[3] = 0;
    g_4b192c = listChosenSnoids();
    g_4b2414 = g_4b192c->count;
    fillMemory(counts, 0, sizeof counts);
    for (i = 0; i < g_4b2414; i++)
        for (j = 0; j < 4; j++)
            counts[j][g_4b192c->features[i][j]]++;
    for (i = 0; i < 4; i++)
        for (j = 1; j < 6; j++)
            if (counts[i][j])
                g_4b251c[i]++;
}

/* Cycles palette colours 19-21 by one (the last to the first). */
/* @zoombi32 0x00448bf5 */
void fn_448bf5()
{
    PALETTEENTRY last;
    PALETTEENTRY colors[256];
    short first;
    short count;

    first = 19;
    count = 3;
    getColors(&colors[10], 10, 236);
    last = colors[first + count - 1];
    memmove(&colors[first + 1], &colors[first], (count - 1) * sizeof(PALETTEENTRY));
    colors[first] = last;
    setColors(&colors[10], 10, 236);
}
