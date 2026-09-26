/*
 * Functions that just return or set a global. Named after their address until
 * their purpose is known.
 */

extern long g_4b9d00;
extern long g_4b9d04;
extern long g_4b9d08;
extern short g_4af35a;
extern short g_4b2d38;
extern short g_4b99d4;
extern short g_4b9cf0;
extern short g_4b9cf8;
extern short g_4b9d4c;
extern short g_4ab49e;
extern short g_4a7b94;

/* @zoombi32 0x0046dd21 */
long fn_46dd21()
{
    return g_4b9d00;
}

/* @zoombi32 0x0046dd27 */
long fn_46dd27()
{
    return g_4b9d04;
}

/* @zoombi32 0x0046dd2d */
long fn_46dd2d()
{
    return g_4b9d08;
}

/* @zoombi32 0x0042e693 */
short fn_42e693()
{
    return g_4af35a;
}

/* @zoombi32 0x00456bf6 */
short fn_456bf6()
{
    return g_4b2d38;
}

/* @zoombi32 0x0046bee2 */
short fn_46bee2()
{
    return g_4b99d4;
}

/* @zoombi32 0x0046d9c8 */
short fn_46d9c8()
{
    return g_4b9cf0;
}

/* @zoombi32 0x0046dff0 */
short fn_46dff0()
{
    return g_4b9cf8;
}

/* @zoombi32 0x0046e5ed */
short fn_46e5ed()
{
    return g_4b9d4c;
}

/* @zoombi32 0x00415811 */
void fn_415811()
{
    g_4ab49e = 1;
}

/* @zoombi32 0x00465175 */
void fn_465175()
{
    g_4a7b94 = 1;
}
