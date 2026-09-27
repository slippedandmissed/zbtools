/*
 * module_48e0a0 (Mohawk engine): decompressing images; setPortError
 */

/* @flags -p -x- */

#include <malloc.h>
#include <string.h>
#include "zoombinis.h"

/*
 * Decompresses an image resource in its handle, in place: Mohawk's LZ, or a
 * decompressor DLL's format. Byte-swaps 16-bit pixels once, too.
 *
 * Functional: the code is the same as the original's, but its module holds
 * assembly, so it went through Turbo Assembler, which encodes `and ax, 0xf`
 * and `cmp ax, n` in their accumulator forms; BCC32 uses the short
 * sign-extended ones.
 */
/* @zoombi32-functional 0x0048e0a0 */
short decompressImage(short handle)
{
    short newHandle_;
    unsigned long size;
    ImageHeader *out;
    unsigned short window;
    short bits;
    unsigned char *ring;
    ExternalImage *external;
    short depth;
    ImageHeader *header;
    unsigned short flags;
    LzImage *lz;
    Decompressor *decompressor;
    short error;
    unsigned short kind;
    unsigned long count;

    if ((header = (ImageHeader *)handleData(handle)) == 0)
        return setPortError(memError());
    flags = fn_492730(header->flags);
    if (flags & 0xf00) {
        switch (fn_492730(header->flags) & 0xf00) {
        case 0x100:
            lz = (LzImage *)(header + 1);
            switch (window = fn_492730(lz->window)) {
            case 0x100:
                bits = 8;
                break;
            case 0x200:
                bits = 9;
                break;
            case 0x400:
                bits = 10;
                break;
            case 0x800:
                bits = 11;
                break;
            case 0x1000:
                bits = 12;
                break;
            default:
                return setPortError(0x2a63);
            }
            if ((ring = (unsigned char *)alloca(window)) == 0)
                return setPortError(0x2a37);
            lockHandle(handle);
            size = fn_4926ff(lz->size);
            newHandle_ = newHandle(size + 8);
            unlockHandle(handle);
            if (!newHandle_)
                return setPortError(memError());
            out = (ImageHeader *)handleData(newHandle_);
            memcpy(out, header, 8);
            out->flags &= ~fn_492730(0xf00);
            lzDecompress((unsigned char *)(out + 1), lz->data, size, ring, bits);
            error = fn_48f4bc(handle, newHandle_);
            disposeHandle(newHandle_);
            if (error)
                return setPortError(error);
            header = out;
            break;
        case 0xf00:
            external = (ExternalImage *)(header + 1);
            for (decompressor = graphics.decompressors;; decompressor = decompressor->next) {
                if (!decompressor)
                    return setPortError(0x2a63);
                if (!memicmp(external->name, decompressor->name, 8))
                    break;
            }
            lockHandle(handle);
            size = fn_4926ff(external->size);
            newHandle_ = newHandle(size + 8);
            if (!newHandle_) {
                setPortError(memError());
                unlockHandle(handle);
                return graphics.error;
            }
            out = (ImageHeader *)lockHandle(newHandle_);
            memcpy(out, header, 8);
            out->flags &= ~fn_492730(0xf00);
            kind = fn_492730(header->flags) & 0xf;
            depth = !kind        ? 1
                    : kind == 1 ? 4
                    : kind == 2 ? 8
                    : kind == 3 ? 16
                    : kind == 4 ? 24
                                : 0;
            if (!depth
                || ((DecompressProc)decompressor->proc)(
                    out + 1, size, fn_492730(header->width), fn_492730(header->height), depth,
                    (unsigned short)fn_492730(header->rowBytes), external->params,
                    fn_4926ff(external->paramSize), 0xffff))
                error = 0x2a63;
            else
                error = 0;
            unlockHandle(newHandle_);
            unlockHandle(handle);
            if (!error)
                error = fn_48f4bc(handle, newHandle_);
            disposeHandle(newHandle_);
            if (error)
                return setPortError(error);
            header = out;
            break;
        default:
            return setPortError(0x2a63);
        }
        flags &= 0xf0ff;
        header->flags = fn_492730(flags);
    }
    if ((flags & 0xf) == 3 && !(flags & 0xf0) && !(flags & 0x1000)) {
        count = (unsigned short)fn_492730(header->height)
                * ((short)fn_492730(header->rowBytes) >= 0 ? (short)fn_492730(header->rowBytes)
                                                          : -(short)fn_492730(header->rowBytes));
        swapWords(header + 1, count / 2);
        header->flags = fn_492730(flags | 0x1000);
    }
    return setPortError(0);
}

/*
 * Byte-swaps `count` 16-bit words in place.
 *
 * The original is in assembly (`xchg ah, al` and `loop`); this does the same
 * in C++.
 */
/* @zoombi32-functional 0x0048e4c7 */
void swapWords(void *data, unsigned long count)
{
    unsigned char *p;
    unsigned char byte;

    p = (unsigned char *)data;
    do {
        byte = p[0];
        p[0] = p[1];
        p[1] = byte;
        p += 2;
    } while (--count);
}

/*
 * Mohawk's LZ decompression (LZSS): flag bytes, low bit first, each 1 a
 * literal byte and each 0 a big-endian word, its low `bits` bits a position
 * in a ring buffer of 1 << `bits` bytes and the rest a length less 3. The
 * ring starts zeroed, with writing (1 << (16 - bits)) + 2 bytes from its end.
 *
 * The original is in assembly (with a fast path for a flag byte of 0xff:
 * eight literals); this does the same in C++.
 */
/* @zoombi32-functional 0x0048e4df */
void lzDecompress(unsigned char *dest, const unsigned char *source, unsigned long size,
                  unsigned char *ring, short bits)
{
    unsigned short ringSize;
    unsigned short mask;
    unsigned short position;
    unsigned short flags;
    unsigned short word;
    unsigned short from;
    unsigned char length;
    unsigned char byte;

    ringSize = (unsigned short)(1 << bits);
    mask = ringSize - 1;
    position = ringSize - ((1 << (16 - bits)) + 2);
    flags = 0;
    memset(ring, 0, ringSize);
    while (size) {
        if (!(flags & 0xff00))
            flags = *source++ | 0xff00;
        if (flags & 1) {
            flags >>= 1;
            byte = *source++;
            *dest++ = byte;
            ring[position & mask] = byte;
            position = (position & mask) + 1;
            size--;
        } else {
            flags >>= 1;
            word = (unsigned short)(source[0] << 8 | source[1]);
            source += 2;
            length = (unsigned char)((word >> 8 >> (bits - 8)) + 3);
            from = word;
            do {
                byte = ring[from & mask];
                from = (from & mask) + 1;
                ring[position & mask] = byte;
                position = (position & mask) + 1;
                *dest++ = byte;
                size--;
            } while (size && --length);
        }
    }
}

/* @zoombi32 0x0048e5d9 */
short setPortError(short error)
{
    return graphics.error = error;
}
