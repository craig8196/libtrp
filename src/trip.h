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


#define trip_torouter(R, _R) _trip_router_t * R  = ((_trip_router_t *)(_R));

typedef struct _trip_router_s _trip_router_t;
typedef struct _trip_connection_s _trip_connection_t;
typedef struct _trip_stream_s _trip_stream_t;
typedef struct _trip_msg_s _trip_msg_t;
typedef struct _tripbuf_s _tripbuf_t;
typedef struct _trip_wait_s _trip_wait_t;
typedef struct _trip_prefix_s _trip_prefix_t;
typedef struct _trip_open_s _trip_open_t;

enum _tripr_state
{
    _TRIPR_STATE_START,
    _TRIPR_STATE_STOP,
    _TRIPR_STATE_BIND,
    _TRIPR_STATE_LISTEN,
    _TRIPR_STATE_NOTIFY,
    _TRIPR_STATE_CLOSE,
    _TRIPR_STATE_UNBIND,
    _TRIPR_STATE_END,
    _TRIPR_STATE_ERROR,
};

#define _TRIP_MAX_EVENTS (16)
struct _trip_wait_s
{
    int efd;
    int timeout;
};

#define _TRIP_ALLOW_IN         (1 << 0)
#define _TRIP_ALLOW_OUT        (1 << 1)
#define _TRIP_ALLOW_PLAINOPEN  (1 << 2)
#define _TRIP_ALLOW_PLAINSEGM  (1 << 3)
#define _TRIP_ALLOW_PLAINSIGN  (1 << 4)

struct _trip_router_s
{
    /* Frequently Accessed */
    void *ud;

    /* User Hooks */
    trip_handle_screen_t *screen;
    trip_handle_watch_t *watch;
    trip_handle_timeout_t *timeout;
    trip_handle_connection_t *connection;
    trip_handle_stream_t *stream;
    trip_handle_message_t *message;

    /* Wait Data */
    _trip_wait_t *wait;
    int wtimeout;

    /* State */
    enum _tripr_state state;
    uint64_t statedeadline;

    /* Error Handling */
    int error;
    char *errmsg;

    /* Connection Info
     * An array of pointers is currently used.
     * The connection's ID is also the index when modulo conlen.
     * This gives us lookup speed and some level of randomness.
5    * Also, guarantee's ID uniqueness within the array.
     * Example:
     * conmask = 0x03;
     * concap = 4;
     * id = 0xABCDEF11; // upper bits can be random if above max_conn
     * true = id % concap == id & conmask == index == 1;
     */
    uint32_t conmask; /* concap - 1 */
    uint32_t concap; /* Power of 2. */
    uint32_t conlen; /* Number of connections. */
    _trip_connection_t **con; /* Array of connections. */
    _trip_connection_t *confree; /* Linked list of previously used connections. */
    _trip_connection_t **conempty; /* Pointer to where to start searching. */

    /* Send Management */
    _trip_connection_t *sendbeg;
    _trip_connection_t *sendend;
    _tripbuf_t *sendbuf;
    _tripbuf_t *decryptbuf;

    /* Memory, Packet, and Encryption */
    trip_memory_t *mem;
    trip_packet_t *pack;
    unsigned char *openpub;
    unsigned char *opensec;
    unsigned char *signpub;
    unsigned char *signsec;

    /* Limits */
    uint32_t max_mem;
    uint32_t max_packet_mem;
    uint32_t max_conn;// TODO use uppermost bits on max_conn for connection ID randomization??
    uint32_t max_in;
    uint32_t max_out;
    uint32_t max_packet_read_count;
    uint32_t max_connection_send;
    uint32_t max_message_id;

    /* Settings */
    // TODO compress to bit field
    uint32_t allow;
    bool allow_in;
    bool allow_out;
    bool allow_plain_open;
    bool allow_plain_seg;
    bool allow_plain_sig;

    int timeout_bind;
    int timeout_data;
    int timeout_close;
    int timeout_stop;
    int timeout_notify;
};

#ifdef __cplusplus
}
#endif
#endif /* _LIBTRP_ROUTER_H_ */
