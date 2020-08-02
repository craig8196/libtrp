
#include "reliable_packet.h"

#include <errno.h>
#include <stdlib.h>

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

typedef struct reliable_s
{
    /*
     * 0 - Opened connection from calling trip_open_connection.
     * 1 - Received connection.
     */
    void *c[2];
    /*
     * Allocated buffer.
     */
    size_t alen;
    size_t len;
    unsigned char *buf;
} reliable_t;

static void
reliable_bind(trip_packet_t *p)
{
    trip_ready(p->router);
    trip_timeout(p->router, 1);
}

static void
reliable_unbind(trip_packet_t *p)
{
    trip_unready(p->router, 0);
}

static void
reliable_resolve(trip_packet_t *p, trip_connection_t *c)
{
    reliable_t *rel = p->data;
    if (NULL == rel->c[0])
    {
        rel->c[0] = c;
        c->src = 1;
        trip_resolve(p->router, c, 0);
    }
    else
    {
        trip_resolve(p->router, c, EBUSY);
    }
}

static int
reliable_send(trip_packet_t *p, int src, size_t len, void *buf)
{
    /* Toggle the src descriptor.
     * Pass the  buffer through to the one other connection.
     */
    trip_seg(p->router, src ^ 1, len, buf);
    return 0;
}

static int
reliable_read(trip_packet_t * UNUSED(p), trip_socket_t UNUSED(s), int UNUSED(max))
{
    /* Since we pass the buffer directly to the other connection
     * we don't ever have to read.
     */
    return 0;
}

static int
reliable_wait(trip_packet_t * UNUSED(p))
{
    return 1;
}


trip_packet_t *
reliable_new(void)
{
    trip_packet_t *p = malloc(sizeof(trip_packet_t));

    reliable_t *rel = malloc(sizeof(reliable_t));
    rel->c[0] = NULL;
    rel->c[1] = NULL;
    rel->alen = 512;
    rel->len = 0;
    rel->buf = malloc(rel->alen);

    p->data = rel;
    p->bind = &reliable_bind;
    p->resolve = &reliable_resolve;
    p->send = &reliable_send;
    p->read = &reliable_read;
    p->unbind = &reliable_unbind;
    p->wait = &reliable_wait;

    return p;
}

void
reliable_free(trip_packet_t *p)
{
    free(p->data);
    free(p);
}



