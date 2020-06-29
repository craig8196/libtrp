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
 * @file trip.c
 * @author Craig Jacobson
 * @brief Core code for handling routers, connections, and streams.
 */
#include "libtrp.h"
#include "libtrp_memory.h"

#include "core.h"
#include "trip.h"
#include "conn.h"
#include "message.h"
#include "time.h"
#include "protocol.h"
#include "pack.h"

#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <sys/epoll.h>


_trip_msg_t *
_trip_msg_new()
{
    // TODO check internal queue first
    return tripm_alloc(sizeof(_trip_msg_t));
}

void
_trip_msg_free(_trip_msg_t *msg)
{
    // TODO enqueue
    tripm_free(msg);
}

_trip_stream_t *
_trip_stream_new(_trip_router_t *r)
{
    return tripm_alloc(sizeof(_trip_stream_t));
}

trip_stream_t *
tripc_open_stream(trip_connection_t *_c, int sid, int opts)
{
    trip_toconn(c, _c);

    _trip_stream_t *s = _trip_stream_new(c->router);

    if (s)
    {
        s->ud = NULL;
        s->connection = c;
        s->id = sid;
        s->flags = opts & _TRIPS_OPT_PUBMASK;
        s->listbeg = NULL;
        s->listend = NULL;

        // TODO check that connection has list available otherwise stall and place on wait queue

    }

    return (trip_stream_t *)s;
}

void
_trip_set_error(_trip_router_t *r, int eval, const char *msg)
{
    size_t mlen = strlen(msg ? msg : "");
    r->error = eval;
    _trip_set_state(r, _TRIPR_STATE_ERROR);
    r->errmsg = tripm_alloc(mlen + 1);
    if (r->errmsg)
    {
        memcpy(r->errmsg, msg, mlen);
        r->errmsg[mlen] = 0;
    }
}

_trip_connection_t *
_trip_send_dq(_trip_router_t *r)
{
    _trip_connection_t *c = r->sendbeg;

    if (c)
    {
        r->sendbeg = c->next;
        if (!r->sendbeg)
        {
            r->sendend = NULL;
        }
    }

    return c;
}

void
_trip_send_nq(_trip_router_t *r, _trip_connection_t *c)
{
    if (r->sendend)
    {
        r->sendend->next = c;
        r->sendend = c;
    }
    else
    {
        r->sendbeg = c;
        r->sendend = c;
    }
}

void
_trip_listen(_trip_router_t *r, trip_socket_t fd, int events)
{
    /* Let the packet interface read. */
    r->pack->read(r->pack->ud, fd, events, r->max_packet_read_count);

    /* Rate limit number of send. */
    _trip_connection_t *c = _trip_send_dq(r);
    trip_packet_t *p = r->pack;
    while (c)
    {
        size_t len;
        void *buf;
        int code = _tripc_send(c, buf);
        if (!code)
        {
            int error = p->send(p->ud, len, buf);

            if (error)
            {
                // TODO handle send error
                break;
            }

            _trip_send_nq(r, c);
        }
        else
        {
            // TODO check connection write error code
            /* Don't re-enqueue. */
        }

        c = _trip_send_dq(r);
    }

    if (TRIP_SOCKET_TIMEOUT == fd)
    {
        // TODO handle connection timeouts
    }
}

void
_trip_notify(_trip_router_t *r)
{
    r = r;
    /** TODO iterate over connections
    size_t i;
    for (i = 0; i < r->concap; ++i)
    {
        _trip_connection_t *c = r->con[i];
        _tripc_notify(c);
    }
    */
}

void
_trip_close(_trip_router_t *r)
{
    /* TODO
    size_t i;
    for (i = 0; i < r->concap; ++i)
    {
        _trip_connection_t *c = r->con[i];
        tripc_close((trip_connection_t *)c);
        tripm_free(c);
    }
    */

    connmap_destroy(&r->con);
}

const char *
trip_errmsg(trip_router_t *_r)
{
    trip_torouter(r, _r);
    return r->errmsg ? r->errmsg : "";
}

int
trip_setopt(trip_router_t *_r, enum trip_router_opt opt, ...)
{
	va_list ap;

    trip_torouter(r, _r);
    int rval = 0;

	va_start(ap, opt);

    switch (opt)
    {
        case TRIPOPT_OPEN_KP:
            r->openpub = va_arg(ap, unsigned char *);
            r->opensec = va_arg(ap, unsigned char *);
            break;
        case TRIPOPT_SIGN_KP:
            r->signpub = va_arg(ap, unsigned char *);
            r->signsec = va_arg(ap, unsigned char *);
            break;
        case TRIPOPT_USER_DATA:
            r->ud = va_arg(ap, void *);
            break;
        case TRIPOPT_WATCH_CB:
            r->watch = va_arg(ap, trip_handle_watch_t);
            break;
        case TRIPOPT_TIMEOUT_CB:
            r->timeout = va_arg(ap, trip_handle_timeout_t);
            break;
        case TRIPOPT_SCREEN_CB:
            r->screen = va_arg(ap, trip_handle_screen_t);
            break;
        case TRIPOPT_CONNECTION_CB:
            r->connection = va_arg(ap, trip_handle_connection_t);
            break;
        case TRIPOPT_STREAM_CB:
            r->stream = va_arg(ap, trip_handle_stream_t);
            break;
        case TRIPOPT_MESSAGE_CB:
            r->message = va_arg(ap, trip_handle_message_t);
            break;
        default:
            rval = EINVAL;
            break;
    }

    va_end(ap);

    return rval;
}

static void
_trip_router_reject(_trip_router_t *r, int src, int reason)
{
}

static bool
_trip_router_is_reported(_trip_router_t *r, int src)
{
    return false;
}

_trip_connection_t *
_trip_router_get_by_address(_trip_router_t *r, int src)
{
    return NULL;
}

#if 0
/**
 * @brief Expand the connection array size.
 * @return Zero on success.
 */
int
_trip_expand_connections(_trip_router_t *r)
{
    if (!r->con)
    {
        r->con = tripm_alloc(sizeof(_trip_connection_t *));
    
        if (r->con)
        {
            r->con[0] = tripm_alloc(sizeof(_trip_connection_t));

            if (r->con[0])
            {
                _tripc_init(r->con[0], r, 0);
                r->concap = 1;
                r->conlen = 0;
                r->confree = r->con[0];
            }
            else
            {
                tripm_free(r->con);
                r->con = NULL;
                return ENOMEM;
            }
        }
        else
        {
            return ENOMEM;
        }
    }
    else
    {
        if (r->concap < 0x80000000)
        {
            /* Expand the array. */
            r->concap <<= 1;
            void *con = trip_realloc(r->con, r->concap * sizeof(_trip_connection_t *));

            if (con)
            {
                r->conmask = r->concap - 1;
                r->con = con;
                r->conempty = &r->con[r->concap >> 1];
            }
            else
            {
                r->concap >>= 1;
                return ENOMEM;
            }

        }
        else
        {
            return ENOSPC;
        }
    }

    return 0;
}
#endif

_trip_connection_t *
_trip_new_connection()
{
    return tripm_alloc(sizeof(_trip_connection_t));
}

static void
_trip_segment(_trip_router_t *r, int src, size_t len, unsigned char *buf)
{
    /* Discard too short segments immediately. */
    if (len < 16)
    {
        // TODO add reporting codes
        _trip_router_reject(r, src, 0);
        return;
    }

    /* Check if the source is flagged for misbehavior. */
    if (_trip_router_is_reported(r, src))
    {
        // TODO should this be a feature???
        return;
    }

    /* Unpack the prefix information. */
    _trip_prefix_t prefix;
    int end = trip_unpack(len, buf, "OVV",
                          &prefix.control,
                          &prefix.id,
                          &prefix.seq);

    /* Discard if unpack failed. */
    if (end <= 0)
    {
        _trip_router_reject(r, src, 1);
        return;
    }

    /* Extract encrypted flag. */
    prefix.encrypted = prefix.control & _TRIP_PREFIX_EMASK;
    prefix.control = prefix.control & (~_TRIP_PREFIX_EMASK);

    /* Check for valid control. */
    if (prefix.control >= _TRIP_CONTROL_MAX)
    {
        _trip_router_reject(r, src, 12);
        return;
    }

    /* Special treatment for OPEN requests. */
    if (_TRIP_CONTROL_OPEN == prefix.control)
    {
        /* Check if incoming requests are allowed. */
        if (!r->allow_in)
        {
            _trip_router_reject(r, src, 2);
            return;
        }

        /* Check if encryption is required. */
        if (!prefix.encrypted && !r->allow_plain_open)
        {
            _trip_router_reject(r, src, 3);
            return;
        }

        /* Unpack version and routing information. */
        uint16_t version = -1;
        _trip_route_t route;
        int end2 = trip_unpack(len - end, buf + end, "HVp",
                               &version,
                               &route.len,
                               &route.route);

        /* Discard if unpack failed. */
        if (end2 <= 0)
        {
            _trip_router_reject(r, src, 4);
            return;
        }

        /* Discard if we don't support the version number. */
        if (version > 0)
        {
            _trip_router_reject(r, src, 5);
            return;
        }

        /* Find the connection if it exists.
         * Create if it doesn't exist.
         */
        _trip_connection_t *c = _trip_router_get_by_address(r, src);
        if (!c)
        {
            c = _trip_new_connection(r);
            if (!c)
            {
                _trip_router_reject(r, src, 6);
                return;
            }
        }

        /* Discard if this sequence has been seen. */
        int seen = _tripc_check_open_seq(c, prefix.seq);
        if (seen)
        {
            _trip_router_reject(r, src, 20);
            return;
        }

        /* Pass information to the user.
         * User is allowed to view source of incoming request.
         * User is allowed to view routing buffer.
         * User is allowed to specify open decryption keys.
         * User is allowed to create user data.
         */
        // TODO user validation

        /* Decrypt OPEN buffer. */
        // TODO


        /* Flag the sequence now that validation has taken place. */
        _tripc_flag_open_seq(c, prefix.seq);

        if (!seen)
        {
            _tripc_seg(c, prefix.control, 0, NULL);
        }
        else
        {
            _trip_router_reject(r, src, 16);
            return;
        }
    }
    else
    {
        if (!prefix.encrypted && !r->allow_plain_seg)
        {
            /* Not encrypted when required. */
            _trip_router_reject(r, src, 7);
            return;
        }

        _trip_connection_t *c = connmap_get(&r->con, prefix.id);
        if (c)
        {
            /* TODO
            _tripc_handle_segment(c, src, len - end, buf + end);
            */
            // TODO handle error code
        }
        else
        {
            _trip_router_reject(r, src, 8);
            return;
        }
    }
}

void
_trip_set_state(_trip_router_t *r, enum _tripr_state state)
{
    /* When entering the state, do this. */
    switch (state)
    {
        case _TRIPR_STATE_LISTEN:
            {
                r->statedeadline = 0;
            }
            break;
        case _TRIPR_STATE_NOTIFY:
            {
                /* Possible next states: ERROR, NOTIFY.
                 *
                 * Perform notify once.
                 */
                if (r->timeout_notify > 0)
                {
                    _trip_notify(r);
                    r->statedeadline = triptime_deadline(r->timeout_notify);
                    r->timeout(r->ud, r->timeout_notify);
                }
            }
            break;
        default:
            break;
    }

    r->state = state;
}

void
trip_seg(trip_router_t *_r, int src, size_t len, void *buf)
{
    trip_torouter(r, _r);

    if (_TRIPR_STATE_LISTEN == r->state || _TRIPR_STATE_NOTIFY == r->state)
    {
        _trip_segment(r, src, len, (unsigned char *)buf);
    }
}

void
trip_ready(trip_router_t *_r)
{
    trip_torouter(r, _r);

    if (_TRIPR_STATE_BIND == r->state)
    {
        _trip_set_state(r, _TRIPR_STATE_LISTEN);
    }
    else
    {
        _trip_set_state(r, _TRIPR_STATE_ERROR);
    }
}

/*
 * TODO what was the point of this?? callback for packet to use?
void
trip_resolve(trip_router_t *_r, trip_connection_t *_c, int err)
{
    trip_torouter(r, _r);
    trip_toconn(c, _c);

    if (!err)
    {
        _tripc_start(c);
    }
    else
    {
        c->error = err;
        c->state = _TRIPR_STATE_ERROR;
        r->connection(r->ud, (trip_connection_t *)c);
        _tripc_emptyq_add(r, c);
    }
}
*/

void
trip_watch(trip_router_t *_r, trip_socket_t fd, int events)
{
    trip_torouter(r, _r);

    r->watch(_r, fd, events);
}

void
trip_unready(trip_router_t *_r, int err)
{
    trip_torouter(r, _r);

    r->error = err;
    _trip_set_state(r, _TRIPR_STATE_UNBIND);
}

trip_router_t *
trip_new(enum trip_preset preset, trip_packet_t *p)
{
    if (!p)
    {
        
        p = trip_packet_new_udp(NULL);
        if (!p)
        {
            return NULL;
        }
    }

    _trip_router_t *r = tripm_alloc(sizeof(_trip_router_t));

    do
    {
        if (!r)
        {
            break;
        }


        switch (preset)
        {
            case TRIP_PRESET_SERVER:
                break;
            case TRIP_PRESET_CLIENT:
                break;
            default:
                (*r) = (const _trip_router_t){ 0 };
                break;
        }

        _trip_set_state(r, _TRIPR_STATE_START);
        r->pack = p;
        // TODO
    } while (false);


    return (trip_router_t *)r;
}

int
_trip_verify(_trip_router_t *r)
{
    int code = 0;
    const char *msg = NULL;

    do
    {
        if (!r->watch
            || !r->timeout
            || !r->connection
            || !r->stream
            || !r->message)
        {
            code = EINVAL;
            msg = "Required method missing. See documentation.";
        }
    } while (false);

    if (code)
    {
        _trip_set_error(r, code, msg);
    }

    return code;
}

int
trip_action(trip_router_t *_r, trip_socket_t fd, int events)
{
    _trip_router_t *r = (_trip_router_t *)_r;

    switch (r->state)
    {
        case _TRIPR_STATE_START:
            {
                /* Possible next states: ERROR, BIND.
                 *
                 * If the fd is NOT TRIP_SOCKET_TIMEOUT, set error.
                 *
                 * Validate that the router is setup properly.
                 * Set state to bind.
                 * Call bind on packet type.
                 * Check for error or change in state after call,
                 * the call may be synchronous or asynchronous.
                 * If error, exit.
                 * If state changed to LISTEN, then set timeout to zero.
                 * If state is the same, then set timeout according to settings.
                 * State should not be changed to anything else without
                 * the error code being set.
                 */
                if (TRIP_SOCKET_TIMEOUT != fd)
                {
                    _trip_set_error(r, EINVAL, "Invalid descriptor for state START.");
                    break;
                }

                if (_trip_verify(r))
                {
                    /* Error already set. */
                    break;
                }

                r->pack->router = _r;

                _trip_set_state(r, _TRIPR_STATE_BIND);
                r->pack->bind(r->pack->ud);

                if (r->error)
                {
                    break;
                }

                if (_TRIPR_STATE_LISTEN == r->state)
                {
                    r->statedeadline = triptime_deadline(0);
                    r->timeout(r->ud, 0);
                }
                else if (_TRIPR_STATE_BIND == r->state)
                {
                    r->statedeadline = triptime_deadline(r->timeout_bind);
                    r->timeout(r->ud, r->timeout_bind);
                }
                else if (_TRIPR_STATE_ERROR == r->state)
                {
                    /* Do nothing. */
                }
                else
                {
                    _trip_set_error(r, EINVAL, "Invalid state.");
                }
            }
            break;
        case _TRIPR_STATE_BIND:
            {
                /* Possible next states: ERROR, LISTEN.
                 *
                 * If TRIP_SOCKET_TIMEOUT is NOT set, set error.
                 *
                 * Get the current time.
                 * Check the deadline.
                 * If we're past the deadline, set error.
                 * Else recalculate the timeout.
                 */
                if (TRIP_SOCKET_TIMEOUT != fd)
                {
                    _trip_set_error(r, EINVAL, "Invalid descriptor for state BIND.");
                    break;
                }

                uint64_t now = triptime_now();
                if (now > r->statedeadline)
                {
                    _trip_set_error(r, ETIME, "Bind timeout.");
                }
                else
                {
                    int timeout = triptime_timeout(r->statedeadline, now);
                    r->timeout(r->ud, timeout);
                }
            }
            break;
        case _TRIPR_STATE_LISTEN:
            {
                /* Possible next states: ERROR, NOTIFY.
                 *
                 * If timeout, test deadline, try to read and send data.
                 * If fd, verify and check read and send for that item.
                 */
                _trip_listen(r, fd, events);
            }
            break;
        case _TRIPR_STATE_NOTIFY:
            {
                /* Possible next states: ERROR, CLOSE, END.
                 *
                 * Perform same duties as LISTEN.
                 */
                _trip_listen(r, fd, events);

                uint64_t now = triptime_now();
                if (now >= r->statedeadline)
                {
                    _trip_set_state(r, _TRIPR_STATE_CLOSE);
                }
                else
                {
                    /* Continue waiting. */
                }
            }
            break;
        case _TRIPR_STATE_CLOSE:
            {
                /* Possible next states: ERROR, END.
                 *
                 * Perform close only
                 */
                _trip_close(r);
                _trip_set_state(r, _TRIPR_STATE_END);
            }
            break;
        case _TRIPR_STATE_END:
            _trip_set_error(r, ESHUTDOWN, "Router is shutdown.");
            break;
        case _TRIPR_STATE_ERROR:
            break;
        default:
            r->error = EINVAL;
            break;
    }

    return r->error;
}

void
trip_open_connection(trip_router_t *_r, void *ud, size_t ilen,
                     const unsigned char *info)
{
    trip_torouter(r, _r);

    if (r->allow_out)
    {
        _trip_connection_t *c = _trip_new_connection(r);
        // TODO acquire connection and ID...
        r->pack->resolve(r->pack->ud, (trip_connection_t *)c);
    }
    else
    {
        // TODO how to fail gracefully... temporary connection in error state...
        //r->connection(r->ud, NULL, info, ud, false);
    }
}

int
trip_start(trip_router_t *_r)
{
    trip_torouter(r, _r);
    return trip_action(_r, TRIP_SOCKET_TIMEOUT, 0);
}

int
_trip_fd_events_to_epoll(int events)
{
    int eout = 0;

    switch (events)
    {
        case TRIP_REMOVE:
            break;
        case TRIP_IN:
        case TRIP_OUT:
        case TRIP_INOUT:
            if (events != TRIP_IN)
            {
                eout |= EPOLLOUT;
            }
            if (events != TRIP_OUT)
            {
                eout |= EPOLLIN;
            }
            break;
        default:
            break;
    }

    return eout;
}

void
_trip_watch_cb(trip_router_t *_r, int fd, int events)
{
    trip_torouter(r, _r);
    _trip_wait_t *w = r->wait;

    int c;
    struct epoll_event ev = { 0 };

    ev.events = _trip_fd_events_to_epoll(events);
    ev.data.fd = fd;

    if (TRIP_REMOVE != events)
    {
        if (TRIP_ADD & events)
        {
            c = epoll_ctl(w->efd, EPOLL_CTL_ADD, fd, &ev);
        }
        else
        {
            c = epoll_ctl(w->efd, EPOLL_CTL_MOD, fd, &ev);
        }
    }
    else
    {
        c = epoll_ctl(w->efd, EPOLL_CTL_DEL, fd, &ev);
    }

    if (c)
    {
        printf("Error on epoll (%d): %s", c, strerror(c));
    }
}

void
_trip_timeout_cb(trip_router_t *_r, int timeout)
{
    trip_torouter(r, _r);
    _trip_wait_t *w = r->wait;
    w->timeout = timeout;
}

_trip_wait_t *
_trip_wait_new()
{
    _trip_wait_t *w = tripm_alloc(sizeof(_trip_wait_t));

    w->efd = epoll_create1(0);

    int c = 0;

    do
    {
        if (-1 == w->efd)
        {
            c = EINVAL;
            break;
        }
    } while (false);

    if (c)
    {
        tripm_free(w);
        w = NULL;
    }

    return w;
}

int
_trip_timeout(_trip_router_t *r, uint64_t now, uint64_t maxdeadline)
{
    // TODO

    return 1;
}

int
trip_run(trip_router_t *_r, int maxtimeout)
{
    trip_torouter(r, _r);

    int c = 0;

    do
    {
        if (!r->wait)
        {
            r->watch = _trip_watch_cb;
            r->timeout = _trip_timeout_cb;
            r->wait = _trip_wait_new();
            if (!r->wait)
            {
                r->watch = NULL;
                r->timeout = NULL;
                c = ENOMEM;
                break;
            }
        }

        _trip_wait_t *w = r->wait;

        uint64_t now = 0;
        uint64_t deadline = triptime_deadline(maxtimeout);
        int timeout = maxtimeout;
        
        for (;;)
        {
            struct epoll_event eventlist[_TRIP_MAX_EVENTS];
            int nfds = epoll_wait(w->efd, eventlist, _TRIP_MAX_EVENTS, timeout);

            if (-1 == nfds)
            {
                c = errno;
                break;
            }

            int n;
            for (n = 0; n < nfds; ++n)
            {
                int events = 0;
                int evs = eventlist[n].events;
                int fd = eventlist[n].data.fd;

                if (evs & EPOLLIN)
                {
                    events |= TRIP_IN;
                }

                if (evs & EPOLLOUT)
                {
                    events |= TRIP_OUT;
                }

                c = trip_action(_r, fd, events);
                
                if (c)
                {
                    break;
                }
            }

            if (c)
            {
                break;
            }

            now = triptime_now();

            if (now >= deadline)
            {
                break;
            }

            timeout = _trip_timeout(r, now, deadline);
        }
    } while (false);


    return c;
}

int
trip_stop(trip_router_t *_r)
{
    trip_torouter(r, _r);

    int c = 0;

    if (r->state <= _TRIPR_STATE_LISTEN)
    {
        _trip_set_state(r, _TRIPR_STATE_NOTIFY);
    }
    else
    {
        c = EINVAL;
    }

    return c;
}

void
trip_free(trip_router_t *_r)
{
    trip_torouter(r, _r);

    tripm_free(r);
}

