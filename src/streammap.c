
#include "streammap.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

#include "util.h"



void
streammap_init(streammap_t *map, int max)
{
    *map = (streammap_t){ 0 };
    map->cap = max;
}

void
streammap_destroy(streammap_t *map)
{
    if (map->map)
    {
        free(map->map);
        *map = (streammap_t){ 0 };
    }
}

int
streammap_iter_beg(streammap_t * UNUSED(map))
{
    return 0;
}

int
streammap_iter_end(streammap_t *map)
{
    return map->cap;
}

bool
streammap_has_space(streammap_t *map)
{
    return map->size < map->cap;
}

int
streammap_add(streammap_t *map, _trip_stream_t *s)
{
    int code = 0;

    do
    {
        if (!map->free)
        {
            int max = map->cap;

            void *m = malloc(sizeof(_trip_stream_t *) * max);

            if (!m)
            {
                code = ENOMEM;
                break;
            }

            map->map = m;

            int i = 0;
            map->cap = map->cap * 2;
            map->free = &map->map[i];
            for (; i < max; ++i)
            {
                map->map[i] = (_trip_stream_t *)&map->map[i + 1];
            }
        }

        /* There are free slots. */
        _trip_stream_t **slot = map->free;
        /* Advance free slot. */
        map->free = *((_trip_stream_t **)map->free);
        /* Set index. */
        int index = (int)((slot) - map->map);
        s->id = index;
        *slot = s;
        /* Increment size. */
        ++map->size;
    } while (false);

    return code;
}

_trip_stream_t *
streammap_del(streammap_t *map, int index)
{
    if (LIKELY(index < map->cap))
    {
        _trip_stream_t *s = map->map[index];
        _trip_stream_t **slot = &map->map[index];
        if (map->free)
        {
            *slot = (_trip_stream_t *)map->free;
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

        return s;
    }

    return NULL;
}

_trip_stream_t *
streammap_get(streammap_t *map, int index)
{
    if (LIKELY(index < map->cap))
    {
        _trip_stream_t *s = map->map[index];
        return s;
    }

    return NULL;
}

