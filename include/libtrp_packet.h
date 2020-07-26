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
#ifndef LIBTRP_PACKET_H_
#define LIBTRP_PACKET_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>

#include "libtrp_handles.h"


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
void
trip_error(trip_router_t *r, int error, const char *emsg);


/* TRiP Packet Interface */
typedef struct trip_packet_s trip_packet_t;
typedef void trip_packet_bind_t(trip_packet_t *);
typedef void trip_packet_resolve_t(trip_packet_t *, trip_connection_t *);
typedef int trip_packet_send_t(trip_packet_t *, size_t, void *buf);
typedef int trip_packet_read_t(trip_packet_t *, trip_socket_t fd, int events, int max);
typedef void trip_packet_unbind_t(trip_packet_t *);
typedef int trip_packet_wait_t(trip_packet_t *); // TODO what does trip need to call wait for?

struct trip_packet_s
{
    void *data;
    trip_packet_bind_t *bind;
    trip_packet_resolve_t *resolve;
    trip_packet_send_t *send;
    trip_packet_read_t *read;
    trip_packet_unbind_t *unbind;
    trip_packet_wait_t *wait;
    /* Filled out by router on start. */
    trip_router_t *router;
};

trip_packet_t *
trip_packet_new_udp(const char *info);
void
trip_packet_free_udp(trip_packet_t *);


#ifdef __cplusplus
}
#endif
#endif /* LIBTRP_PACKET_H_ */

