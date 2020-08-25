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
 * @file connmap.h
 * @author Craig Jacobson
 * @brief Connection map.
 */
#ifndef _LIBTRP_CONNECTION_MAP_H_
#define _LIBTRP_CONNECTION_MAP_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>

#include "conn.h"


typedef struct connmap_entry_s
{
    bool isfull;
    union
    {
        _trip_connection_t *c;
        size_t next;
    } u;
} connmap_entry_t;

/**
 * Currently limited to power of 2 max connections.
 * The connection's ID is also the index when modulo conlen.
 * This gives us lookup speed and some level of randomness.
 * Also, guarantee's ID uniqueness within the array.
 * Example:
 * mask = 0x03;
 * cap = 4;
 * id = 0xABCDEF11; // upper bits can be random if above max
 * (id % cap) == (id & mask) == (index == 1);
 */
typedef struct connmap_s
{
    /* Mask for indexing. +1 for max number of entries, determines upper bits. */
    uint64_t mask;
    /* Size of the map. */
    size_t size;
    /* Capacity of the map. */
    size_t cap;
    /* Bitmap. */
    // TODO bitmap_t *bitmap;
    /* Map. */
    connmap_entry_t *map;
    /* Empty slot list. */
    size_t free;
} connmap_t;

void
connmap_init(connmap_t *map, uint64_t max);

void
connmap_destroy(connmap_t *map);

void
connmap_clear(connmap_t *map);

size_t
connmap_iter_beg(connmap_t *map);

_trip_connection_t *
connmap_iter_get(connmap_t *map, size_t it);

size_t
connmap_iter_end(connmap_t *map);

int
connmap_add(connmap_t *map, _trip_connection_t *conn);

_trip_connection_t *
connmap_del(connmap_t *map, uint64_t id);

_trip_connection_t *
connmap_get(connmap_t *map, uint64_t id);


#ifdef __cplusplus
}
#endif
#endif /* _LIBTRP_CONNECTION_MAP_H_ */

