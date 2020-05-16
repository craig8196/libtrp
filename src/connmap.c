
#include "connmap.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

#include "util.h"



void
connmap_init(connmap_t *map, int max)
{
    *map = (connmap_t){ 0 };
    map->mask = near_pwr2(max);
}

void
connmap_destroy(connmap_t *map)
{
    if (map->map)
    {
        free(map->map);
        *map = (connmap_t){ 0 };
    }
}

static int
connmap_max(connmap_t *map)
{
    return (int)map->mask + 1;
}

static uint64_t
connmap_random()
{
    return 0;
}

int
connmap_iter_beg(connmap_t * UNUSED(map))
{
    return 0;
}

int
connmap_iter_end(connmap_t *map)
{
    return map->cap;
}

int
connmap_add(connmap_t *map, _trip_connection_t *conn)
{
    int code = 0;

    do
    {
        if (!free)
        {
            /* Need more free. */
            int max = connmap_max(map);

            /* Check if map is at max. */
            if (map->size >= max)
            {
                code = ENOSPC;
                break;
            }

            /* Check if map exists. */
            if (map->map)
            {
                void *m = realloc(map->map, sizeof(_trip_connection_t *) * (map->cap * 2));
                
                if (!m)
                {
                    code = ENOMEM;
                    break;
                }

                int i = map->cap;
                map->cap = map->cap * 2;
                map->free = &map->map[i];
                for (; i < (map->cap - 2); ++i)
                {
                    map->map[i] = (_trip_connection_t *)&map->map[i + 1];
                }

                map->map = m;
            }
            else
            {
                void *m = malloc(sizeof(_trip_connection_t *));

                if (!m)
                {
                    code = ENOMEM;
                    break;
                }

                map->map = m;
                map->cap = 1;
                map->free = &map->map[0];
            }
        }

        /* There are free slots. */
        _trip_connection_t **slot = map->free;
        /* Advance free slot. */
        map->free = *((_trip_connection_t **)map->free);
        /* Create ID. */
        uint64_t index = (slot) - map->map;
        uint64_t r = connmap_random();
        uint64_t id = index ^ (r & ~map->mask);
        conn->id = id;
        *slot = conn;
        /* Increase size. */
        ++map->size;


    } while (false);

    return code;
}

_trip_connection_t *
connmap_del(connmap_t *map, uint64_t id)
{
    int index = (int)(map->mask & id);
    if (index < map->cap)
    {
        _trip_connection_t *c = map->map[index];
        if (id == c->id)
        {
            _trip_connection_t **slot = &map->map[index];
            if (map->free)
            {
                *slot = (_trip_connection_t *)map->free;
            }
            else
            {
                *slot = NULL;
            }

            map->free = (void *)slot;
            --map->size;

            if (!map->size)
            {
                free(map->map);
                map->map = NULL;
            }

            return c;
        }
    }

    return NULL;
}

_trip_connection_t *
connmap_get(connmap_t *map, uint64_t id)
{
    int index = (int)(map->mask & id);
    if (LIKELY(index < map->cap))
    {
        _trip_connection_t *c = map->map[index];
        if (LIKELY(id == c->id))
        {
            return c;
        }
        else
        {
            return NULL;
        }
    }

    return NULL;
}

