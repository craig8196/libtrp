/*******************************************************************************
 * Copyright (c) 2019 Craig Jacobson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ******************************************************************************/
#include "libtrp.h"
#include "_libtrp.h"

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


typedef struct _trip_udp_context_s
{
    trip_memory_t *m;
    char *info;
    int fd;
} _trip_udp_context_t;

int
_trip_udp_af_to_pf(int ai_family)
{
    if (AF_INET == ai_family)
    {
        return PF_INET;
    }
    else
    {
        return PF_INET6;
    }
}

static void
_trip_udp_bind(void *_c)
{
    _trip_udp_context_t *c = _c;

    struct addrinfo hints = (struct addrinfo){ 0 };
    struct addrinfo *servinfo = NULL;
    
    do
    {
        /* Either of IPv4 or IPv6. */
        hints.ai_family = AF_UNSPEC;
        /* Packet/datagram socket. */
        hints.ai_socktype = SOCK_DGRAM;
        /* Auto-select IP. */
        hints.ai_flags = AI_PASSIVE;

        int status = getaddrinfo(NULL, c->info, &hints, &servinfo);
        if (status)
        {
            // TODO call trip with error
            break;
        }
        
        int protocol = 0;
        struct protoent *proto = getprotobyname("udp");
        if (proto)
        {
            protocol = proto->p_proto;
            endprotoent();
            proto = NULL;
        }

        struct addrinfo *choice = NULL;
        struct addrinfo *p = NULL;
        for (p = servinfo; p; p = p->ai_next)
        {
            if (SOCK_DGRAM == p->ai_socktype && protocol == p->ai_protocol)
            {
                if (p->ai_family == AF_INET)
                {
                    /* IPv4 */
                    if (!choice)
                    {
                        choice = p;
                    }
                }
                else
                {
                    /* IPv6 */
                    if (!choice)
                    {
                        choice = p;
                    }
                }
            }
        }

        if (!choice)
        {
            // TODO report error to router
            break;
        }

        int s = socket(_trip_udp_af_to_pf(choice->ai_family),
                       choice->ai_socktype,
                       choice->ai_protocol);

        if (s < 0)
        {
            // TODO report error to router
            break;
        }

        int err = bind(s, choice->ai_addr, choice->ai_addrlen);
        if (err < 0)
        {
            // TODO report error to router
            break;
        }

        c->fd = s;

        // TODO am I done?
        // TODO report success to router

    } while (0);

    if (servinfo)
    {
        freeaddrinfo(servinfo);
    }
    //TODO move notifying router down here
}

trip_packet_t *
trip_packet_new_udp(trip_memory_t *m, const unsigned char *info)
{
    if (!m)
    {
        m = trip_memory_default();
    }

    int e = 0;
    trip_packet_t *p = NULL;

    do
    {
        p = (trip_packet_t *)m->alloc(m->ud, sizeof(trip_packet_t *));

        if (!p)
        {
            e = ENOMEM;
            break;
        }

        if (!info)
        {
            info = (const unsigned char *)"42423";
        }

        size_t len = strlen((const char *)info);
        char *s = m->alloc(m->ud, len + 1);
        if (!s)
        {
            e = ENOMEM;
            m->free(m->ud, p);
            break;
        }
        
        memcpy(s, info, len + 1);

        _trip_udp_context_t *c = m->alloc(m->ud, sizeof(_trip_udp_context_t));

        if (!c)
        {
            e = ENOMEM;
            m->free(m->ud, s);
            m->free(m->ud, p);
            break;
        }

        c->m = m;
        c->info = s;
        c->fd = 0;
        p->ud = c;

        p->bind = _trip_udp_bind;
    } while (0);

    if (e)
    {
        if (p)
        {
            m->free(m->ud, p);
        }
        p = NULL;
    }

    return p;
}

