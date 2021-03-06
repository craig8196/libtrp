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


#include <stddef.h>
#include <stdint.h>


#include "connpeer.h"
#include "connself.h"
#include "core.h"
#include "ping.h"
#include "streammap.h"
#include "messageq.h"


enum _trip_message_q
{
    _TRIP_MESSAGE_PRIORITY = 0,
    _TRIP_MESSAGE_WHENEVER = 1,
};

enum _tripc_state
{
    _TRIPC_STATE_START,
    _TRIPC_STATE_RESOLVE,
    _TRIPC_STATE_OPEN,
    _TRIPC_STATE_CHAL,
    _TRIPC_STATE_PING,
    _TRIPC_STATE_READY,
    _TRIPC_STATE_READY_PING,
    _TRIPC_STATE_CLOSE,
    _TRIPC_STATE_END,
    _TRIPC_STATE_ERROR,
};

#define _TRIP_SEQ_WINDOW (512)

struct _trip_connection_s
{
    /* Public and Frequently Accessed */
    void *data;
    size_t ilen;
    unsigned char *info;
    size_t rlen;
    unsigned char *route;
    _trip_router_t *router;

    /* Packet source key. */
    int src;
    int resolvekey;
    
    /* Status */
    enum trip_connection_status status;
    bool incoming;// if false, is primary pinger
    bool encrypted;

    /* State */
    enum _tripc_state state;
    uint64_t statedeadline;
    void *statetimer;
    int retry;
    int maxretry;
    int maxstatems;
    int growms;
    int statems;
    int maxresolve;

    /* Error */
    int error;
    char *errmsg;

    /* Self information. */
    connself_t self;

    /* Peer information. */
    connpeer_t peer;

    /* Ping information. */
    ping_t ping;

    /* Used to round-robin through connections when sending data.
     * Re-used to link unused connection structs.
     */
    bool insend;
    bool hassend;
    _trip_connection_t *next;

    /* Stream Map */
    streammap_t streams;

    /* Message Q */
    messageq_t msg;

    /* Sometimes we're unable to send the buffer, store here until ready.
     * segfull is true if just waiting to send.
     * segment contains the final product.
     * segwork contains unencrypted portion.
     *
     * TODO needed? I don't think so.
     */
    bool segfull;
};


void
_tripc_init(_trip_connection_t *c, _trip_router_t *r, bool incoming);
void
_tripc_destroy(_trip_connection_t *c);

void
_tripc_cancel_timeout(_trip_connection_t *c);
void
_tripc_set_error(_trip_connection_t *c, int eval, const char *msg);
void
_tripc_start(_trip_connection_t *c);
void
_tripc_resolved(_trip_connection_t *c);
void
_tripc_set_state(_trip_connection_t *c, enum _tripc_state state);
void
_tripc_send_add(_trip_connection_t *c, _trip_msg_t *m);
_trip_msg_t *
_tripc_new_message(_trip_connection_t *c);
void
_tripc_free_message(_trip_connection_t *c, _trip_msg_t *m);
void
_tripc_close_stream(_trip_connection_t *c, _trip_stream_t *s);

int
_tripc_check_seq(_trip_connection_t *c, uint64_t seq);
void
_tripc_flag_seq(_trip_connection_t *c, uint64_t seq);
void
_tripc_flag_open_seq(_trip_connection_t *c, uint32_t seq);
int
_tripc_check_open_seq(_trip_connection_t *c, uint32_t seq);
int
_tripc_read(_trip_connection_t *c, _trip_prefix_t *prefix, size_t len, const unsigned char *buf);
size_t
_tripc_send(_trip_connection_t *c, size_t len, void *buf);
void
_tripc_set_send(_trip_connection_t *c);
void
_tripc_set_screen(_trip_connection_t *c, trip_screen_t *screen);


#ifdef __cplusplus
}
#endif
#endif /* _LIBTRP_CONNECTION_H_ */

