
#include "resolveq.h"

#include <string.h>

#include "util.h"
#include "libtrp_memory.h"


void
resolveq_init(resolveq_t *q)
{
    memset(q, 0, sizeof(resolveq_t));
}

void
resolveq_destroy(resolveq_t *q)
{
    tripm_cfree(q->map);
    memset(q, 0, sizeof(resolveq_t));
}

void
resolveq_clear(resolveq_t *q)
{
    resolveq_destroy(q);
    resolveq_init(q);
}

static void
_enqueue(resolveq_t *q, resolveq_union_t *el)
{
    el->inuse = false;
    el->u.next = q->free;
    q->free = el;
}

static void
_grow(resolveq_t *q)
{
    if (NULL == q->map)
    {
        ++q->len;
        q->map = tripm_alloc(sizeof(resolveq_union_t));
        if (NULL == q->map)
        {
            --q->len;
            return;
        }
    }
    else
    {
        ++q->len;
        void *tmpmap = tripm_realloc(q->map, sizeof(resolveq_union_t) * q->len);
        if (NULL == tmpmap)
        {
            --q->len;
            return;
        }
        q->map = tmpmap;
    }

    resolveq_union_t *last = &q->map[q->len - 1];
    _enqueue(q, last);
}

static resolveq_union_t *
_pop(resolveq_t *q, int *keyout)
{
    resolveq_union_t *el = q->free;
    q->free = el->u.next;
    *keyout = el - q->map;
    el->inuse = true;
    return el;
}

static void
_check_size(resolveq_t *q)
{
    if (!q->size)
    {
        resolveq_clear(q);
    }
}

int
resolveq_put(resolveq_t *q, _trip_connection_t *c)
{
    if (NULL == q->free)
    {
        _grow(q);
        if (NULL == q->free)
        {
            return -1;
        }
    }

    int index = 0;
    resolveq_union_t *el = _pop(q, &index);
    ++q->size;
    el->u.data = c;
    el->inuse = true;
    return index;
}

/**
 * Free up the entry and return the data.
 */
_trip_connection_t *
resolveq_pop(resolveq_t *q, int key)
{
    _trip_connection_t *c = NULL;

    if (key < q->len)
    {
        resolveq_union_t *el = &q->map[key];

        if (el->inuse)
        {
            c = el->u.data;
            _enqueue(q, el);
            --q->size;
        }
        else
        {
            /* Not in use, return NULL. */
        }

        _check_size(q);
    }

    return c;
}

/**
 * Nullify the data for the given entry.
 */
void
resolveq_del(resolveq_t *q, int key)
{
    if (key < q->len)
    {
        resolveq_union_t *el = &q->map[key];
        
        if (el->inuse)
        {
            el->u.data = NULL;
        }
    }
}

