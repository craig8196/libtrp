/**
 * @file buffer.c
 * @author Craig Jacobson
 * @brief External interface for buffer use.
 */
#include <stdlib.h>
#include "libtrp.h"
#include "_libtrp.h"


size_t
tripbuf_len(tripbuf_t *_b)
{
    trip_bufferify(b, _b);
    return b->len;
}

size_t
tripbuf_cap(tripbuf_t *_b)
{
    trip_bufferify(b, _b);
    return b->cap;
}

void *
tripbuf(tripbuf_t *_b)
{
    trip_bufferify(b, _b);
    return (void *)(b + 1);
}

