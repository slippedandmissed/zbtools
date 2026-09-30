/*
 * wmxmix (Mohawk engine): WaveMix's mixing loops. The originals are
 * hand-written assembly (xlat through the volume table, jo for saturation,
 * rep movs/stos); these are portable equivalents.
 *
 * Each loop reads samples from `in` (every `inStride` bytes), steps through
 * them at `step` (16.16 fixed point: output samples per input sample) and
 * writes `count` output samples (every `outStride` bytes). 8-bit input goes
 * through `table`, the volume table of 256 bytes (an 8-bit sample, offset by
 * 0x80). 16-bit output samples are stored with their bytes swapped (the
 * engine keeps them big-endian, as on the Mac); wmxWaveOut swaps them back.
 */

/* @flags -p -x- */

#include "zoombinis.h"

static signed char saturate8(int value)
{
    return (signed char)(value > 127 ? 127 : value < -128 ? -128 : value);
}

static short saturate16(long value)
{
    return (short)(value > 32767 ? 32767 : value < -32768 ? -32768 : value);
}

static unsigned short swapBytes(unsigned short value)
{
    return (unsigned short)(value << 8 | value >> 8);
}

/* Adds 8-bit samples into 8-bit output. */
/* @zoombi32-functional 0x0047fae8 */
void mixByteIntoByte(unsigned char *out, const unsigned char *in, unsigned long count,
                     long outStride, long inStride, unsigned long step, long, short,
                     const unsigned char *table)
{
    unsigned long phase = 0;

    for (;;) {
        signed char sample = (signed char)(table[*in] - 0x80);
        for (phase += step; phase >= 0x10000; phase -= 0x10000) {
            *out = (unsigned char)(saturate8((signed char)(*out - 0x80) + sample) + 0x80);
            out += outStride;
            if (!--count)
                return;
        }
        in += inStride;
    }
}

/* Adds 8-bit samples into the high bytes of 16-bit output. */
/* @zoombi32-functional 0x0047fb3b */
void mixByteIntoWord(unsigned char *out, const unsigned char *in, unsigned long count,
                     long outStride, long inStride, unsigned long step, long, short,
                     const unsigned char *table)
{
    unsigned long phase = 0;

    for (;;) {
        signed char sample = (signed char)(table[*in] - 0x80);
        for (phase += step; phase >= 0x10000; phase -= 0x10000) {
            out[1] = (unsigned char)saturate8((signed char)out[1] + sample);
            out += outStride;
            if (!--count)
                return;
        }
        in += inStride;
    }
}

/* Adds 16-bit samples into 16-bit output (the volume isn't applied). */
/* @zoombi32-functional 0x0047fb8a */
void mixWordIntoWord(unsigned char *out, const unsigned char *in, unsigned long count,
                     long outStride, long inStride, unsigned long step, long, short,
                     const unsigned char *)
{
    unsigned long phase = 0;

    for (;;) {
        short sample = *(const short *)in;
        for (phase += step; phase >= 0x10000; phase -= 0x10000) {
            short mixed = (short)swapBytes(*(unsigned short *)out);
            *(unsigned short *)out = swapBytes((unsigned short)saturate16((long)mixed + sample));
            out += outStride;
            if (!--count)
                return;
        }
        in += inStride;
    }
}

/* Copies 8-bit samples to 8-bit output: a plain copy when nothing changes. */
/* @zoombi32-functional 0x0047fbdc */
void copyByteToByte(unsigned char *out, const unsigned char *in, unsigned long count,
                    long outStride, long inStride, unsigned long step, long, short identity,
                    const unsigned char *table)
{
    unsigned long phase = 0;

    if (outStride == 1 && inStride == 1 && identity && step == 0x10000) {
        memcpy(out, in, count);
        return;
    }
    for (;;) {
        unsigned char sample = table[*in];
        for (phase += step; phase >= 0x10000; phase -= 0x10000) {
            *out = sample;
            out += outStride;
            if (!--count)
                return;
        }
        in += inStride;
    }
}

/* Copies 8-bit samples to 16-bit output. */
/* @zoombi32-functional 0x0047fc45 */
void copyByteToWord(unsigned char *out, const unsigned char *in, unsigned long count,
                    long outStride, long inStride, unsigned long step, long, short,
                    const unsigned char *table)
{
    unsigned long phase = 0;

    if (step == 0x10000) {
        do {
            *(unsigned short *)out = (unsigned short)((unsigned char)(table[*in] - 0x80) << 8);
            in += inStride;
            out += outStride;
        } while (--count);
        return;
    }
    for (;;) {
        unsigned short sample = (unsigned short)((unsigned char)(table[*in] - 0x80) << 8);
        for (phase += step; phase >= 0x10000; phase -= 0x10000) {
            *(unsigned short *)out = sample;
            out += outStride;
            if (!--count)
                return;
        }
        in += inStride;
    }
}

/* Copies 16-bit samples to 16-bit output, swapping their bytes: a plain
   copy when nothing changes. */
/* @zoombi32-functional 0x0047fcad */
void copyWordToWord(unsigned char *out, const unsigned char *in, unsigned long count,
                    long outStride, long inStride, unsigned long step, long, short identity,
                    const unsigned char *)
{
    unsigned long phase = 0;

    if (outStride == 2 && inStride == 2 && identity && step == 0x10000) {
        memcpy(out, in, count * 2);
        return;
    }
    for (;;) {
        unsigned short sample = swapBytes(*(const unsigned short *)in);
        for (phase += step; phase >= 0x10000; phase -= 0x10000) {
            *(unsigned short *)out = sample;
            out += outStride;
            if (!--count)
                return;
        }
        in += inStride;
    }
}

/* @zoombi32-functional 0x0047fd11 */
void fillBytes(void *out, unsigned char value, unsigned long count)
{
    memset(out, value, count);
}

/* @zoombi32-functional 0x0047fd3b */
void fillWords(void *out, unsigned short value, unsigned long count)
{
    unsigned short *word = (unsigned short *)out;

    while (count--)
        *word++ = value;
}
