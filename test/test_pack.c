
#include "libtrp.h"
#include "../_libtrp.h"

#include <assert.h>


int
main()
{
    int min = -1;
    int max = -1;
    trip_pack_approx("cChHiIqQpsVnk", &min, &max);

    int expectmin = 2 + 4 + 8 + 16 + 1 + _TRIP_NONCE + TRIP_KP_PUB;
    int expectmax = 2 + 4 + 8 + 16 + 5 + _TRIP_NONCE + TRIP_KP_PUB;

    assert(min == expectmin);
    assert(max == expectmax);

    return 0;
}


