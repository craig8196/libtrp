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
 * @file connpeer.h
 * @author Craig Jacobson
 * @brief Peer information for connections.
 */
#ifndef _LIBTRP_CONNPEER_H_
#define _LIBTRP_CONNPEER_H_
#ifdef __cplusplus
extern "C" {
#endif


#include "connlim.h"
#include "connstat.h"


typedef struct connpeer_s
{
    /* Peer connection ID. */
    uint64_t id;

    /* Encryption. */
    unsigned char *openpk;
    unsigned char *pk;
    unsigned char *sig;
    unsigned char *nonce;

    /* Sequences
     * Starting at zero.
     * If we encounter a sequence less than what we have we discard it.
     * Currently not doing a bitset to save on space.
     * Increment each floor with each successful packet received.
     */
    uint64_t seqfloor;
    uint32_t window;

    /* Limits */
    connlim_t lim;

    /* Stats */
    connstat_t stat;
} connpeer_t;


#ifdef __cplusplus
}
#endif
#endif /* _LIBTRP_CONNPEER_H_ */


