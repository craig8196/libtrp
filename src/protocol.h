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
 * @file protocol.h
 * @author Craig Jacobson
 * @brief Protocol masks and numbers.
 */
#ifndef _LIBTRP_PROTOCOL_H_
#define _LIBTRP_PROTOCOL_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>


enum _trip_control
{
    _TRIP_CONTROL_OPEN,
    _TRIP_CONTROL_MAX,
};

#define _TRIP_PREFIX_EMASK (0x80)

struct _trip_prefix_s
{
    uint8_t control;
    uint64_t id;
    uint32_t seq;
    bool encrypted;
};

struct _trip_route_s
{
    uint16_t version;
    uint64_t len;
    void *route;
};


#ifdef __cplusplus
}
#endif
#endif /* _LIBTRP_PROTOCOL_H_ */

