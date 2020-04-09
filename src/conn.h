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
 * @file conn.h
 * @author Craig Jacobson
 * @brief Connection definitions.
 */
#ifndef _LIBTRP_CONNECTION_H_
#define _LIBTRP_CONNECTION_H_
#ifdef __cplusplus
extern "C" {
#endif


#include "core.h"


enum _trip_message_q
{
    _TRIP_MESSAGE_PRIORITY = 0,
    _TRIP_MESSAGE_WHENEVER = 1,
};

enum _tripc_state
{
    _TRIPC_STATE_START,
    _TRIPC_STATE_OPEN,
    _TRIPC_STATE_CHAL,
    _TRIPC_STATE_PING,
    _TRIPC_STATE_READY,
    _TRIPC_STATE_READY_PING,
    _TRIPC_STATE_NOTIFY,
    _TRIPC_STATE_DISCONNECT,
    _TRIPC_STATE_END,
    _TRIPC_STATE_ERROR,
};

#define _TRIP_SEQ_WINDOW (512)

struct _trip_connection_s
{
    /* Frequently Accessed */
    void *ud;
    _trip_router_t *router;

    /* Address */
    size_t infolen;
    unsigned char *info;

    /* Error */
    int error;
    char *msg;

    uint32_t id;
    /* Used to round-robin through connections when sending data.
     * Re-used to link unused connection structs.
     */
    _trip_connection_t *next;

    /* State */
    enum _tripc_state state;
    uint64_t statedeadline;


    /* Sometimes we're unable to send the buffer, store here until ready.
     * segfull is true if just waiting to send.
     * segment contains the final product.
     * segwork contains unencrypted portion.
     */
    bool segfull;
    _tripbuf_t *segment;
    _tripbuf_t *segwork;

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

    /* Sequences
     * Starting at zero.
     * If we encounter a sequence less than what we have we discard it.
     * Currently not doing a bitset to save on space.
     * Increment each floor with each successful packet received.
     */
    uint32_t seqfloor;
    uint32_t seqlatest;
};


#ifdef __cplusplus
}
#endif
#endif /* _LIBTRP_CONNECTION_H_ */
