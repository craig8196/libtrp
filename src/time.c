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
#include <stdint.h>
#include <time.h>


#define _TRIP_MIN_TIMEOUT (1)
#define _TRIP_MAX_TIMEOUT (10000)

uint64_t
triptime_now(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_REALTIME, &t) < 0)
    {
        return 0;
    }
    uint64_t n = ((uint64_t)t.tv_sec * 1000) + (t.tv_nsec/1000000);
    return n;
}

/**
 * @param timeout_ms - Must be zero or positive value.
 */
uint64_t
triptime_deadline(int timeout_ms)
{
    uint64_t n = triptime_now();
    return n + timeout_ms;
}

int
triptime_timeout(uint64_t deadline, uint64_t now)
{
    int timeout = (int)(deadline - now);
    if (timeout > _TRIP_MAX_TIMEOUT)
    {
        timeout = _TRIP_MAX_TIMEOUT;
    }
    else if (timeout < 0)
    {
        timeout = _TRIP_MIN_TIMEOUT;
    }
    return timeout;
}

