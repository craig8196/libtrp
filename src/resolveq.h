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
 * @file resolveq.h
 * @author Craig Jacobson
 * @brief Resolve Q. Connections pending resolving.
 *
 * The entire reason for this is to reduce size of cancelled connection
 * due to resolve timeout.
 * If the connection information is still resolving and we timeout, then
 * we don't want the connection information stuck in packet interface
 * space. We decouple the two so the user can be notified and the connection
 * can be cleaned up ASAP.
 */
#ifndef _LIBTRP_RESOLVEQ_H_
#define _LIBTRP_RESOLVEQ_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdbool.h>

#include "conn.h"


typedef struct resolveq_union_s
{
    union
    {
        void *data;
        struct resolveq_union_s *next;
    } u;
    bool inuse;
} resolveq_union_t;

typedef struct resolveq_s
{
    /* Length of the map/array. */
    int len;
    int size;
    /* Map. */
    resolveq_union_t *map;
    /* Empty slot list. */
    resolveq_union_t *free;
} resolveq_t;

void
resolveq_init(resolveq_t *q);

void
resolveq_destroy(resolveq_t *q);

void
resolveq_clear(resolveq_t *q);

int
resolveq_put(resolveq_t *q, _trip_connection_t *c);

_trip_connection_t *
resolveq_pop(resolveq_t *q, int key);

void
resolveq_del(resolveq_t *q, int key);



#ifdef __cplusplus
}
#endif
#endif /* _LIBTRP_RESOLVEQ_H_ */


