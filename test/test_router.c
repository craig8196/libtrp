
#include "libtrp.h"
#include <stdio.h>


int
main()
{
    trip_signature_t *sig = trip_signature_new(NULL);
    trip_keypair_t *pair = trip_keypair_new(NULL);
    trip_router_t *server = trip_router_new_preset(TRIP_PRESET_SERVER, NULL, NULL);
    trip_router_setopt(server, TRIPOPT_KEYPAIR, pair);
    trip_router_setopt(server, TRIPOPT_SIGNATURE, sig);

    /*
    int c;
    while (0 == (c = trip_router_perform(server)))
    {
        c = trip_router_watch(server, 1000);
        if (0 != c)
        {
            printf("Error (%d): %s\n", c, trip_router_errmsg(server));
            break;
        }
    }

    trip_router_free(server);
    trip_keypair_free(NULL, pair);
    trip_signature_free(NULL, sig);
    */

    return 0;
}

