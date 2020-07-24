
#include "sendq.h"

#include "util.h"


void
sendq_init(sendq_t *q)
{
    q->head = NULL;
    q->tail = NULL;
}

void
sendq_destroy(sendq_t * UNUSED(q))
{
}

_trip_connection_t *
sendq_dq(sendq_t *q)
{
    _trip_connection_t *c = q->head;

    if (c)
    {
        q->head = c->next;
        if (!q->head)
        {
            q->tail = NULL;
        }
        c->next = NULL;
    }

    return c;
}

void
sendq_nq(sendq_t *q, _trip_connection_t *c)
{
    if (q->tail)
    {
        q->tail->next = c;
        q->tail = c;
    }
    else
    {
        q->head = c;
        q->tail = c;
    }
    c->next = NULL;
}

