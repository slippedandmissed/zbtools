/*
 * First functions matched against zoombi32.exe, to validate the toolchain.
 * Functions whose purpose isn't known yet are named after their address.
 */

extern long g_4a7f58;

/* @zoombi32 0x0046be2e */
void fn_46be2e(long value)
{
    g_4a7f58 = value;
}

/* @zoombi32 0x00455e85 */
long fn_455e85(long, long)
{
    return 0;
}
