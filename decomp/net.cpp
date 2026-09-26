/*
 * net (0x439560-0x4402c0): 'Net.MHK'
 */

#include "zoombinis.h"

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

/* @zoombi32 0x0044027b */
short fn_44027b(long, long)
{
    return 1;
}
