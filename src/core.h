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
 * @file core.h
 * @author Craig Jacobson
 * @brief Core typedefs used in many places.
 *
 * Prefixes:
 * _trip  -> router/top level
 * _tripc -> connection
 * _trips -> stream
 * _tripm -> message
 * _tripbuf -> buffer
 */
#ifndef _LIBTRP_CORE_H_
#define _LIBTRP_CORE_H_
#ifdef __cplusplus
extern "C" {
#endif


#define trip_torouter(R, _R) _trip_router_t * R  = ((_trip_router_t *)(_R));
#define trip_toconn(C, _C) _trip_connection_t * C  = ((_trip_connection_t *)(_C));
#define trip_tostream(S, _S) _trip_stream_t * S  = ((_trip_stream_t *)(_S));

typedef struct _trip_router_s _trip_router_t;
typedef struct _trip_connection_s _trip_connection_t;
typedef struct _trip_stream_s _trip_stream_t;
typedef struct _trip_msg_s _trip_msg_t;
typedef struct _trip_part_s _trip_part_t;
typedef struct _trip_wait_s _trip_wait_t;
typedef struct _trip_prefix_s _trip_prefix_t;
typedef struct _trip_open_s _trip_open_t;


#ifdef __cplusplus
}
#endif
#endif /* _LIBTRP_CORE_H_ */

