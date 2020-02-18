
#include "libtrp.h"
#include "_libtrp.h"
#include <errno.h>


enum _tripr_state
{
    _TRIPR_STATE_START,
    _TRIPR_STATE_STOP,
    _TRIPR_STATE_BIND,
    _TRIPR_STATE_LISTEN,
    _TRIPR_STATE_NOTIFY,
    _TRIPR_STATE_CLOSE,
    _TRIPR_STATE_END,
    _TRIPR_STATE_ERROR,
};


typedef struct _trip_router_s
{
    int state;
    int error;
    char *errmsg;
    trip_memory_t *mem;
    trip_packet_t *pack;
    trip_keypair_t *pair;
    trip_signature_t *sig;
    uint32_t max_mem;
    uint32_t max_packet_mem;
    uint32_t max_conn;
    bool allow_in;
    bool allow_out;
    bool allow_plain_open;
    bool allow_plain_seg;
    bool allow_plain_sig;
    uint32_t time_bind;
    uint32_t time_close;
    uint32_t time_stop;
    /* User provided hooks. */
    void *ud;
    trip_screen_t *screen;
} _trip_router_t;


trip_router_t *
trip_router_new_preset(enum trip_preset preset, trip_memory_t *m, trip_packet_t *p)
{
    if (!m)
    {
        m = trip_memory_default();
    }

    if (!p)
    {
        p = trip_packet_new_udp(m, NULL);
    }

    _trip_router_t *r = m->alloc(m->ud, sizeof(_trip_router_t));

    do
    {
        if (!r)
        {
            break;
        }


        switch (preset)
        {
            case TRIP_PRESET_SERVER:
                break;
            case TRIP_PRESET_CLIENT:
                break;
            default:
                (*r) = (const _trip_router_t){ 0 };
                break;
        }

        r->state = _TRIPR_STATE_START;
        r->mem = m;
        r->pack = p;
        // TODO
    } while (false);


    return (trip_router_t *)r;
}

int
trip_router_perform(trip_router_t *_r)
{
    _trip_router_t *r = (_trip_router_t *)_r;

    switch (r->state)
    {
        case _TRIPR_STATE_BIND:
            break;

        case _TRIPR_STATE_END:
            return EINVAL;
        default:
            break;
    }

    return 0;
}

