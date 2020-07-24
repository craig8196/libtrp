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
/**
 * @file packet.c
 * @brief Implementation of the UDP packet interface.
 */
#include "libtrp.h"
#include "libtrp_memory.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


typedef struct _trip_udp_context_s
{
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
    // TODO validate this function and the procedure for binding
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

// TODO create unbind, resolve, send, read, wait??

static bool
_trip_udp_validate_info(const unsigned char *s)
{
    int i;
    for (i = 0; i < 5; ++i)
    {
        if (!s[i])
        {
            break;
        }

        if (!isdigit((char)s[i]))
        {
            return false;
        }
    }

    if (s[i])
    {
        return false;
    }

    int port = atoi((const char *)s);
    if (port > 65535 || port <= 0)
    {
        return false;
    }

    // TODO eventually parse IP address to bind to...

    return true;
}

trip_packet_t *
trip_packet_new_udp(const char *_info)
{
    int e = 0;
    trip_packet_t *p = NULL;
    const unsigned char *info = (const unsigned char *)_info;

    do
    {
        /* Validate info. */
        if (!info)
        {
            info = (const unsigned char *)"42423";
        }
        else if (!_trip_udp_validate_info(info))
        {
            e = EINVAL;
            break;
        }

        /* Allocate packet struct. */
        p = (trip_packet_t *)tripm_alloc(sizeof(trip_packet_t *));
        if (!p)
        {
            e = ENOMEM;
            break;
        }

        /* Copy info string. */
        size_t len = strlen((const char *)info);
        char *s = tripm_alloc(len + 1);
        if (!s)
        {
            e = ENOMEM;
            break;
        }
        memcpy(s, info, len + 1);

        /* Allocate context information. */
        _trip_udp_context_t *c = tripm_alloc(sizeof(_trip_udp_context_t));
        if (!c)
        {
            e = ENOMEM;
            tripm_free(s);
            break;
        }

        /* Fill structs. */
        c->info = s;
        c->fd = 0;
        p->ud = c;
        p->bind = _trip_udp_bind;
        p->resolve = NULL;
        p->send = NULL;
        p->read = NULL;
        p->unbind = NULL;
        p->wait = NULL;
        p->router = NULL;
    } while (0);

    if (e)
    {
        if (p)
        {
            tripm_free(p);
        }
        p = NULL;
    }

    return p;
}

