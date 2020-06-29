
#include "libtrp.h"
#include <stdio.h>

/*******************************************************************************
 * CLIENT
 ******************************************************************************/
static void
run_client()
{
}


/*******************************************************************************
 * SERVER
 ******************************************************************************/

typedef struct myserver_s
{
    unsigned char openpub[TRIP_KP_PUB];
    unsigned char opensec[TRIP_KP_SEC];
    unsigned char signpub[TRIP_SIGN_PUB];
    unsigned char signsec[TRIP_SIGN_SEC];
    bool running;
} myserver_t;

/*
 * Possible information:
 * Routing.
 * Source address.
 * Source address binary.
 * Source address as string.
 * Signature.
 * Decrypt OPEN keys.
 */
void
handle_screen(void *ud)
{
    // TODO how do I want to handle screen???
    // Set some values in screen struct....
}

void
handle_connection(trip_connection_t *c, const unsigned char *info)
{
    if (tripc_isopen(c))
    {
        /* New data for this connection's scope. */
    }
    else
    {
        /* Free associated data. */
    }
}

void
handle_stream(trip_stream_t *s)
{
    if (trips_isopen(s))
    {
        /* New data for this stream's scope. */
    }
    else
    {
        /* Free associated data. */
    }
}

void
handle_message(trip_stream_t *s, tripbuf_t *msg)
{
}

static void
run_server()
{
    trip_router_t *server = trip_new(TRIP_PRESET_SERVER, NULL, NULL);
    trip_setopt(server, TRIPOPT_OPEN_KP, ud.openpub, ud.opensec);
    trip_setopt(server, TRIPOPT_SIGN_KP, ud.signpub, ud.signsec);
    trip_setopt(server, TRIPOPT_USER_DATA, &ud);
    trip_setopt(server, TRIPOPT_SCREEN_CB, handle_screen);
    trip_setopt(server, TRIPOPT_CONNECTION_CB, handle_connection);
    trip_setopt(server, TRIPOPT_STREAM_CB, handle_stream);
    trip_setopt(server, TRIPOPT_MESSAGE_CB, handle_message);

    int c = trip_start(server);
    while (ud.running && !c)
    {
        /*
         * Run will setup internals and get things going.
         */
        c = trip_run(server, 1000);
    }

    if (c)
    {
        printf("Error (%d): %s\n", c, trip_errmsg(server));
    }

    trip_free(server);
}

int
main()
{
    myserver_t ud;

    ud.running = true;

    /* Create our own signature. */
    trip_kp(ud.openpub, ud.opensec);
    trip_sign_kp(ud.signpub, ud.signsec);

    const int parent = fork();
    if (parent)
    {
        run_server();
    }
    else
    {
        run_client();
    }

    return 0;
}

