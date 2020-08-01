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
#include "sendq.h"

#include <errno.h>
#include <stdarg.h>
#include <string.h>


_trip_connection_t *
_trip_new_connection()
{
    return tripm_alloc(sizeof(_trip_connection_t));
}

void
_trip_free_connection(_trip_connection_t *c)
{
    tripm_free(c);
}

void
_trip_qconnection(_trip_router_t *r, _trip_connection_t *c)
{
    sendq_nq(&r->sendq, c);
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

void
_trip_timeout(_trip_router_t *r, int ms, bool forstate)
{
    if (forstate)
    {
        r->timeout((trip_router_t *)r, (long)ms);
    }
    else
    {
        if (ms >= 0)
        {
            /* Only allow timeouts shorter than the state deadline.
             * TODO should we save the deadline for non-state deadlines? Tricky.
             */
            uint64_t now = triptime_now();
            if ((now + ms) < r->statedeadline)
            {
                r->timeout((trip_router_t *)r, ms);
            }
        }
    }
}

void
_trip_listen(_trip_router_t *r, trip_socket_t fd, int events)
{
    if (TRIP_SOCKET_TIMEOUT == fd)
    {
        // TODO ??? check if actuall socket timeout or connection???
        return;
    }

    /* Let the packet interface read. */
    if (events & TRIP_IN)
    {
        r->packet->read(r->packet, fd, r->max_packet_read_count);
    }

    if (events & TRIP_OUT)
    {
        /* Rate limit number of send. */
        _trip_connection_t *c = sendq_dq(&r->sendq);
        trip_packet_t *p = r->packet;
        while (c)
        {
            // TODO get actual buffer? i think _tripc_send is populating the buffer
            size_t len = 0;
            void *buf = NULL;
            int code = _tripc_send(c, len, buf);
            if (!code)
            {
                int error = p->send(p->data, c->src, len, buf);

                if (error)
                {
                    // TODO handle send error
                    break;
                }

                sendq_nq(&r->sendq, c);
            }
            else
            {
                // TODO check connection write error code
                /* Don't re-enqueue. */
            }

            c = sendq_dq(&r->sendq);
        }
    }
}

void
_trip_close(_trip_router_t *r, int gracems)
{
    int i = connmap_iter_beg(&r->con);
    int end = connmap_iter_end(&r->con);
    while (i != end)
    {
        _trip_connection_t *c = connmap_iter_get(&r->con, i);
        tripc_close((trip_connection_t *)c, gracems);
        if (0 == gracems)
        {
            _trip_free_connection(c);
        }
        ++i;
    }

    connmap_clear(&r->con);
}

static void
_trip_router_reject(_trip_router_t *r, int src, int reason)
{
    // TODO figure out how we handle rejections
    r = r;
    src = src;
    reason = reason;
}

static bool
_trip_router_is_reported(_trip_router_t *r, int src)
{
    // TODO check data structure for reported source
    r = r;
    src = src;
    return false;
}

_trip_connection_t *
_trip_router_get_by_address(_trip_router_t *r, int src)
{
    // TODO get connection based on source
    r = r;
    src = src;
    return NULL;
}

/**
 * Decrypt the OPEN buffer.
 * @note The same buffer should be able to be used for inplace decryption.
 * @return Zero on success; errno otherwise.
 */
int
_trip_decrypt(_trip_router_t *r, size_t len, unsigned char *buf)
{
    // TODO
    r = r; len = len; buf = buf;
    return EINVAL;
}

static void
_trip_segment(_trip_router_t *r, int src, size_t len, unsigned char *buf)
{
    /* Discard too short segments immediately. */
    if (len < 16) // TODO calculate the exact min length of a packet
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
        if (!(r->flag & _TRIPR_FLAG_ALLOW_IN))
        {
            _trip_router_reject(r, src, 2);
            return;
        }

        /* Check if encryption is required. */
        if (!prefix.encrypted && !(r->flag & _TRIPR_FLAG_ALLOW_PLAIN_OPEN))
        {
            _trip_router_reject(r, src, 3);
            return;
        }

        /* Unpack version and routing information. */
        uint16_t version = -1;
        _trip_route_t route;
        int endroute = trip_unpack(len - end, buf + end, "HVp",
                               &version,
                               &route.len,
                               &route.route);

        /* Discard if unpack failed. */
        if (endroute <= 0)
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
            _tripc_init(c, r, false);
            c->src = src;
        }
        else
        {
            // TODO should I be notifying the packet manager of unused src? yes.
            /* Update the source. */
            c->src = src;
            // TODO what is the best thing here? should I reparse OPEN?
            // TODO should just resend challenge...
            _tripc_set_send(c);
            return;
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
        // TODO signature validation

        /* Decrypt OPEN buffer. */
        int blen = len - endroute;
        void *b = buf + endroute;
        if (prefix.encrypted)
        {
            if (_trip_decrypt(r, blen, b))
            {
                _trip_router_reject(r, src, 21);
                return;
            }
            // TODO blen -= mac_bytes...
            // TODO b += mac_bytes... ?
        }

        if (_tripc_seg(c, prefix.control, blen, b))
        {
            /* Flag the sequence now that validation has taken place. */
            _tripc_flag_open_seq(c, prefix.seq);
        }
    }
    else
    {
        if (!prefix.encrypted && !(r->flag & _TRIPR_FLAG_ALLOW_PLAIN_SEGM))
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
    /* When leaving the state, do this. */
    switch (r->state)
    {
        case _TRIPR_STATE_START:
            {
                /* Shouldn't need to clean up anything. */
            }
            break;
        case _TRIPR_STATE_BIND:
            {
                r->statedeadline = triptime_deadline(0);
                _trip_timeout(r, -1, true);
            }
            break;
        case _TRIPR_STATE_LISTEN:
            {
                /* Do nothing.
                 * Should be listening for socket events by this point.
                 */
            }
            break;
        case _TRIPR_STATE_CLOSE:
            {
                r->statedeadline = triptime_deadline(0);
                _trip_timeout(r, -1, true);
            }
            break;
        case _TRIPR_STATE_UNBIND:
            {
                r->statedeadline = triptime_deadline(0);
                _trip_timeout(r, -1, true);
            }
            break;
        case _TRIPR_STATE_END:
            {
                /* Technically you should never get here. */
            }
            break;
        case _TRIPR_STATE_ERROR:
            {
                state = _TRIPR_STATE_ERROR;
            }
            break;
        default:
            break;
    }

    /* When entering the state, do this. */
    switch (state)
    {
        case _TRIPR_STATE_START:
            {
                /* Technically you should never get here. */
            }
            break;
        case _TRIPR_STATE_BIND:
            {
                r->statedeadline = triptime_deadline(r->timeout_bind);
                _trip_timeout(r, r->timeout_bind, true);
            }
            break;
        case _TRIPR_STATE_LISTEN:
            {
                r->statedeadline = 0;
            }
            break;
        case _TRIPR_STATE_CLOSE:
            {
                /* Possible next states: ERROR, UNBIND.
                 *
                 * Start closing connections.
                 */
                _trip_close(r, r->timeout_close);
                r->statedeadline = triptime_deadline(r->timeout_close);
                _trip_timeout(r, r->timeout_close, true);
            }
            break;
        case _TRIPR_STATE_UNBIND:
            {
                _trip_close(r, 0);
                r->packet->unbind(r->packet);
                r->statedeadline = triptime_deadline(r->timeout_unbind);
                _trip_timeout(r, r->timeout_unbind, true);
            }
            break;
        case _TRIPR_STATE_END:
            {
                /* No events for entering end. */
            }
            break;
        case _TRIPR_STATE_ERROR:
            {
                /* In error state, previous states should be 
                 * cleaned up by this point.
                 * The exception being unbinding.
                 */
                r->packet->unbind(r->packet);
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

    if (_TRIPR_STATE_LISTEN == r->state || _TRIPR_STATE_CLOSE == r->state)
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
 * Point was to allow the packet interface to resolve connection info.
 * Should be assigned a source entry or some information for
 * sending data.
 */
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
        _tripc_set_error(c, err);
        r->connection((trip_connection_t *)c);
        _tripc_destroy(c);
        _trip_free_connection(c);
    }
}

void
trip_watch(trip_router_t *_r, trip_socket_t fd, int events)
{
    trip_torouter(r, _r);

    void *data = sockmap_get(&r->sockmap, fd);
    r->watch(_r, fd, events, data);
}

void
trip_unready(trip_router_t *_r, int err)
{
    trip_torouter(r, _r);

    if (err)
    {
        _trip_set_error(r, err, NULL);
    }
    else
    {
        _trip_set_state(r, _TRIPR_STATE_END);
    }
}

void
trip_error(trip_router_t *_r, int error, const char *emsg)
{
    trip_torouter(r, _r);

    _trip_set_error(r, error, emsg);
}

int
_trip_fill_missing(_trip_router_t *r)
{
    if (!r->packet)
    {
        trip_setopt((trip_router_t *)r, TRIPOPT_PACKET, NULL);

        if (!r->packet)
        {
            _trip_set_error(r, EINVAL,
                "Unable to initialize default UDP packet interface.");
            return EINVAL;
        }
    }

    return 0;
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

trip_router_t *
trip_new(enum trip_preset preset)
{
    _trip_router_t *r = tripm_alloc(sizeof(_trip_router_t));

    do
    {
        if (!r)
        {
            break;
        }

        /* Make sure we start at zero. */
        memset(r, 0, sizeof(_trip_router_t));

        /* Set defaults. */
        r->state = _TRIPR_STATE_START;
        r->timeout_bind = 1000;
        r->max_conn = _TRIPR_DEFAULT_MAX_CONN;
        r->max_in = r->max_conn;
        r->max_out = r->max_conn;
        r->max_streams = _TRIPR_DEFAULT_MAX_STREAM;

        /* Modify values according to preset. */
        switch (preset)
        {
            case TRIP_PRESET_ROUTER:
                /* No changes, default. */
                break;
            case TRIP_PRESET_SERVER:
                r->max_out = 0;
                r->flag &= ~_TRIPR_FLAG_ALLOW_OUT;
                break;
            case TRIP_PRESET_CLIENT:
                r->max_out = 1;
                r->max_in = 0;
                r->flag &= ~_TRIPR_FLAG_ALLOW_IN;
                break;
            case TRIP_PRESET_MULTICLIENT:
                r->max_in = 0;
                r->flag &= ~_TRIPR_FLAG_ALLOW_IN;
                break;
            default:
                break;
        }

        connmap_init(&r->con, r->max_conn);
        // TODO set other settings
    } while (false);


    return (trip_router_t *)r;
}

void
trip_free(trip_router_t *_r)
{
    trip_torouter(r, _r);

    if (r->flag & _TRIPR_FLAG_FREE_PACKET)
    {
        trip_packet_free_udp(r->packet);
        r->packet = NULL;
    }

    if (r->errmsg)
    {
        tripm_free(r->errmsg);
    }

    connmap_destroy(&r->con);

    tripm_free(r);
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
            r->data = va_arg(ap, void *);
            break;
        case TRIPOPT_PACKET:
            r->packet = va_arg(ap, void *);
            if (!r->packet)
            {
                r->packet = trip_packet_new_udp(NULL);
                r->flag |= _TRIPR_FLAG_FREE_PACKET;
            }
            break;
        case TRIPOPT_WATCH_CB:
            r->watch = va_arg(ap, trip_handle_watch_t *);
            break;
        case TRIPOPT_TIMEOUT_CB:
            r->timeout = va_arg(ap, trip_handle_timeout_t *);
            break;
        case TRIPOPT_SCREEN_CB:
            r->screen = va_arg(ap, trip_handle_screen_t *);
            break;
        case TRIPOPT_CONNECTION_CB:
            r->connection = va_arg(ap, trip_handle_connection_t *);
            break;
        case TRIPOPT_STREAM_CB:
            r->stream = va_arg(ap, trip_handle_stream_t *);
            break;
        case TRIPOPT_MESSAGE_CB:
            r->message = va_arg(ap, trip_handle_message_t *);
            break;
        default:
            rval = EINVAL;
            break;
    }

    va_end(ap);

    return rval;
}

const char *
trip_errmsg(trip_router_t *_r)
{
    trip_torouter(r, _r);
    return r->errmsg ? r->errmsg : "";
}

/**
 * Perform any needed actions on timeout or socket event.
 * Called by trip_start to get the ball rolling.
 * Only a limited number of actions will take place to help prevent starvation
 * of other resources.
 * @return Errno; get string message with trip_errmsg.
 */
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

                if (_trip_fill_missing(r))
                {
                    /* Error already set. */
                    break;
                }

                if (_trip_verify(r))
                {
                    /* Error already set. */
                    break;
                }

                r->packet->router = _r;
                _trip_set_state(r, _TRIPR_STATE_BIND);
                r->packet->bind(r->packet);
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
                if (now >= r->statedeadline)
                {
                    _trip_set_error(r, ETIME, "Bind timeout.");
                    break;
                }

                int timeout = triptime_timeout(r->statedeadline, now);
                _trip_timeout(r, timeout, true);
            }
            break;
        case _TRIPR_STATE_LISTEN:
            {
                /* Possible next states: ERROR, CLOSE.
                 *
                 * If timeout, test deadline, try to read and send data.
                 * If fd, verify and check read and send for that item.
                 */
                _trip_listen(r, fd, events);
            }
            break;
        case _TRIPR_STATE_CLOSE:
            {
                /* Possible next states: ERROR, UNBIND.
                 *
                 * Perform same duties as LISTEN.
                 * LISTEN first since packets for closing may be received.
                 */
                _trip_listen(r, fd, events);

                uint64_t now = triptime_now();
                if (now >= r->statedeadline)
                {
                    _trip_set_state(r, _TRIPR_STATE_UNBIND);
                }
                else
                {
                    /* Continue waiting. */
                    // TODO set timeout again
                }
            }
            break;
        case _TRIPR_STATE_UNBIND:
            {
                /* Possible next states: ERROR, END.
                 *
                 * Wait for unbind event so we can officially be closed.
                 */
                uint64_t now = triptime_now();
                if (now >= r->statedeadline)
                {
                    _trip_set_state(r, _TRIPR_STATE_END);
                }
                else
                {
                    /* Continue waiting. */
                    // TODO set timeout again
                }
            }
            break;
        case _TRIPR_STATE_END:
            {
                /* Done, no further actions. */
            }
            break;
        case _TRIPR_STATE_ERROR:
            {
                /* Error, no further actions. */
            }
            break;
        default:
            {
                _trip_set_error(r, EINVAL, "Invalid router state.");
            }
            break;
    }

    return r->error;
}

/**
 * Associate data with the socket descriptor.
 */
void
trip_assign(trip_router_t *_r, trip_socket_t s, void *data)
{
    trip_torouter(r, _r);

    sockmap_put(&r->sockmap, s, data);
}

/**
 * Start the router.
 * @return Zero on success; errno otherwise.
 */
int
trip_start(trip_router_t *_r)
{
    trip_torouter(r, _r);
    if (_TRIPR_STATE_END == r->state)
    {
        r->state = _TRIPR_STATE_START;
    }

    if (_TRIPR_STATE_START == r->state)
    {
        return trip_action(_r, TRIP_SOCKET_TIMEOUT, 0);
    }
    else
    {
        return EINVAL;
    }
}

int
trip_stop(trip_router_t *_r)
{
    trip_torouter(r, _r);

    int c = 0;

    if (r->state <= _TRIPR_STATE_LISTEN)
    {
        _trip_set_state(r, _TRIPR_STATE_CLOSE);
    }
    else
    {
        c = EINVAL;
    }

    return c;
}

/**
 * Reject an open connection request when there is no connection struct.
 */
void
_trip_reject_connection(_trip_router_t *r, void *data, size_t ilen,
                        unsigned char *info, int err)
{
    _trip_connection_t c;
    _tripc_init(&c, r, true);
    c.data = data;
    c.ilen = ilen;
    c.info = info;
    _tripc_set_error(&c, err);
    r->connection((trip_connection_t *)&c);
}

void
trip_open_connection(trip_router_t *_r, void *data, size_t ilen,
                     unsigned char *info)
{
    trip_torouter(r, _r);

    if (r->flag & _TRIPR_FLAG_ALLOW_OUT)
    {
        _trip_connection_t *c = _trip_new_connection(r);
        if (NULL == c)
        {
            /* Out of connections. */
            _trip_reject_connection(r, data, ilen, info, ENOMEM);
            return;
        }

        _tripc_init(c, r, true);
        c->data = data;
        c->ilen = ilen;
        c->info = info;

        /* Acquire connection ID on this router. */
        if (connmap_add(&r->con, c))
        {
            /* Error. */
            _tripc_set_error(c, EBUSY);
            r->connection((trip_connection_t *)c);
            tripm_free(c);
            return;
        }

        /* Pass connection object to packet interface to 
         * interpret the info and create comms entry
         * for this connection.
         */
        r->packet->resolve(r->packet->data, (trip_connection_t *)c);
        return;
    }
    else
    {
        _trip_reject_connection(r, data, ilen, info, EINVAL);
        return;
    }
}

