/*******************************************************************************
 * Copyright (c) 2019 Craig Jacobson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ******************************************************************************/
/**
 * @file pack.c
 * @author Craig Jacobson
 * @brief Specify methods for encoding and decoding buffers.
 * @see https://beej.us/guide/bgnet/html/#serialization
 * @see https://beej.us/guide/bgnet/examples/pack2.c
 */
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "crypto.h"
#include "libtrp.h"
#include "util.h"

#define TRIPPACK_MAX_VAR (sizeof(uint32_t))
#define TRIPPACK_MAX_DVAR (sizeof(uint64_t))


/**
 * Store 16-bit int into buffer.
 */ 
static void
packi16(unsigned char *buf, unsigned int i)
{
    *buf++ = i >> 8;
    *buf = i;
}

/**
 * Store 32-bit int into buffer.
 */
static void
packi32(unsigned char *buf, unsigned long int i)
{
    *buf++ = i >> 24;
    *buf++ = i >> 16;
    *buf++ = i >> 8;
    *buf = i;
}

/**
 * Store 64-bit int into buffer.
 */ 
static void
packi64(unsigned char *buf, unsigned long long int i)
{
    *buf++ = i >> 56;
    *buf++ = i >> 48;
    *buf++ = i >> 40;
    *buf++ = i >> 32;
    *buf++ = i >> 24;
    *buf++ = i >> 16;
    *buf++ = i >> 8;
    *buf = i;
}

/**
 * @return Unpack signed 16-bit int.
 */
static int
unpacki16(unsigned char *buf)
{
    unsigned int i2 = ((unsigned int)buf[0] << 8) | buf[1];
    int i;

    /* Restore sign. */
    if (i2 <= 0x7fffu)
    {
        i = i2;
    }
    else
    {
        i = -1 - (unsigned int)(0xffffu - i2);
    }

    return i;
}

/**
 * @return Unpack unsigned 16-bit int.
 */
static unsigned int
unpacku16(unsigned char *buf)
{
    return ((unsigned int)buf[0] << 8) | buf[1];
}

/**
 * @return Unpack signed 32-bit int.
 */
static long int
unpacki32(unsigned char *buf)
{
    unsigned long int i2 = ((unsigned long int)buf[0] << 24) |
                           ((unsigned long int)buf[1] << 16) |
                           ((unsigned long int)buf[2] << 8)  |
                           buf[3];
    long int i;

    /* Restore sign. */
    if (i2 <= 0x7fffffffu)
    {
        i = i2;
    }
    else
    {
        i = -1 - (long int)(0xffffffffu - i2);
    }

    return i;
}

/**
 * @return Unpack unsigned 32-bit int.
 */
static unsigned long int
unpacku32(unsigned char *buf)
{
    return ((unsigned long int)buf[0] << 24) |
           ((unsigned long int)buf[1] << 16) |
           ((unsigned long int)buf[2] << 8)  |
           buf[3];
}

/**
 * @return Unpack signed 64-bit int.
 */
static long long int
unpacki64(unsigned char *buf)
{
    unsigned long long int i2 = ((unsigned long long int)buf[0] << 56) |
                                ((unsigned long long int)buf[1] << 48) |
                                ((unsigned long long int)buf[2] << 40) |
                                ((unsigned long long int)buf[3] << 32) |
                                ((unsigned long long int)buf[4] << 24) |
                                ((unsigned long long int)buf[5] << 16) |
                                ((unsigned long long int)buf[6] << 8)  |
                                buf[7];
    long long int i;

    /* Restore sign. */
    if (i2 <= 0x7fffffffffffffffu)
    {
        i = i2;
    }
    else
    {
        i = -1 -(long long int)(0xffffffffffffffffu - i2);
    }

    return i;
}

/**
 * @return Unpack signed 64-bit int.
 */
static unsigned long long int
unpacku64(unsigned char *buf)
{
    return ((unsigned long long int)buf[0] << 56) |
           ((unsigned long long int)buf[1] << 48) |
           ((unsigned long long int)buf[2] << 40) |
           ((unsigned long long int)buf[3] << 32) |
           ((unsigned long long int)buf[4] << 24) |
           ((unsigned long long int)buf[5] << 16) |
           ((unsigned long long int)buf[6] << 8)  |
           buf[7];
}



/**
 * See format specifiers in trip_pack documentation.
 * @return Max length of data.
 */
size_t
trip_pack_len(const char *format)
{
    unsigned int len = 0;

    for (; *format != '\0'; ++format)
    {
        switch (*format)
        {
            case 'o':
                len += crypto_box_SEALBYTES;
                break;

            case 'e':
                len += crypto_box_MACBYTES;
                break;

            case 's':
                len += crypto_sign_BYTES;
                break;

            case 'n':
                len += crypto_box_NONCEBYTES;
                break;

            case 'k':
                len += crypto_box_PUBLICKEYBYTES;
                break;

            case 'c':
            case 'C':
                ++len;
                break;

            case 'h':
            case 'H':
                len += 2;
                break;

            case 'i':
            case 'I':
                len += 4;
                break;

            case 'q':
            case 'Q':
                len += 8;
                break;

            case 'V':
                /* Max supported by platform implementation. */
                len += 1 + TRIPPACK_MAX_VAR;
                break;

            case 'W':
                /* Max supported by platform implementation. */
                len += 1 + TRIPPACK_MAX_DVAR;
                break;

            default:
                break;
        }
    }

    return len;
}

static int
_trip_uvar_len(uint64_t n)
{
    // TODO this could be attack vector, limit loop
    int len = 0;

    while (n)
    {
        ++len;
        n = n >> 8;
    }

    return len;
}

/**
 * @return Length on success; -1 on error.
 */
size_t
trip_pack(size_t cap, unsigned char *buf, const char *format, ...)
{
	va_list ap;

    /* 8-bit */
	signed char c;
	unsigned char C;

    /* 16-bit */
	int h;
	unsigned int H;

    /* 32-bit */
	long int i;
	unsigned long int I;

    /* 64-bit */
	long long int q;
	unsigned long long int Q;

    /* Buffers */
    size_t tlen;
    size_t rlen;
    unsigned char *raw = NULL;

    /* Encryption/Signatures */
    unsigned char *sigstart = buf;
    unsigned char *cryptstart = buf;
    unsigned char *sigkey = buf;
    unsigned char *key = NULL;
    unsigned char *nonce = NULL;

    /* Start Packing */
	size_t size = 0;

	va_start(ap, format);

	for(; *format != '\0'; format++)
    {
		switch (*format)
        {
            case 'o':
                size += crypto_box_SEALBYTES;
                if (size > cap)
                {
                    size = NPOS;
                    goto _trip_pack_end;
                }
                cryptstart = buf;
                key = va_arg(ap, unsigned char *);
                buf += crypto_box_SEALBYTES;
                break;

            case 'O':
                if (key)
                {
                    /* TODO so I don't see documentation allowing
                     * the input/output buffer to overlap for
                     * sealed boxes....
                     * TODO use malloc as shim
                     * TODO maybe switch to some kind of custom scheme?
                     */
                    memset(cryptstart, 0, crypto_box_SEALBYTES);
                }
                else
                {
                    memset(cryptstart, 0, crypto_box_SEALBYTES);
                }
                break;

            case 'e':
                cryptstart = buf;
                break;

            case 'E':
                break;

            case 's':
                sigstart = buf;
                break;

            case 'S':
                size += crypto_sign_BYTES;
                if (size > cap)
                {
                    size = NPOS;
                    goto _trip_pack_end;
                }

                sigkey = va_arg(ap, unsigned char *);
                if (sigkey)
                {
                    size_t siglen = (unsigned char *)buf - sigstart;
                    crypto_sign(buf, NULL, sigstart, siglen, sigkey);
                }
                else
                {
                    memset(buf, 0, crypto_sign_BYTES);
                }
                buf += crypto_sign_BYTES;
                break;

            case 'n':
                size += crypto_box_NONCEBYTES;
                if (size > cap)
                {
                    size = NPOS;
                    goto _trip_pack_end;
                }

                nonce = va_arg(ap, unsigned char *);
                if (nonce)
                {
                    memcpy(buf, nonce, crypto_box_NONCEBYTES);
                }
                else
                {
                    memset(buf, 0, crypto_box_NONCEBYTES);
                }
                buf += crypto_box_NONCEBYTES;
                break;

            case 'k':
                size += crypto_box_PUBLICKEYBYTES;
                if (size > cap)
                {
                    size = NPOS;
                    goto _trip_pack_end;
                }

                key = va_arg(ap, unsigned char *);
                if (key)
                {
                    memcpy(buf, key, crypto_box_PUBLICKEYBYTES);
                }
                else
                {
                    memset(buf, 0, crypto_box_PUBLICKEYBYTES);
                }
                buf += crypto_box_PUBLICKEYBYTES;
                break;

            case 'b':
                rlen = va_arg(ap, size_t);
                raw = va_arg(ap, unsigned char *);
                i = _trip_uvar_len(rlen);
                size += i + 1 + rlen;
                if (size > cap)
                {
                    size = NPOS;
                    goto _trip_pack_end;
                }
                *buf = i;
                ++buf;
                tlen = rlen;
                for (; i; --i, tlen = tlen >> 8, ++buf)
                {
                    *buf = ((size_t)0x000000FF & tlen);
                }
                if (rlen)
                {
                    memcpy(buf, raw, rlen);
                    buf += rlen;
                }
                break;

            case 'c':
                size += 1;
                if (size > cap)
                {
                    size = NPOS;
                    goto _trip_pack_end;
                }
                c = (signed char)va_arg(ap, int); // promoted
                *buf++ = c;
                break;

            case 'C':
                size += 1;
                if (size > cap)
                {
                    size = NPOS;
                    goto _trip_pack_end;
                }
                C = (unsigned char)va_arg(ap, unsigned int); // promoted
                *buf++ = C;
                break;

            case 'h':
                size += 2;
                if (size > cap)
                {
                    size = NPOS;
                    goto _trip_pack_end;
                }
                h = va_arg(ap, int);
                packi16(buf, h);
                buf += 2;
                break;

            case 'H':
                size += 2;
                if (size > cap)
                {
                    size = NPOS;
                    goto _trip_pack_end;
                }
                H = va_arg(ap, unsigned int);
                packi16(buf, H);
                buf += 2;
                break;

            case 'i':
                size += 4;
                if (size > cap)
                {
                    size = NPOS;
                    goto _trip_pack_end;
                }
                i = va_arg(ap, long int);
                packi32(buf, i);
                buf += 4;
                break;

            case 'I':
                size += 4;
                if (size > cap)
                {
                    size = NPOS;
                    goto _trip_pack_end;
                }
                I = va_arg(ap, unsigned long int);
                packi32(buf, I);
                buf += 4;
                break;

            case 'q':
                size += 8;
                if (size > cap)
                {
                    size = NPOS;
                    goto _trip_pack_end;
                }
                q = va_arg(ap, long long int);
                packi64(buf, q);
                buf += 8;
                break;

            case 'Q':
                size += 8;
                if (size > cap)
                {
                    size = NPOS;
                    goto _trip_pack_end;
                }
                Q = va_arg(ap, unsigned long long int);
                packi64(buf, Q);
                buf += 8;
                break;

            case 'p':
                size = NPOS;
                goto _trip_pack_end;
                break;

            case 'V':
                I = va_arg(ap, unsigned long int);
                i = _trip_uvar_len(I);
                size += i + 1;
                if (size > cap)
                {
                    size = NPOS;
                    goto _trip_pack_end;
                }
                *buf = i;
                ++buf;
                for (; i; --i, I = I >> 8, ++buf)
                {
                    *buf = (0x000000FF & I);
                }
                break;

            case 'W':
                Q = va_arg(ap, unsigned long long int);
                i = _trip_uvar_len(Q);
                size += i + 1;
                if (size > cap)
                {
                    size = NPOS;
                    goto _trip_pack_end;
                }
                *buf = i;
                ++buf;
                for (; i; --i, Q = Q >> 8, ++buf)
                {
                    *buf = (0x00000000000000FF & Q);
                }
                break;

            default:
                size = NPOS;
                goto _trip_pack_end;
                break;
		}
	}

    _trip_pack_end:
	va_end(ap);

	return size;
}

/**
 * See documentation in trip_pack.
 * @return Zero on success; errno otherwise.
 */
size_t
trip_unpack(size_t blen, unsigned char *buf, const char *format, ...)
{
	va_list ap;

    /* 8-bit */
	signed char *c;
	unsigned char *C;

    /* 16-bit */
	int *h;
	unsigned int *H;

    /* 32-bit */
	long int *i;
	unsigned long int *I;

    /* 64-bit */
	long long int *q;
	unsigned long long int *Q;

    /* Buffers */
    unsigned char *raw;
    unsigned char **pointer;

	size_t len = 0;

	va_start(ap, format);

	for(; *format != '\0'; format++)
    {
		switch (*format)
        {
            case 'c':
                len++;
                if (len > blen)
                {
                    len = NPOS;
                    goto _trip_unpack_end;
                }
                c = va_arg(ap, signed char*);
                *c = *((char *)buf);
                buf++;
                break;

            case 'C':
                len++;
                if (len > blen)
                {
                    len = NPOS;
                    goto _trip_unpack_end;
                }
                C = va_arg(ap, unsigned char*);
                *C = *buf;
                buf++;
                len++;
                break;

            case 'h':
                len += 2;
                if (len > blen)
                {
                    len = NPOS;
                    goto _trip_unpack_end;
                }
                h = va_arg(ap, int*);
                *h = unpacki16(buf);
                buf += 2;
                break;

            case 'H':
                len += 2;
                if (len > blen)
                {
                    len = NPOS;
                    goto _trip_unpack_end;
                }
                H = va_arg(ap, unsigned int*);
                *H = unpacku16(buf);
                buf += 2;
                break;

            case 'i':
                len += 4;
                if (len > blen)
                {
                    len = NPOS;
                    goto _trip_unpack_end;
                }
                i = va_arg(ap, long int*);
                *i = unpacki32(buf);
                buf += 4;
                break;

            case 'I':
                len += 4;
                if (len > blen)
                {
                    len = NPOS;
                    goto _trip_unpack_end;
                }
                I = va_arg(ap, unsigned long int*);
                *I = unpacku32(buf);
                buf += 4;
                break;

            case 'q':
                len += 8;
                if (len > blen)
                {
                    len = NPOS;
                    goto _trip_unpack_end;
                }
                q = va_arg(ap, long long int*);
                *q = unpacki64(buf);
                buf += 8;
                break;

            case 'Q':
                len += 8;
                if (len > blen)
                {
                    len = NPOS;
                    goto _trip_unpack_end;
                }
                Q = va_arg(ap, unsigned long long int*);
                *Q = unpacku64(buf);
                buf += 8;
                break;

            case 'p':
                pointer = va_arg(ap, unsigned char **);
                *pointer = buf;
                break;

            case 's':
                I = va_arg(ap, unsigned long int *);
                len += *I;
                if (len > blen)
                {
                    len = NPOS;
                    goto _trip_unpack_end;
                }
                buf += *I;
                break;

            case 'V':
                {
                    if (len + 1 > blen)
                    {
                        len = NPOS;
                        goto _trip_unpack_end;
                    }
                    unsigned char l = *buf;
                    len += 1 + l;
                    if (l > 4 || len > blen)
                    {
                        len = NPOS;
                        goto _trip_unpack_end;
                    }
                    ++buf;
                    int index = 0;
                    uint32_t n = 0;
                    for (; index < l; ++index, ++buf)
                    {
                        n = n << 8;
                        n ^= (unsigned long int)*buf;
                    }
                    I = va_arg(ap, unsigned long int *);
                    *I = n;
                }
                break;

            case 'n':
                len += _TRIP_NONCE;
                if (len > blen)
                {
                    len = NPOS;
                    goto _trip_unpack_end;
                }
                raw = va_arg(ap, unsigned char *);
                memcpy(raw, buf, _TRIP_NONCE);
                buf += _TRIP_NONCE;
                break;

            case 'k':
                len += TRIP_KEY_PUB;
                if (len > blen)
                {
                    len = NPOS;
                    goto _trip_unpack_end;
                }
                raw = va_arg(ap, unsigned char *);
                memcpy(raw, buf, TRIP_KEY_PUB);
                buf += TRIP_KEY_PUB;
                break;

            default:
                len = NPOS;
                goto _trip_unpack_end;
                break;
		}
	}

    _trip_unpack_end:
	va_end(ap);

    return len;
}

void
trip_dump(size_t blen, const unsigned char *buf)
{
    size_t i;
    for (i = 0; i < blen; ++i)
    {
        if (i)
        {
            printf(".%x", buf[i]);
        }
        else
        {
            printf("%x", buf[i]);
        }
    }
}

