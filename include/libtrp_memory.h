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
 * @file libtrp_memory.h
 * @author Craig Jacobson
 * @brief Memory interface.
 */
#ifndef LIBTRP_MEMORY_H_
#define LIBTRP_MEMORY_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdlib.h>



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


#ifdef __cplusplus
}
#endif
#endif /* LIBTRP_MEMORY_H_ */

