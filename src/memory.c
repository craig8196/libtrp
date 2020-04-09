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
#include "libtrp.h"
#include "util.h"

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
