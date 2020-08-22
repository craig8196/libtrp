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
 * @file pack.h
 * @author Craig Jacobson
 * @brief Packing and unpacking utilities.
 *
 * Pack/Unpack Format Spec
 * =======================
 * NOTE THAT THESE TURN TO DECRYPT ON UNPACK!
 * NOTE THAT LEAVING A POINTER AS NULL DISABLES ENCRYPTION/SIGNATURES/NONCE!
 *   o: Encrypt open start.
 *   O: Encrypt open end.
 *   e: Encrypt start.
 *   E: Encrypt end.
 *   s: Signature start.
 *   S: Signature end.
 *   n: Nonce.
 *   k: Public key (box/seal).
 *
 *   b: Binary string (size_t, void *).
 *   c: Signed octet.
 *   C: Unsigned octet.
 *   h: Signed 16-bit int.
 *   H: Unsigned 16-bit int.
 *   i: Signed 32-bit int.
 *   I: Unsigned 32-bit int.
 *   q: Signed 64-bit int.
 *   Q: Unsigned 64-bit int.
 *
 *   p: Pointer save. Unpack only.
 *   V: Unsigned variable int from 32-bit on pack. To 32-bit on unpack.
 *      First byte indicates number of following bytes.
 *   W: Unsigned variable int from 64-bit on pack. To 64-bit on unpack.
 *      First byte indicates number of following bytes.
 */
#ifndef _LIBTRP_PACK_H_
#define _LIBTRP_PACK_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stddef.h>


size_t
trip_pack_len(const char *format);

size_t
trip_pack(size_t cap, unsigned char *buf, const char *format, ...);

size_t
trip_unpack(size_t blen, const unsigned char *buf, const char *format, ...);

void
trip_dump(size_t blen, const unsigned char *buf);


#ifdef __cplusplus
}
#endif
#endif /* _LIBTRP_PACK_H_ */

