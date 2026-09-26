/*
 * First functions matched against zoombi32.exe, to validate the toolchain.
 * Functions whose purpose isn't known yet are named after their address.
 */

extern long g_4a7f58;

/* @zoombi32 0x0046be2e */
void __stdcall fn_46be2e(long value)
{
    g_4a7f58 = value;
}

/* @zoombi32 0x00455e85 */
long __stdcall fn_455e85(long a, long b)
{
    return 0;
}
