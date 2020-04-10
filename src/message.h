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
 * @file message.h
 * @author Craig Jacobson
 * @brief Message definition.
 */
#ifndef _LIBTRP_MESSAGE_H_
#define _LIBTRP_MESSAGE_H_
#ifdef __cplusplus
extern "C" {
#endif


#include "core.h"


struct _trip_msg_s
{
    _trip_stream_t *stream;

    /* Streams are stored in a list.
     */
    _trip_msg_t *next;

    int zone;// 0 or 1, indicates which segment zone this message falls
    uint32_t id;// id or index in zone
    int priority;// 0 high 1 low
    size_t boff; // amount sent
    size_t len; // length, don't change
    unsigned char *buf; // data, don't change
    _trip_msg_t *sendnext; // next message in the queue
};


#ifdef __cplusplus
}
#endif
#endif /* _LIBTRP_MESSAGE_H_ */

