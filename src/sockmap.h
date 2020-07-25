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
 * @file sockmap.h
 * @author Craig Jacobson
 * @brief Socket descriptor map: trip_socket_t -> (void *).
 */
#ifndef _LIBTRP_SOCKET_MAP_H_
#define _LIBTRP_SOCKET_MAP_H_
#ifdef __cplusplus
extern "C" {
#endif


#include "libtrp_handles.h"


typedef struct sockmap_entry_s
{
    trip_socket_t fd;
    void *data;
} sockmap_entry_t;


/**
 * Simple map for file descriptor lookups.
 * An array is used because this map is not expected to ever be very long.
 */
typedef struct sockmap_s
{
    int len; /* Length. */
    int alen; /* Allocated length. */
    sockmap_entry_t *map;
} sockmap_t;

void
sockmap_init(sockmap_t *map);

void
sockmap_destroy(sockmap_t *map);

int
sockmap_put(sockmap_t *map, trip_socket_t fd, void *data);

void
sockmap_del(sockmap_t *map, trip_socket_t fd);

void *
sockmap_get(sockmap_t *map, trip_socket_t fd);


#ifdef __cplusplus
}
#endif
#endif /* _LIBTRP_SOCKET_MAP_H_ */


