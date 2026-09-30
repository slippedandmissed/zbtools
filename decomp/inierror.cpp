/*
 * inierror (Mohawk engine): the settings-file error
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* The last settings-file error. */
/* @zoombi32 0x00480788 */
short iniError()
{
    return iniState.error;
}
