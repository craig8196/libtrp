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
#ifndef LIBTRP_HANDLES_H_
#define LIBTRP_HANDLES_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stddef.h>


/* TRiP Forward-Declared Handles */
typedef struct trip_router_handle_s
{
    void *data;
} trip_router_t;

typedef struct trip_connection_handle_s
{
    void *data;
    int src;
    size_t ilen;
    unsigned char *info;
} trip_connection_t;

typedef struct trip_stream_handle_s
{
    void *data;
    trip_connection_t *connection;
} trip_stream_t;

typedef int trip_socket_t;



#ifdef __cplusplus
}
#endif
#endif /* LIBTRP_HANDLES_H_ */

