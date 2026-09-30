/*
 * programdirectory (Mohawk engine): the program's directory
 */

/* @flags -p -x- */

#include "zoombinis.h"

/* @zoombi32 0x00484678 */
void programDirectory(fileSpec *directory)
{
    *directory = files.appDirectory;
}
