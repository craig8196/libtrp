
#include "reliable_packet.h"

#include <stdlib.h>


static void
reliable_bind(trip_packet_t *p)
{
    trip_ready(p->router);
}

static void
reliable_unbind(trip_packet_t *p)
{
    trip_unready(p->router, 0);
}

static void
reliable_resolve(trip_packet_t *p, trip_connection_t *c)
{
    trip_resolve(p->router, c, 0);
}

static int
reliable_send(trip_packet_t *p, int src, size_t len, void *buf)
{
    trip_seg(p->router, src, len, buf);
    return 0;
}

static int
reliable_read(trip_packet_t *p, trip_socket_t s, int events, int max)
{
}

static int
reliable_wait(trip_packet_t *p)
{
    return 1;
}


trip_packet_t *
reliable_new(void)
{
    trip_packet_t *p = malloc(sizeof(trip_packet_t));

    p->bind = reliable_bind;
    p->resolve = reliable_resolve;
    p->send = reliable_send;
    p->read = reliable_read;
    p->unbind = reliable_unbind;
    p->wait = reliable_wait;

    return p;
}

void
reliable_free(trip_packet_t *p)
{
    free(p);
}



