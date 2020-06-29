
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


