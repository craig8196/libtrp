
#include "util.h"


static const int MAX_SIZE = 1 << ((sizeof(int) * 8) - 2);

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


