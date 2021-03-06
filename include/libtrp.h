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

#include "libtrp_handles.h"
#include "libtrp_packet.h"


#define TRIP_VERSION_MAJOR (0)


/* TRiP Global Init/Destroy */
int
trip_global_init(void);
void
trip_global_destroy(void);


/* TRiP Cryptography for User */
#define TRIP_SIGN_PUB (crypto_sign_PUBLICKEYBYTES)
#define TRIP_SIGN_SEC (crypto_sign_SECRETKEYBYTES)
#define TRIP_KEY_PUB (crypto_box_PUBLICKEYBYTES)
#define TRIP_KEY_SEC (crypto_box_SECRETKEYBYTES)
#define trip_sign_kp crypto_sign_keypair
#define trip_kp crypto_box_keypair


/* TRiP Socket Abstraction */
#define TRIP_SOCKET_TIMEOUT ((trip_socket_t)(-1))
enum trip_socket_event
{
    TRIP_REMOVE  = 0,
    TRIP_IN      = 1,
    TRIP_OUT     = 2,
    TRIP_INOUT   = 3,
};


/* TRiP Router Interface */
typedef void trip_handle_watch_t(trip_router_t *, trip_socket_t, int, void *);
typedef void trip_handle_timeout_t(trip_router_t *, long);

typedef struct trip_screen_s
{
    bool allow;
    uint16_t version; /* Major */
    uint32_t routelen;
    unsigned char *route;
    void *data;
    unsigned char *opensk; /* Decrypt custom incoming OPEN */
    unsigned char *signpk; /* Verify custom incoming OPEN */
    unsigned char *signsk; /* Verify custom outgoing CHALLENGE */
} trip_screen_t;

typedef void trip_handle_screen_t(trip_router_t *r, trip_screen_t *);
typedef void trip_handle_connection_t(trip_connection_t *);
typedef void trip_handle_stream_t(trip_stream_t *);
enum trip_message_status
{
    /* Receiving message. */
    TRIPM_RECV,

    /* Message sent (confirmed if reliable). */
    TRIPM_SENT,
    /* Message not fully sent or received on other end. */
    TRIPM_KILL,
};
typedef void trip_handle_message_t(trip_stream_t *, enum trip_message_status, size_t, unsigned char *);

enum trip_preset
{
    TRIP_PRESET_ROUTER,
    TRIP_PRESET_SERVER,
    TRIP_PRESET_CLIENT,
    TRIP_PRESET_MULTICLIENT,
};

enum trip_router_opt
{
    TRIPOPT_OPEN_KP,
    TRIPOPT_SIGN_KP,
    TRIPOPT_USER_DATA,
    TRIPOPT_PACKET,
    TRIPOPT_WATCH_CB,
    TRIPOPT_TIMEOUT_CB,
    TRIPOPT_SCREEN_CB,
    TRIPOPT_CONNECTION_CB,
    TRIPOPT_STREAM_CB,
    TRIPOPT_MESSAGE_CB,
    TRIPOPT_ALLOW_PLAIN_OPEN,
    TRIPOPT_ALLOW_PLAIN_ISIG, /* Incoming unsigned packets. */
    TRIPOPT_ALLOW_PLAIN_OSIG, /* Outgoing unsigned packets. */
    TRIPOPT_ALLOW_PLAIN_COMM,
};

trip_router_t *
trip_new(enum trip_preset preset);
void
trip_free(trip_router_t *r);

int
trip_setopt(trip_router_t *r, enum trip_router_opt opt, ...);

const char *
trip_errmsg(trip_router_t *_r);

int
trip_timeout(trip_router_t *);
int
trip_action(trip_router_t *, trip_socket_t, int);
void
trip_assign(trip_router_t *, trip_socket_t, void *);

int
trip_run_init(trip_router_t *_r);
int
trip_start(trip_router_t *r);
int
trip_run(trip_router_t *r, int timeout);
int
trip_stop(trip_router_t *r);

void
trip_open_connection(trip_router_t *r, void *ud, size_t ilen, unsigned char *info);


/* TRiP Connection Interface */
enum trip_connection_status
{
    /* Incoming connections. */
    TRIPC_STATUSI_OPEN   = 0,
    TRIPC_STATUSI_CLOSED = 1,
    TRIPC_STATUSI_KILLED = 2,
    TRIPC_STATUSI_ERROR  = 3,
    /* Outgoing connections. */
    TRIPC_STATUSO_OPEN   = 4,
    TRIPC_STATUSO_CLOSED = 5,
    TRIPC_STATUSO_KILLED = 6,
    TRIPC_STATUSO_ERROR  = 7,
};
enum trip_connection_status
tripc_status(trip_connection_t *c);
void
tripc_close(trip_connection_t *c, int gracems);
#define TRIPS_OPT_PRIORITY (1 << 0)
#define TRIPS_OPT_CHUNK    (1 << 1)
#define TRIPS_OPT_ORDERED  (1 << 2)
#define TRIPS_OPT_RELIABLE (1 << 3)
trip_stream_t *
tripc_open_stream(trip_connection_t *c, int sid, int options);
int
tripc_get_errno(trip_connection_t *c);
const char *
tripc_get_errmsg(trip_connection_t *c);


/* TRiP Stream Interface */
enum trip_stream_status
{
    /* Incoming streams. */
    TRIPS_STATUSI_OPEN   = 0,
    TRIPS_STATUSI_CLOSED = 1,
    TRIPS_STATUSI_KILLED = 2,
    TRIPS_STATUSI_ERROR  = 3,
    /* Outgoing streams. */
    TRIPS_STATUSO_OPEN   = 4,
    TRIPS_STATUSO_CLOSED = 5,
    TRIPS_STATUSO_KILLED = 6,
    TRIPS_STATUSO_ERROR  = 7,
};

int
trips_id(trip_stream_t *s);
enum trip_stream_status
trips_status(trip_stream_t *s);
int
trips_type(trip_stream_t *s);
void
trips_close(trip_stream_t *s);
int
trips_send(trip_stream_t *s, size_t, const unsigned char *);


#ifdef __cplusplus
}
#endif
#endif /* LIBTRP_H_ */

