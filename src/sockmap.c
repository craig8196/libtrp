
#include "sockmap.h"

#include <errno.h>
#include <limits.h>
#include <string.h>

#include "libtrp_memory.h"

#define SOCKMAP_MAX ((INT_MAX) - 1)

void
sockmap_init(sockmap_t *map)
{
    memset(map, 0, sizeof(sockmap_t));
}

void
sockmap_destroy(sockmap_t *map)
{
    if (map->map)
    {
        tripm_free(map->map);
        map->map = NULL;
    }
}

/**
 * Update or add the association fd->data.
 * @return Zero on success; ENOMEM otherwise.
 */
int
sockmap_put(sockmap_t *map, trip_socket_t fd, void *data)
{
    /* Search for existing entry. */
    int i;
    for (i = 0; i < map->len; ++i)
    {
        if (fd == map->map[i].fd)
        {
            map->map[i].data = data;
            return 0;
        }
    }

    if (map->alen <= map->len)
    {
        /* Check if we've reached our max limit. */
        if (SOCKMAP_MAX == map->alen)
        {
            return ENOMEM;
        }

        /* Allocate new map. */
        sockmap_entry_t *nmap = 
            map->map ?
            tripm_realloc(map->map, sizeof(sockmap_entry_t) * (map->len + 1)) :
            tripm_alloc(sizeof(sockmap_entry_t));
        if (!nmap)
        {
            return ENOMEM;
        }
        map->map = nmap;
        map->alen = map->len + 1;
    }

    /* Add the new entry. */
    map->map[map->len].fd = fd;
    map->map[map->len].data = data;
    ++map->len;
    return 0;
}

/**
 * Remove the association from the map.
 */
void
sockmap_del(sockmap_t *map, trip_socket_t fd)
{
    int i;
    for (i = 0; i < map->len; ++i)
    {
        if (fd == map->map[i].fd)
        {
            --map->len;
            map->map[i].fd = map->map[map->len].fd;
            map->map[i].data = map->map[map->len].data;
            map->map[map->len].fd = 0;
            map->map[map->len].data = NULL;
            return;
        }
    }
}

/**
 * Get the associated data for the given socket descriptor.
 */
void *
sockmap_get(sockmap_t *map, trip_socket_t fd)
{
    int i;
    for (i = 0; i < map->len; ++i)
    {
        if (fd == map->map[i].fd)
        {
            return map->map[i].data;
        }
    }
    return NULL;
}

