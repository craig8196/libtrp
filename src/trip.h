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
 * @file trip.h
 * @author Craig Jacobson
 * @brief Router internals and core interface implementation to library.
 *
 * TODO change zones to be called rivers
 */
#ifndef _LIBTRP_ROUTER_H_
#define _LIBTRP_ROUTER_H_
#ifdef __cplusplus
extern "C" {
#endif


#include "core.h"
#include "connmap.h"
#include "resolveq.h"
#include "sendq.h"
#include "sockmap.h"
#include "trip_poll.h"
#include "timerwheel.h"



typedef struct _trip_router_s _trip_router_t;
typedef struct _trip_connection_s _trip_connection_t;
typedef struct _trip_stream_s _trip_stream_t;
typedef struct _trip_msg_s _trip_msg_t;
typedef struct _trip_prefix_s _trip_prefix_t;

enum _tripr_state
{
    _TRIPR_STATE_START, /* Don't confuse start action with START state. */
    _TRIPR_STATE_BIND,
    _TRIPR_STATE_LISTEN,
    _TRIPR_STATE_CLOSE,
    _TRIPR_STATE_UNBIND,
    _TRIPR_STATE_END,
    _TRIPR_STATE_ERROR,
};

#define _TRIPR_FLAG_ALLOW_IN           (1 << 0)
#define _TRIPR_FLAG_ALLOW_OUT          (1 << 1)
#define _TRIPR_FLAG_ALLOW_PLAIN_OPEN   (1 << 2)
#define _TRIPR_FLAG_ALLOW_PLAIN_ISIG   (1 << 3)
#define _TRIPR_FLAG_ALLOW_PLAIN_OSIG   (1 << 4)
#define _TRIPR_FLAG_ALLOW_PLAIN_COMM   (1 << 5)
#define _TRIPR_FLAG_FREE_PACKET        (1 << 6)
#define _TRIPR_FLAG_ALWAYS_READY       (1 << 7)

#define _TRIPR_DEFAULT_MAX_CONN (1 << 19)
#define _TRIPR_DEFAULT_MAX_STREAM (8)

// TODO fix this, we should update zones when we get to large offset
// TODO deprecated already...
#define _TRIPR_MAX_MESSAGE_ID (1 << 20)
#define _TRIPR_MAX_TIMEOUT (1024)



struct _trip_router_s
{
    /* Frequently Accessed */
    void *data;

    /* User Hooks */
    trip_handle_screen_t *screen;
    trip_handle_watch_t *watch;
    trip_handle_timeout_t *timeout;
    trip_handle_connection_t *connection;
    trip_handle_stream_t *stream;
    trip_handle_message_t *message;

    /* Wait Data */
    _trip_poll_t *poll;

    /* State */
    enum _tripr_state state;
    uint64_t statedeadline;// TODO I think this is useless because of the timerwheel
    timer_entry_t *statetimer;

    /* Error Handling */
    int error;
    char *errmsg;

    /* Resolve pending. */
    resolveq_t resolveq;

    /* Connections. */
    connmap_t conn;
    // TODO create real datastructure, src should be easily indexed...
    _trip_connection_t *connsrc[2];

    /* Socket map. */
    sockmap_t sockmap;

    /* Send Management */
    sendq_t sendq;
    unsigned char *buf;
    size_t buflen;
    size_t sendlen; // set when the buffer has a packet to send
    int sendsrc;

    /* Packet Interface */
    trip_packet_t *packet;

    /* Timer Wheel */
    timerwheel_t wheel;
    uint64_t mindeadline;

    /* Encryption */
    unsigned char *openpk;
    unsigned char *opensk;
    unsigned char *signpub;
    unsigned char *signsec;

    /* Limits */
    //limits_t lim; // TODO move below to limits structure
    uint32_t max_conn;// TODO use uppermost bits on max_conn for connection ID randomization??
    uint32_t max_in;
    uint32_t max_out;
    uint32_t max_packet_read_count;
    uint32_t max_packet_send_count;
    uint32_t max_connection_send_count;
    uint32_t max_streams;

    // TODO move to own struct
    int timeout_data;
    int timeout_bind;
    int timeout_close;
    int timeout_unbind;

    /* Boolean flags. */
    uint32_t flag;
};

void
_trip_close_connection(_trip_router_t *r, _trip_connection_t *c);
void
_trip_set_state(_trip_router_t *r, enum _tripr_state state);
void
_trip_qconnection(_trip_router_t *r, _trip_connection_t *c);
void
_trip_unqconnection(_trip_router_t *r, _trip_connection_t *c);
timer_entry_t *
_trip_set_timeout(_trip_router_t *r, int ms, void *data, timer_cb_t *cb);
void
_trip_cancel_timeout(timer_entry_t *e);

#ifdef __cplusplus
}
#endif
#endif /* _LIBTRP_ROUTER_H_ */

