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
 * @file messageq.h
 * @author Craig Jacobson
 * @brief Manage messages.
 */
#ifndef _LIBTRP_MESSAGEQ_H_
#define _LIBTRP_MESSAGEQ_H_
#ifdef __cplusplus
extern "C" {
#endif


#include "core.h"


typedef struct messageq_s
{
    /* Message Queues
     * Focus on sending one message at a time.
     * Only send partial buffers if priority and nothing else will fit.
     * Partial buffers will flush after max_message_buffer_wait ms.
     * Resending messages, or partial messages,
     * jumps to the front of the appropriate Q.
     * 0 - Priority Q.
     * 1 - Whenever Q.
     * 
     * Pull next partial message from Whenever Q when which indicator reaches zero.
     * This helps avoid starvation.
     * sendwhich & sendmask > 0 => priority next
     * sendwhich & sendmask = 0 => whenever next
     *
     * If sendmask = 0x01, then queueing is fair.
     * If sendmask = 0x03, then priority sends first 3/4 of time, whenever 1/4.
     */
    uint32_t sendmask;
    uint32_t sendwhich;
    _trip_msg_t *sendbeg[2];
    _trip_msg_t *sendend[2];
    uint32_t nextmsgid;
    int zone;
} messageq_t;


#ifdef __cplusplus
}
#endif
#endif /* _LIBTRP_MESSAGEQ_H_ */


