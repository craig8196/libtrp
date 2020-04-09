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
 * @file stream.h
 * @author Craig Jacobson
 * @brief Stream details.
 */
#ifndef _LIBTRP_STREAM_H_
#define _LIBTRP_STREAM_H_
#ifdef __cplusplus
extern "C" {
#endif


#include "core.h"


/* First options/flags are set by the user.
 * Remaining flags are set internally and used by the framework.
 */
#define _TRIPS_OPT_BACKFLOW (1 << 4)
#define _TRIPS_OPT_CLOSED   (1 << 5)
#define _TRIPS_OPT_STALLED  (1 << 6)
#define _TRIPS_OPT_PUBMASK (0x000F)
#define _TRIPS_OPT_SECMASK (0x007F)

// TODO idea for stream message lookup is to store in n x n array
// TODO n long and n max messages linked per entry grow array by one
// TODO attackers would need number that hashes perfectly and growing
// TODO will throw clustering off
struct _trip_stream_s
{
    /* Frequently Accessed */
    void *ud;
    _trip_connection_t *connection;

    int id;
    int flags;
    _trip_msg_t *listbeg;
    _trip_msg_t *listend;
};


#ifdef __cplusplus
}
#endif
#endif /* _LIBTRP_STREAM_H_ */

