/*
 * module_484994 (Mohawk engine): the temporary directory
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x00484994 */
void tempDirectory(fileSpec *directory)
{
    *directory = files.tempDirectory;
}
