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
#include "libtrp_handles.h"

#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


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

/**
 * @see https://beej.us/guide/bgnet/html/
 */
static void
_trip_udp_bind(trip_packet_t *packet)
{
    _trip_udp_context_t *c = (_trip_udp_context_t *)packet->data;

    struct addrinfo hints = (struct addrinfo){ 0 };
    struct addrinfo *servinfo = NULL;
    int error = 0;
    const char *emsg = NULL;
    
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
            error = EINVAL; // TODO convert from getaddrinfo error to generic
            emsg = "Error from getaddrinfo."; // Use gai_strerror.
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
            error = EINVAL;
            emsg = "No valid address to bind to.";
            break;
        }

        /*
         * Since Linux 2.6.27 the *type* argument can be bitwise OR'd
         * with SOCK_NONBLOCK to save a call to fcntl.
         * @see https://man7.org/linux/man-pages/man2/socket.2.html
         */
        int s = socket(_trip_udp_af_to_pf(choice->ai_family),
                       choice->ai_socktype | SOCK_NONBLOCK,
                       choice->ai_protocol);
        if (s < 0)
        {
            error = errno;
            emsg = "Error trying to create socket.";
            break;
        }

        int err = bind(s, choice->ai_addr, choice->ai_addrlen);
        if (err < 0)
        {
            error = errno;
            emsg = "Error trying to bind socket.";
            /* Ignore further errors. */
            close(s);
            break;
        }

        c->fd = s;
    } while (0);

    if (servinfo)
    {
        freeaddrinfo(servinfo);
    }

    if (!error)
    {
        trip_ready(packet->router);
        trip_watch(packet->router,
            ((_trip_udp_context_t *)packet->data)->fd,
            TRIP_IN);
    }
    else
    {
        trip_error(packet->router, error, emsg);
    }
}

// TODO create resolve, send, read??

static void
_trip_udp_unbind(trip_packet_t *packet)
{
    _trip_udp_context_t *c = packet->data;
    int error = 0;
    if (c->fd >= 0)
    {
        if (close(c->fd) < 0)
        {
            error = errno;
        }
    }

    trip_unready(packet->router, error);
}

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

    // TODO eventually parse IP address to bind to... only port supported now.

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
        p = (trip_packet_t *)tripm_alloc(sizeof(trip_packet_t));
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
        p->data = c;
        p->bind = _trip_udp_bind;
        p->resolve = NULL;
        p->send = NULL;
        p->read = NULL;
        p->unbind = _trip_udp_unbind;
        p->router = NULL;
    } while (0);

    /* Handle error. */
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

void
trip_packet_free_udp(trip_packet_t *p)
{
    _trip_udp_context_t *c = p->data;

    if (c->info)
    {
        tripm_free(c->info);
    }
    
    tripm_free(c);
    tripm_free(p);
}

