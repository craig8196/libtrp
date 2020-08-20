
#include "sendq.h"

#include "util.h"


void
sendq_init(sendq_t *q)
{
    q->head = NULL;
    q->tail = NULL;
}

void
sendq_destroy(sendq_t * q)
{
    sendq_init(q);
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
        c->insend = false;
    }

    return c;
}

void
sendq_nq(sendq_t *q, _trip_connection_t *c)
{
    if (c->insend)
    {
        return;
    }

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
    c->insend = true;
}

void
sendq_del(sendq_t *q, _trip_connection_t *c)
{
    if (c->insend)
    {
        _trip_connection_t **p = &q->head;
        _trip_connection_t *n = q->head;
        while (n)
        {
            if (n == c)
            {
                /* Unlink connection. */
                *p = n->next;
                break;
            }

            p = &n->next;
            n = n->next;
        }
    }
    c->insend = false;
}

bool
sendq_has(sendq_t *q)
{
    return NULL != q->head;
}

