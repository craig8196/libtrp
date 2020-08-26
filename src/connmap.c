
#include "connmap.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
// TODO switch to interface function for random generation
#include <sodium.h>

#include "libtrp_memory.h"
#include "util.h"


static uint64_t
connmap_max_conn(void)
{
    return (uint64_t)1 << 31;
}

static size_t
connmap_index_at(connmap_t *map, connmap_entry_t *e)
{
    return (size_t)(e - map->map);
}

static size_t
connmap_index(connmap_t *map, uint64_t id)
{
    return (size_t)(map->mask & id);
}

static bool
connmap_has_free(connmap_t *map)
{
    return NPOS != map->free;
}

static void
connmap_push(connmap_t *map, connmap_entry_t *e)
{
    size_t eindex = connmap_index_at(map, e);
    e->isfull = false;
    e->u.next = map->free;
    map->free = eindex;
}

static connmap_entry_t *
connmap_pop(connmap_t *map)
{
    connmap_entry_t *e = &map->map[map->free];
    map->free = e->u.next;
    return e;
}

static size_t
connmap_max(connmap_t *map)
{
    return (size_t)(map->mask + 1);
}

static uint64_t
connmap_random()
{
    uint64_t r = randombytes_random();
    r <<= 32;
    r ^= randombytes_random();
    return r;
}

void
connmap_init(connmap_t *map, uint64_t max)
{
    if (!max)
    {
        max = 1;
    }
    if (max > connmap_max_conn())
    {
        max = connmap_max_conn();
    }
    memset(map, 0, sizeof(connmap_t));
    map->mask = near_pwr2_64(max) - 1;
    map->free= NPOS;
}

void
connmap_destroy(connmap_t *map)
{
    map->map = tripm_cfree(map->map);
}

void
connmap_clear(connmap_t *map)
{
    uint64_t max = connmap_max(map);
    connmap_destroy(map);
    connmap_init(map, max);
}

size_t
connmap_iter_beg(connmap_t * UNUSED(map))
{
    return 0;
}

_trip_connection_t *
connmap_iter_get(connmap_t *map, size_t it)
{
    return map->map[it].isfull ? map->map[it].u.c : NULL;
}

size_t
connmap_iter_end(connmap_t *map)
{
    return map->cap;
}

/**
 * Add the connection to the map assigning it a unique ID.
 */
int
connmap_add(connmap_t *map, _trip_connection_t *conn)
{
    int code = 0;

    do
    {
        if (!connmap_has_free(map))
        {
            /* Need more free. */
            size_t max = connmap_max(map);

            /* Check if map is at max. */
            if (map->size >= max)
            {
                code = ENOSPC;
                break;
            }

            /* Check if map exists. */
            if (map->map)
            {
                void *m = tripm_realloc(map->map, sizeof(connmap_entry_t) * (map->cap * 2));
                
                if (!m)
                {
                    code = ENOMEM;
                    break;
                }

                map->map = m;
                size_t i = map->cap;
                map->cap = map->cap * 2;
                size_t len = map->cap;
                for (; i < len; ++i)
                {
                    connmap_push(map, &map->map[i]);
                }

            }
            else
            {
                void *m = tripm_alloc(sizeof(connmap_entry_t));

                if (!m)
                {
                    code = ENOMEM;
                    break;
                }

                map->map = m;
                map->cap = 1;
                connmap_push(map, map->map);
            }
        }

        /* There are free slots. */
        connmap_entry_t *e = connmap_pop(map);
        /* Create ID. */
        uint64_t index = connmap_index_at(map, e);
        uint64_t r = connmap_random();
        uint64_t id = index ^ (r & ~map->mask);
        conn->self.id = id;
        e->isfull = true;
        e->u.c = conn;
        /* Increase size. */
        ++map->size;
    } while (false);

    return code;
}

_trip_connection_t *
connmap_del(connmap_t *map, uint64_t id)
{
    size_t index = connmap_index(map, id);

    if (index < map->cap)
    {
        connmap_entry_t *e = &map->map[index];
        if (e->isfull && id == e->u.c->self.id)
        {
            _trip_connection_t *c = e->u.c;
            connmap_push(map, e);
            --map->size;

            if (!map->size)
            {
                connmap_clear(map);
            }

            return c;
        }
    }

    return NULL;
}

_trip_connection_t *
connmap_get(connmap_t *map, uint64_t id)
{
    size_t index = connmap_index(map, id);

    if (LIKELY(index < map->cap))
    {
        connmap_entry_t *e = &map->map[index];
        if (LIKELY(e->isfull && id == e->u.c->self.id))
        {
            return e->u.c;
        }
        else
        {
            return NULL;
        }
    }

    return NULL;
}

