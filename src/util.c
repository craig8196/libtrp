
#include "util.h"

#include <string.h>

#include "libtrp_memory.h"


static const int MAX_SIZE = 1 << ((sizeof(int) * 8) - 2);

void *
tripm_cfree(void *p)
{
    if (p)
    {
        tripm_free(p);
    }
    return NULL;
}

void *
tripm_bdup(size_t len, unsigned char *buf)
{
    if (len)
    {
        void *bdup = tripm_alloc(len);
        if (bdup)
        {
            memcpy(bdup, buf, len);
        }
        return bdup;
    }
    else
    {
        return NULL;
    }
}

int
near_pwr2(int n)
{
    if (n < 1)
    {
        return 1;
    }

    if (!(n & (n - 1)))
    {
        return n;
    }

    if (n > MAX_SIZE)
    {
        return MAX_SIZE;
    }

    int p = 1;

    while (p < n)
    {
        p <<= 1;
    }

    return p;
}

/**
 * @return The lowest power of 2 >= the value given.
 */
uint64_t
near_pwr2_64(uint64_t n)
{
    --n;
    n |= (n >> 1);
    n |= (n >> 2);
    n |= (n >> 4);
    n |= (n >> 8);
    n |= (n >> 16);
    n |= (n >> 32);
    return n + 1;
}

bool
are_zeros(size_t len, unsigned char *buf)
{
    size_t i;
    for (i = 0; i < len; ++i)
    {
        if (buf[i] != 0)
        {
            return false;
        }
    }

    return true;
}

