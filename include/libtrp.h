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
 * @file libtrp.h
 * @author Craig Jacobson
 * @brief TRiP implementation.
 */
#ifndef LIBTRP_H_
#define LIBTRP_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <sodium.h>


/* TRiP Forward-Declared Handles */
typedef struct trip_router_handle_s     { void *ud;}
        trip_router_t;
typedef struct trip_connection_handle_s { void *ud; }
        trip_connection_t;
typedef struct trip_stream_handle_s     { void *ud; }
        trip_stream_t;
typedef struct trip_buffer_handle_s     { char _hidden; }
        tripbuf_t;


/* TRiP Global Init/Destroy */
int
trip_global_init(void);
void
trip_global_destroy(void);


/* TRiP Time */
uint64_t
triptime_now(void);
uint64_t
triptime_deadline(int);
int
triptime_timeout(uint64_t, uint64_t);


/* TRiP Memory Interface */
typedef void *trip_memory_alloc_t(void *, size_t);
typedef void *trip_memory_realloc_t(void *, void *, size_t);
typedef void trip_memory_free_t(void *, void *);

typedef struct trip_memory_s
{
    void *ud;
    trip_memory_alloc_t *alloc;
    trip_memory_realloc_t *realloc;
    trip_memory_free_t *free;
} trip_memory_t;

trip_memory_t *
trip_memory_default(void);


/* TRiP Buffers */
size_t
tripbuf_len(tripbuf_t *);
size_t
tripbuf_cap(tripbuf_t *);
void *
tripbuf(tripbuf_t *);


/* TRiP Cryptography for User */
#define TRIP_SIGN_PUB (crypto_sign_PUBLICKEYBYTES)
#define TRIP_SIGN_SEC (crypto_sign_SECRETKEYBYTES)
#define TRIP_KP_PUB (crypto_box_PUBLICKEYBYTES)
#define TRIP_KP_SEC (crypto_box_SECRETKEYBYTES)
#define trip_sign_kp crypto_sign_keypair
#define trip_kp crypto_box_keypair


/* TRiP Socket Abstraction */
typedef int trip_socket_t;
#define TRIP_SOCKET_TIMEOUT (-1)


/* TRiP Packet Interface */
typedef void trip_packet_bind_t(void *);
typedef void trip_packet_resolve_t(void *, trip_connection_t *);
typedef int trip_packet_send_t(void *, tripbuf_t *);
typedef int trip_packet_read_t(void *, trip_socket_t fd, int events, int max);
typedef int trip_packet_unbind_t(void *);
typedef int trip_packet_wait_t(void *);


typedef struct trip_packet_s
{
    void *ud;
    int *fds;
    int *fde;
    size_t fdlen;
    trip_memory_t *mem;
    trip_packet_bind_t *bind;
    trip_packet_resolve_t *resolve;
    trip_packet_send_t *send;
    trip_packet_read_t *read;
    trip_packet_unbind_t *unbind;
    trip_packet_wait_t *wait;
    /* Filled out by router if not by you. */
    trip_router_t *router;
} trip_packet_t;

trip_packet_t *
trip_packet_new_udp(trip_memory_t *m, const unsigned char *info);
void
trip_packet_free_udp(trip_packet_t *);

enum trip_socket_event
{
    TRIP_SOCK_EVENT_REMOVE  = 0,
    TRIP_SOCK_EVENT_IN      = 1,
    TRIP_SOCK_EVENT_OUT     = 2,
    TRIP_SOCK_EVENT_INOUT   = 3,
    TRIP_SOCK_EVENT_ADD     = 4,
};


/* TRiP Packet to Router Communication */
void
trip_seg(trip_router_t *r, int src, size_t len, void *buf);
void
trip_ready(trip_router_t *r);
void
trip_resolve(trip_router_t *r, trip_connection_t *c, int err);
void
trip_watch(trip_router_t *r, trip_socket_t fd, int events);
void
trip_timeout(trip_router_t *r, int timeoutms);
void
trip_unready(trip_router_t *r, int err);


/* TRiP Router Interface */
typedef void trip_handle_watch_t(trip_router_t *, trip_socket_t, int);
typedef void trip_handle_timeout_t(trip_router_t *, int);

typedef void trip_handle_screen_t(void *);
typedef void trip_handle_connection_t(trip_connection_t *);
typedef void trip_handle_stream_t(trip_stream_t *, bool open);
enum trip_message_code
{
    TRIPM_RECV,
    TRIPM_SENT,
    // Message not fully sent or received on other end.
    TRIPM_KILL,
};
typedef void trip_handle_message_t(trip_stream_t *, enum trip_message_code, size_t, unsigned char *);

enum trip_preset
{
    TRIP_PRESET_SERVER,
    TRIP_PRESET_CLIENT,
    TRIP_PRESET_MULTICLIENT,
};

enum trip_router_opt
{
    TRIPOPT_OPEN_KP,
    TRIPOPT_SIGN_KP,
    TRIPOPT_USER_DATA,
    TRIPOPT_WATCH_CB,
    TRIPOPT_TIMEOUT_CB,
    TRIPOPT_SCREEN_CB,
    TRIPOPT_CONNECTION_CB,
    TRIPOPT_STREAM_CB,
    TRIPOPT_MESSAGE_CB,
};

trip_router_t *
trip_new(enum trip_preset preset, trip_memory_t *m, trip_packet_t *p);
void
trip_free(trip_router_t *r);

int
trip_setopt(trip_router_t *r, enum trip_router_opt opt, ...);

const char *
trip_errmsg(trip_router_t *_r);

int
trip_action(trip_router_t *, trip_socket_t, int);

int
trip_start(trip_router_t *r);
int
trip_run(trip_router_t *r, int timeout);
int
trip_stop(trip_router_t *r);

void
trip_open_connection(trip_router_t *r, void *, size_t, const unsigned char *);



/* TRiP Connection Interface */
#define TRIPC_STATUS_OPEN   (1 << 0)
#define TRIPC_STATUS_CLOSED (1 << 1)
#define TRIPC_STATUS_ERROR  (1 << 2)
int
tripc_status(trip_connection_t *c);
void
tripc_close(trip_connection_t *c);
#define TRIPS_OPT_PRIORITY (1 << 0)
#define TRIPS_OPT_CHUNK    (1 << 1)
#define TRIPS_OPT_ORDERED  (1 << 2)
#define TRIPS_OPT_RELIABLE (1 << 3)
trip_stream_t *
tripc_open_stream(trip_connection_t *, int, int);


/* TRiP Stream Interface */
bool
trips_isopen(trip_stream_t *s);
void
trips_close(trip_stream_t *s);
int
trips_send(trip_stream_t *s, size_t, unsigned char *);




#ifdef __cplusplus
}
#endif
#endif /* LIBTRP_H_ */

