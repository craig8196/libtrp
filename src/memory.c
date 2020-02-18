
#include "libtrp.h"
#include "_libtrp.h"

#include <stdlib.h>


void *
trip_memory_alloc_impl(void * UNUSED(ud), size_t len)
{
    return malloc(len);
}

void *
trip_memory_realloc_impl(void * UNUSED(ud), void *p, size_t len)
{
    return realloc(p, len);
}

void
trip_memory_free_impl(void * UNUSED(ud), void *p)
{
    free(p);
}

static trip_memory_t g_trip_memory =
{
    NULL,
    trip_memory_alloc_impl,
    trip_memory_realloc_impl,
    trip_memory_free_impl
};

trip_memory_t *
trip_memory_default(void)
{
    return &g_trip_memory;
}

