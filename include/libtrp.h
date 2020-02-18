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


/* TRiP Forward-Declared Handles */
typedef struct trip_router_handle{int x;} trip_router_t;
//typedef struct trip_packet_handle{} trip_pkt_t;
//typedef struct trip_connection_handle{} TRIPC;
//typedef struct trip_stream_handle{} TRIPS;


/* TRiP Global Init/Destroy */
//int
//trip_global_init(void);
//void
//trip_global_destroy(void);


/* TRiP Synchronous Operation */
//int
//trip_router_perform(TRIPR *r);


/* TRiP Asynchronous Hooks */
//typedef int trip_async_socket_cb(void *ud, TRIPR *r, trip_socket_t s, int what);
//typedef int trip_async_timeout_cb(void *ud, TRIPR *r, long timeout_ms);

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


/* TRiP Cryptography */
typedef struct trip_signature_handle{bool x;} trip_signature_t;
typedef struct trip_keypair_handle{bool x;} trip_keypair_t;

trip_signature_t *
trip_signature_new(trip_memory_t *m);
void
trip_signature_free(trip_memory_t *m, trip_signature_t *);

trip_keypair_t *
trip_keypair_new(trip_memory_t *m);
void
trip_keypair_free(trip_memory_t *m, trip_keypair_t *);


/* TRiP Packet Interface */
typedef void trip_packet_bind_cb_t(void *, int);
typedef void trip_packet_bind_t(void *, void *, trip_packet_bind_cb_t);
typedef void trip_packet_resolve_cb_t(void *, int);
typedef void trip_packet_resolve_t(void *, void *, void *, trip_packet_resolve_cb_t);
typedef int trip_packet_send_t(void *, uint8_t *, size_t);
typedef int trip_packet_unbind_t(void *);

typedef struct trip_packet_s
{
    bool reliable;
    int fd;
    void *ud;
    trip_packet_bind_t *bind;
    trip_packet_resolve_t *resolve;
    trip_packet_send_t *send;
    trip_packet_unbind_t *unbind;
} trip_packet_t;

trip_packet_t *
trip_packet_new_udp(trip_memory_t *m, const char *info);
void
trip_packet_free_udp(trip_packet_t *);


/* TRiP Router Interface */
typedef int trip_screen_t(void *ud);
enum trip_preset
{
    TRIP_PRESET_SERVER,
    TRIP_PRESET_CLIENT,
};
enum trip_router_opt
{
    TRIPOPT_KEYPAIR,
    TRIPOPT_SIGNATURE,
    TRIPOPT_WATCH,
    TRIPOPT_TIMEOUT,
};

trip_router_t *
trip_router_new_preset(enum trip_preset preset, trip_memory_t *m, trip_packet_t *p);
int
trip_router_setopt(trip_router_t *r, enum trip_router_opt opt, ...);
int
trip_router_perform(trip_router_t *r);
int
trip_router_watch(trip_router_t *r, uint32_t timeout);
//trip_connection_t *
//trip_router_mk_connection(trip_router_t *r, const char *info, void *, trip_connection_stream_cb_t cb);
int
trip_router_stop(trip_router_t *r);
void
trip_router_free(trip_router_t *r);


/* TRiP Connection Interface */
//int
//trip_connection_close(TRIPC *c);
//TRIPS *
//trip_connection_mk_stream(TRIPC *c);


/* TRiP Stream Interface */
//int
//trip_stream_close(TRIPS *s);
//int
//trip_stream_send(TRIPS *s, void *d, size_t ds);

/*
module.exports = {
  // Typical-use API
  mkNonce,
  mkKeyPair,
  mkSignPair,
  mkSocket,
  mkServer,
  mkClient,
  SettingPreset,
  mkSettings,
  // Low-level API for Customization
  SocketInterface,
  SenderInterface,
  mkRouter,
};
*/




#ifdef __cplusplus
}
#endif
#endif /* LIBTRP_H_ */

