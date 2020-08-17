
#include "reliable_packet.h"

#include <errno.h>
#include <stdlib.h>

#include "libtrp.h"


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
    int c[2];
    /*
     * Allocated buffer.
     */
    size_t alen;
    size_t len;
    unsigned char *buf;
    triptimer_t *timer;
} reliable_t;

void
reliable_set_timeout(trip_packet_t *p);

static void
reliable_timeout_cb(void *_p)
{
    trip_packet_t *p = (trip_packet_t *)_p;
    reliable_set_timeout(p);
}

void
reliable_set_timeout(trip_packet_t *p)
{
    reliable_t *r = p->data;
    r->timer = trip_timeout(p->router, 50, (void *)p, reliable_timeout_cb);
}

static void
reliable_bind(trip_packet_t *p)
{
    trip_ready(p->router);
    trip_watch(p->router, TRIP_SOCKET_TIMEOUT, 0);
    reliable_set_timeout(p);
}

static void
reliable_unbind(trip_packet_t *p)
{
    reliable_t *r = p->data;
    trip_timeout_cancel(r->timer);
    r->timer = NULL;
    trip_unready(p->router, 0);
}

static void
reliable_resolve(trip_packet_t *p, int rkey, size_t ilen, const unsigned char *info)
{
    ilen = ilen;
    info = info;
#if 1
    reliable_t *rel = p->data;
    if (0 == rel->c[0])
    {
        rel->c[0] = 1;
        trip_resolve(p->router, rkey, 1, 0, NULL);
    }
    else
    {
        trip_resolve(p->router, rkey, 0, EBUSY, NULL);
    }
#else
    p = p;
    rkey = rkey;
#endif
}

static int
reliable_send(trip_packet_t *p, int src, size_t len, void *buf)
{
    /* Toggle the src descriptor.
     * Pass the  buffer through to the one other connection.
     */
    // TODO fill in entry for next thingy shit fuck
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
    rel->c[0] = 0;
    rel->c[1] = 0;
    rel->alen = 512;
    rel->len = 0;
    rel->buf = malloc(rel->alen);
    rel->timer = NULL;

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
    reliable_t *rel = p->data;
    free(rel->buf);
    free(p->data);
    free(p);
}



