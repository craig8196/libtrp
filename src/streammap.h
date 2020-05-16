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
 * @file streammap.h
 * @author Craig Jacobson
 * @brief Stream map.
 */
#ifndef _LIBTRP_STREAM_MAP_H_
#define _LIBTRP_STREAM_MAP_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdbool.h>
#include <stdint.h>

#include "stream.h"


/**
 * Static array.
 */
typedef struct streammap_s
{
    /* Size of the map. */
    int size;
    /* Capacity of the map. */
    int cap;
    /* Map. NULL if not allocated. */
    _trip_stream_t **map;
    /* Empty slot list. */
    void *free;
} streammap_t;

void
streammap_init(streammap_t *map, int max);

void
streammap_destroy(streammap_t *map);

int
streammap_iter_beg(streammap_t *map);

int
streammap_iter_end(streammap_t *map);

bool
streammap_has_space(streammap_t *map);

int
streammap_add(streammap_t *map, _trip_stream_t *s);

_trip_stream_t *
streammap_del(streammap_t *map, int index);

_trip_stream_t *
streammap_get(streammap_t *map, int index);


#ifdef __cplusplus
}
#endif
#endif /* _LIBTRP_STREAM_MAP_H_ */

