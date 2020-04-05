
#include <stdint.h>
#include <time.h>


#define _TRIP_MAX_TIMEOUT (10000)

uint64_t
triptime_now(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_REALTIME, &t) < 0)
    {
        return 0;
    }
    uint64_t n = t.tv_sec + (t.tv_nsec/1000000);
    return n;
}

uint64_t
triptime_deadline(int timeout_ms)
{
    uint64_t n = triptime_now();
    if (timeout_ms >= 0)
    {
        return n + timeout_ms;
    }
    else
    {
        return n - (timeout_ms * -1);
    }
}

int
triptime_timeout(uint64_t deadline, uint64_t now)
{
    int timeout = (int)(deadline - now);
    if (timeout < 0)
    {
        timeout = _TRIP_MAX_TIMEOUT;
    }
    return timeout;
}

