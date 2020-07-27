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
 */
#ifndef _LIBTRP_ROUTER_H_
#define _LIBTRP_ROUTER_H_
#ifdef __cplusplus
extern "C" {
#endif


#include "core.h"
#include "connmap.h"
#include "sendq.h"
#include "sockmap.h"


typedef struct _trip_router_s _trip_router_t;
typedef struct _trip_connection_s _trip_connection_t;
typedef struct _trip_stream_s _trip_stream_t;
typedef struct _trip_msg_s _trip_msg_t;
typedef struct _trip_poll_s _trip_poll_t;
typedef struct _trip_prefix_s _trip_prefix_t;
typedef struct _trip_route_s _trip_route_t;
typedef struct _trip_open_s _trip_open_t;

enum _tripr_state
{
    _TRIPR_STATE_START,
    _TRIPR_STATE_STOP,
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
#define _TRIPR_FLAG_ALLOW_PLAIN_SEGM   (1 << 3)
#define _TRIPR_FLAG_ALLOW_PLAIN_SIGN   (1 << 4)
#define _TRIPR_FLAG_FREE_PACKET        (1 << 5)

#define _TRIPR_DEFAULT_MAX_CONN (1 << 19)

#define _TRIP_MAX_EVENTS (16)
struct _trip_poll_s
{
    int efd;
    int timeout;
};

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
    uint64_t statedeadline;

    /* Error Handling */
    int error;
    char *errmsg;

    /* Connections. */
    connmap_t con;

    /* Socket map. */
    sockmap_t sockmap;

    /* Send Management */
    sendq_t sendq;

    /* Packet Interface */
    trip_packet_t *packet;

    /* Encryption */
    unsigned char *openpub;
    unsigned char *opensec;
    unsigned char *signpub;
    unsigned char *signsec;

    /* Limits */
    //limits_t lim; // TODO move below to limits structure
    uint32_t max_conn;// TODO use uppermost bits on max_conn for connection ID randomization??
    uint32_t max_in;
    uint32_t max_out;
    uint32_t max_packet_read_count;
    uint32_t max_connection_send;
    uint32_t max_message_id;

    // TODO move to own struct
    int timeout_data;
    int timeout_bind;
    int timeout_close;
    int timeout_unbind;

    /* Boolean flags. */
    uint32_t flag;
};

void
_trip_set_state(_trip_router_t *r, enum _tripr_state state);

#ifdef __cplusplus
}
#endif
#endif /* _LIBTRP_ROUTER_H_ */

