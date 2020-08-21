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

#include "conn.h"
#include "crypto.h"
#include "message.h"
#include "pack.h"
#include "protocol.h"
#include "resolveq.h"
#include "stream.h"
#include "time.h"
#include "trip.h"
#include "util.h"

#include <errno.h>
#include <stdarg.h>
#include <string.h>


#define DEBUG_CONNECTION (1)

#if DEBUG_CONNECTION
#include <stdio.h>
#endif

/* CONNECTION PRIVATE */

const char *
_tripc_state_str(int s)
{
    const char *map[] =
    {
        "_TRIPC_STATE_START",
        "_TRIPC_STATE_RESOLVE",
        "_TRIPC_STATE_OPEN",
        "_TRIPC_STATE_CHAL",
        "_TRIPC_STATE_PING",
        "_TRIPC_STATE_READY",
        "_TRIPC_STATE_READY_PING",
        "_TRIPC_STATE_CLOSE",
        "_TRIPC_STATE_END",
        "_TRIPC_STATE_ERROR",
    };
    return map[s];
}

void
_tripc_set_error(_trip_connection_t *c, int eval, const char *msg)
{
    /* Prevent multiple calls. */
    if (_TRIPC_STATE_ERROR != c->state)
    {
        size_t mlen = strlen(msg ? msg : "");
        c->error = eval;
        c->errmsg = tripm_alloc(mlen + 1);
        if (c->errmsg)
        {
            memcpy(c->errmsg, msg, mlen);
            c->errmsg[mlen] = 0;
        }
        _tripc_set_state(c, _TRIPC_STATE_ERROR);
    }
}

void
_tripc_close_stream(_trip_connection_t *c, _trip_stream_t *s)
{
    c = c; // TODO
    s = s; // TODO
}

int
_tripc_message_zone(_trip_connection_t *c)
{
    return c->msg.zone;
}

void
_tripc_update_zones(_trip_connection_t *c)
{
    c = c;
    // TODO stall all streams, add stall to streams so they reject more messages
}

/**
 * @return Next message id.
 */
uint32_t
_tripc_next_message_id(_trip_connection_t *c)
{
    uint32_t id = c->msg.nextmsgid;
    ++c->msg.nextmsgid;

    // TODO change zone update to be zone/river offset
    if (c->msg.nextmsgid > _TRIPR_MAX_MESSAGE_ID)
    {
        _tripc_update_zones(c);
    }

    return id;
}

void
_tripc_mk_keys(_trip_connection_t *c)
{
    int error = 0;

    do
    {
        if (!c->encrypted)
        {
            break;
        }

        if (!c->self.pk)
        {
            c->self.pk = tripm_alloc(TRIP_KEY_PUB);
        }

        if (!c->self.sk)
        {
            c->self.sk = tripm_alloc(TRIP_KEY_SEC);
        }

        if (!c->self.pk || !c->self.sk)
        {
            error = ENOMEM;
            break;
        }

        trip_kp(c->self.pk, c->self.sk);
    } while (false);

    if (error)
    {
        c->self.pk = tripm_cfree(c->self.pk);
        c->self.sk = tripm_cfree(c->self.sk);
    }
}

void
_tripc_generate_ping(_trip_connection_t *c)
{
    if (!c->ping.nonce)
    {
        c->ping.nonce = tripm_alloc(_TRIP_NONCE);
    }

    _trip_nonce_init(c->ping.nonce);
    c->ping.timestamp = triptime_now();
}

int
_tripc_get_growth(_trip_connection_t *c);
void
_tripc_set_timeout(_trip_connection_t *c, int ms, timer_cb_t *cb);

/**
 * Resolve has timedout.
 */
void
_tripc_timeout_state_resolve_cb(void *_c)
{
    trip_toconn(c, _c);

#if DEBUG_CONNECTION
    printf("%s\n", __func__);
#endif
    
    /* Cleanup resolve entry. */
    resolveq_del(&c->router->resolveq, c->resolvekey);
    /* Clear timer ref. */
    c->statetimer = NULL;
    /* Set error and destroy. */
    _tripc_set_error(c, ETIME, NULL);
    _trip_close_connection(c->router, c);
}

void
_tripc_timeout_state_resend_cb(void *_c)
{
    trip_toconn(c, _c);

    uint64_t now = triptime_now();
    if (now > c->statedeadline)
    {
        _tripc_set_error(c, ETIME, NULL);
        _trip_close_connection(c->router, c);
        return;
    }
    else
    {
        _tripc_set_send(c);
        _tripc_set_timeout(c, _tripc_get_growth(c), _tripc_timeout_state_resend_cb);
    }
}

void
_tripc_timeout_state_ready_ping(void *_c)
{
    trip_toconn(c, _c);

    _tripc_set_state(c, _TRIPC_STATE_PING);
}

/**
 * Set the state deadline using maxstatewait.
 */
void
_tripc_set_deadline(_trip_connection_t *c, int ms)
{
    c->statedeadline = triptime_deadline(ms);
}

void
_tripc_set_growth(_trip_connection_t *c, int ms)
{
    c->growms = ms;
    c->retry = 0;
}

int
_tripc_get_growth(_trip_connection_t *c)
{
    int curr = c->growms;
    c->growms += c->growms / 2;
    if (c->growms > c->maxstatems)
    {
        c->growms = c->maxstatems;
    }
    ++c->retry;
    return curr;
}

bool
_tripc_done_retry(_trip_connection_t *c)
{
    return c->maxretry > 0 && c->retry > c->maxretry;
}

void
_tripc_set_timeout(_trip_connection_t *c, int ms, timer_cb_t *cb)
{
    c->statetimer = trip_set_timeout((trip_router_t *)c->router, ms, c, cb);
}

void
_tripc_cancel_timeout(_trip_connection_t *c)
{
    if (c->statetimer)
    {
        _trip_cancel_timeout(c->statetimer);
        c->statetimer = NULL;
    }
}

void
_tripc_set_send(_trip_connection_t *c)
{
    _trip_qconnection(c->router, c);
    c->hassend = true;
}

void
_tripc_clear_send(_trip_connection_t *c)
{
    _trip_unqconnection(c->router, c);
    c->hassend = false;
}

uint64_t
_tripc_seq(_trip_connection_t *c)
{
    return c->self.sequence++;
}

size_t
_tripc_send_data(_trip_connection_t *c, size_t blen, void *buf)
{
    // TODO check the message queue
    c = c;
    blen = blen;
    buf = buf;
    return NPOS;
}

size_t
_tripc_send_open(_trip_connection_t *c, size_t blen, void *buf)
{
#if DEBUG_CONNECTION
    printf("%s\n", __func__);
#endif
    /*
    | Octets | Field |
    |:------ |:----- |
    | PRE | PREFIX
    | 2 | Major Version ID
    | VD | Routing Information (Binary)
    | OPENING INFO |
        | Octets | Field |
        |:------ |:----- |
        | 48 | Encrypt Padding
        | 4 | Receiver ID for future responses/requests
        | 8 | Timestamp
        | 24 | Nonce client (Zeroes if unencrypted)
        | 32 | Public key client (Zeroes if unencrypted)
        | 4 | Sender Max Credit
        | 4 | Sender Max Streams
        | 4 | Sender Max Message Size
        | 4 | Sender Max Messages
        | S | SIGNATURE (OUTSIDE ENCRYPTION) |

    */
    static const char fmt[] = "sCQWHboQnkIIIIOS";
    //static const char fmt[] = "sCQWHboQQnkIIIIOS";

#if DEBUG_CONNECTION
    printf("%s: packlen(%lu)\n", __func__, trip_pack_len(fmt));
#endif

    size_t len = trip_pack(blen, buf, fmt,
            //TODO finish encryption and basic data
        c->self.sig,

        (uint8_t)_TRIP_CONTROL_OPEN,
        (uint64_t)0,
        _tripc_seq(c),

        (uint16_t)TRIP_VERSION_MAJOR,
        (size_t)0,
        (unsigned char *)NULL,

        c->peer.openpk,

        c->self.id,
        c->self.nonce,
        c->self.pk,
        (uint32_t)128000,
        (uint32_t)8,
        (uint32_t)65536,
        (uint32_t)128
        );
#if DEBUG_CONNECTION
    printf("OPEN:\n");
    trip_dump(len, buf);
    printf("\n");
#endif

    return len;
}

size_t
_tripc_send_chal(_trip_connection_t *c, size_t blen, void *buf)
{
    return trip_pack(blen, buf, "sCQWHbeQQnkIIIIES",
        (uint8_t)_TRIP_CONTROL_OPEN,
        c->peer.id,
        _tripc_seq(c),

        c->self.id,
        c->self.nonce,
        c->self.pk,
        (uint32_t)128000,
        (uint32_t)8,
        (uint32_t)65536,
        (uint32_t)128,
        c->peer.pk,
        c->self.sig
        );
}

size_t
_tripc_send_ping(_trip_connection_t *c, size_t blen, void *buf)
{
    return trip_pack(blen, buf, "CQWenQIIIE",
        (uint8_t)_TRIP_CONTROL_PING,
        c->peer.id,
        _tripc_seq(c),

        c->ping.nonce,
        c->ping.timestamp,
        c->self.stat.rtt,
        c->self.stat.sent,
        c->self.stat.recv,
        c->peer.pk
        );
}

int
_tripc_parse_open(_trip_connection_t *c, size_t len, const unsigned char *buf)
{
    int length = trip_unpack((int)len, buf, "sCQWHboQQnkIIIIOS",
        (uint8_t)_TRIP_CONTROL_OPEN,
        (uint64_t)0,
        _tripc_seq(c),

        (uint16_t)TRIP_VERSION_MAJOR,
        (size_t)0,
        (void *)NULL,

        c->self.id,
        c->self.nonce,
        c->self.pk,
        (uint32_t)128000,
        (uint32_t)8,
        (uint32_t)65536,
        (uint32_t)128,
        c->peer.openpk,
        c->peer.sig
        );
    if (length < 0)
    {
        return EINVAL;
    }

    return 0;
}

int
_tripc_parse_chal(_trip_connection_t *c)
{
    c = c;
    return EINVAL;
}

int
_tripc_parse_data(_trip_connection_t *c)
{
    c = c;
    return EINVAL;
}

int
_tripc_parse_ping(_trip_connection_t *c)
{
    c = c;
    return EINVAL;
}

int
_tripc_read(_trip_connection_t *c, unsigned char control, size_t len, const unsigned char *buf)
{
    len = len; buf = buf;
    switch (c->state)
    {
        case _TRIPC_STATE_START:
            {
                /* Possible next states: OPEN.
                 */
            }
            break;
        case _TRIPC_STATE_OPEN:
            {
                if(_tripc_parse_chal(c))
                {
                    return EINVAL;
                }
            }
            break;
        case _TRIPC_STATE_CHAL:
            {
                if (_TRIP_CONTROL_DATA == control)
                {
                    if (_tripc_parse_data(c))
                    {
                        return EINVAL;
                    }

                    _tripc_set_state(c, _TRIPC_STATE_READY);
                }
                else if (_TRIP_CONTROL_OPEN == control)
                {
                    if (_tripc_parse_open(c, len, buf))
                    {
                        return EINVAL;
                    }

                    // TODO friend didn't receive chal set send state
                    // TODO send chal packet in send function
                    _tripc_set_send(c);
                }
                else if (_TRIP_CONTROL_PING == control)
                {
                    if (_tripc_parse_ping(c))
                    {
                        return EINVAL;
                    }

                    _tripc_set_state(c, _TRIPC_STATE_READY);
                }
                else
                {
                    return EINVAL;
                }
            }
            break;
        case _TRIPC_STATE_PING:
            {
                if (_tripc_parse_ping(c))
                {
                    return EINVAL;
                }
            }
            break;
        case _TRIPC_STATE_READY:
            {
            }
            break;
        case _TRIPC_STATE_READY_PING:
            {
            }
            break;
        case _TRIPC_STATE_END:
            {
            }
            break;
        case _TRIPC_STATE_ERROR:
            {
            }
            break;
        default:
            {
                c->error = EINVAL;
            }
            break;
    }

    return c->error;
}

/**
 * @return Zero on finish; NPOS or >len on error; otherwise number of bytes written.
 */
size_t
_tripc_send(_trip_connection_t *c, size_t len, void *buf)
{
#if DEBUG_CONNECTION
    printf("%s: state(%s)\n", __func__, _tripc_state_str(c->state));
#endif

    switch (c->state)
    {
        case _TRIPC_STATE_START:
            {
                // TODO does this trigger an error and kill the connection?
                return NPOS;
            }
            break;
        case _TRIPC_STATE_RESOLVE:
            {
                // TODO does this trigger an error and kill the connection?
                return NPOS;
            }
            break;
        case _TRIPC_STATE_OPEN:
            {
                if (c->hassend)
                {
                    size_t wlen = _tripc_send_open(c, len, buf);
                    printf("len pack: %lu\n", wlen);
                    c->hassend = false;
                    return wlen;
                }
                else
                {
                    /* The timeout is set so we don't need to send again yet. */
                    return 0;
                }
            }
            break;
        case _TRIPC_STATE_CHAL:
            {
                return _tripc_send_chal(c, len, buf);
            }
            break;
        case _TRIPC_STATE_PING:
            {
                // TODO only if we had a timeout do we send ping
                return _tripc_send_ping(c, len, buf);
            }
            break;
        case _TRIPC_STATE_READY:
            {
                return _tripc_send_data(c, len, buf);
            }
            break;
        case _TRIPC_STATE_READY_PING:
            {
                // TODO is this a vestigial state?
            }
            break;
        case _TRIPC_STATE_END:
            {
                return NPOS;
            }
            break;
        case _TRIPC_STATE_ERROR:
            {
                return NPOS;
            }
            break;
        default:
            {
                c->error = EINVAL;
            }
            break;
    }

    return c->error;
}

/**
 * Initialize the connection information with defaults.
 */
void
_tripc_init(_trip_connection_t *c, _trip_router_t *r, bool incoming)
{
    memset(c, 0, sizeof(*c));
    c->router = r;
    c->incoming = incoming;
    streammap_init(&c->streams, r->max_streams);
    c->maxresolve = 500;
    c->maxstatems = 3000;
    c->statems = 100;
}

/**
 * Destroy and free memory.
 */
void
_tripc_destroy(_trip_connection_t *c)
{
    streammap_destroy(&c->streams);

    if (c->statetimer)
    {
        _trip_cancel_timeout(c->statetimer);
        c->statetimer = NULL;
    }

    if (c->insend)
    {
        _trip_unqconnection(c->router, c);
    }

    tripm_cfree(c->errmsg);
    c->errmsg = NULL;
}

/**
 * @brief OPEN sequences are unique.
 *
 * Latest should be set to 512.
 * If greater than 512. Invalid.
 */
int
_tripc_check_open_seq(_trip_connection_t *c, uint32_t seq)
{
    if (seq < c->peer.seqfloor)
    {
        return EINVAL;
    }

    return 0;
}

/**
 * @brief Flag the number. Advance window if needed.
 *
 *
 * seq is bad and previously discarded
 * floor?
 * seq is bad and previously discarded
 * newfloor = 512 less than latest
 * floor?
 *
 * latest
 * seq advance latest
 */
void
_tripc_flag_open_seq(_trip_connection_t *c, uint32_t seq)
{
    uint32_t offset = seq - c->peer.seqfloor;
    if (offset > c->peer.window)
    {
        c->peer.seqfloor += offset - c->peer.window;
    }
}

/**
 * @brief Change states and do proper create/destroy when entering/leaving.
 */
void
_tripc_set_state(_trip_connection_t *c, enum _tripc_state state)
{
#if DEBUG_CONNECTION
    printf("%s: from(%s) -> to(%s)\n", __func__, _tripc_state_str(c->state), _tripc_state_str((int)state));
#endif
    
    /* When leaving the state, do this. */
    switch(state)
    {
        case _TRIPC_STATE_START:
            {
                /* Nothing here. */
            }
            break;
        case _TRIPC_STATE_RESOLVE:
            {
                _tripc_cancel_timeout(c);
            }
            break;
        case _TRIPC_STATE_OPEN:
            {
                _tripc_cancel_timeout(c);
            }
            break;
        case _TRIPC_STATE_CHAL:
            {
                _tripc_cancel_timeout(c);
            }
            break;
        case _TRIPC_STATE_PING:
            {
                _tripc_cancel_timeout(c);
            }
            break;
        case _TRIPC_STATE_READY:
            {
                _tripc_cancel_timeout(c);
            }
            break;
        case _TRIPC_STATE_CLOSE:
            {
                _tripc_cancel_timeout(c);
            }
            break;
        case _TRIPC_STATE_END:
            {
                /* Memory should be free'd shortly after reaching this state. */
            }
            break;
        case _TRIPC_STATE_ERROR:
            {
                /* Don't allow to leave ERROR state. */
                state = _TRIPC_STATE_ERROR;
            }
            break;
        default:
            break;
    }

    c->state = state;

    /* When entering the state, do this. */
    switch (state)
    {
        case _TRIPC_STATE_START:
            {
                /* Do nothing, shouldn't get here. */
            }
            break;
        case _TRIPC_STATE_RESOLVE:
            {
                /* Set a timeout. */
                _tripc_set_timeout(c, c->maxresolve, _tripc_timeout_state_resolve_cb);
            }
            break;
        case _TRIPC_STATE_OPEN:
            {
                _tripc_mk_keys(c);
                _tripc_set_deadline(c, c->maxstatems);
                _tripc_set_growth(c, c->statems);
                _tripc_set_send(c);
                _tripc_set_timeout(c, _tripc_get_growth(c), _tripc_timeout_state_resend_cb);
            }
            break;
        case _TRIPC_STATE_CHAL:
            {
                _tripc_mk_keys(c);
                _tripc_set_deadline(c, c->maxstatems);
                _tripc_set_growth(c, c->statems);
                _tripc_set_timeout(c, _tripc_get_growth(c), _tripc_timeout_state_resend_cb);
                _tripc_set_send(c);
            }
            break;
        case _TRIPC_STATE_PING:
            {
                _tripc_generate_ping(c);
                _tripc_set_deadline(c, c->ping.maxms);
                _tripc_set_growth(c, c->ping.ms);
                _tripc_set_timeout(c, _tripc_get_growth(c), _tripc_timeout_state_resend_cb);
                _tripc_set_send(c);
            }
            break;
        case _TRIPC_STATE_READY:
            {
                _tripc_set_deadline(c, c->ping.maxms * 2);
                _tripc_set_timeout(c, c->ping.maxms + c->ping.maxms/2, _tripc_timeout_state_ready_ping);
            }
            break;
        case _TRIPC_STATE_CLOSE:
            {
                _tripc_set_deadline(c, c->maxstatems);
                _tripc_set_timeout(c, c->statems, _tripc_timeout_state_ready_ping);
            }
            break;
        case _TRIPC_STATE_END:
            {
            }
            break;
        case _TRIPC_STATE_ERROR:
            /* Do nothing, cleanup should be done. */
            // TODO is this where we should notify the user?
            break;
        default:
            break;
    }
}

/**
 * @brief Add the message to the send Q.
 */
void
_tripc_send_add(_trip_connection_t *c, _trip_msg_t *m)
{
    if (c->msg.sendend[m->priority])
    {
        c->msg.sendend[m->priority]->next = m;
        c->msg.sendend[m->priority] = m;
    }
    else
    {
        c->msg.sendbeg[m->priority] = m;
        c->msg.sendend[m->priority] = m;
    }

    m->next = NULL;
}

/**
 * @brief Pick the message to send next data from.
 * @return NULL if Q's are empty; message to send data from otherwise.
 */
_trip_msg_t *
_tripc_send_pick(_trip_connection_t *c)
{
    c->msg.sendwhich = (c->msg.sendwhich + 1) & c->msg.sendmask;

    int which = c->msg.sendwhich ? _TRIP_MESSAGE_PRIORITY : _TRIP_MESSAGE_WHENEVER;

    if (c->msg.sendbeg[which])
    {
        return c->msg.sendbeg[which];
    }
    else
    {
        which = 0x01 & (which + 1);
        if (c->msg.sendbeg[which])
        {
            return c->msg.sendbeg[which];
        }
        else
        {
            return NULL;
        }
    }
}

/**
 * @brief Remove the message.
 * @param m - The message should be at the front of its Q.
 */
void
_tripc_send_clear(_trip_connection_t *c, _trip_msg_t *m)
{
    if (m == c->msg.sendbeg[m->priority])
    {
        if (c->msg.sendbeg[m->priority] == c->msg.sendend[m->priority])
        {
            c->msg.sendbeg[m->priority] = NULL;
            c->msg.sendend[m->priority] = NULL;
        }
        else
        {
            c->msg.sendbeg[m->priority] = m->next;
        }

        m->next = NULL;
    }

}


/* CONNECTION SHARED */

/**
 * Start the connection. Timeouts and state will be set.
 */
void
_tripc_start(_trip_connection_t *c)
{
    if (_TRIPC_STATE_START != c->state)
    {
        _tripc_set_error(c, EINVAL, NULL);
        // TODO close connection
        return;
    }

    enum _tripc_state state = _TRIPC_STATE_RESOLVE;
    if (c->incoming)
    {
        // TODO hmmm, shouldn't be calling start to get to challenge state...?
        state = _TRIPC_STATE_CHAL;
    }

    _tripc_set_state(c, state);
}

/**
 * Notify the connection that the address information has been resolved.
 */
void
_tripc_resolved(_trip_connection_t *c)
{
    _tripc_set_state(c, _TRIPC_STATE_OPEN);
}

/**
 * Get memory for a message.
 * TODO slab allocate memory
 * @return Uninitialized message struct.
 */
_trip_msg_t *
_tripc_new_message(_trip_connection_t *c)
{
    c = c;
    _trip_msg_t *m = tripm_alloc(sizeof(_trip_msg_t));
    return m;
}

/**
 * Release memory into the pool.
 */
void
_tripc_free_message(_trip_connection_t *c, _trip_msg_t *m)
{
    c = c;
    tripm_free(m);
}

/* CONNECTION PUBLIC */

/**
 * @return Status code.
 */
enum trip_connection_status
tripc_status(trip_connection_t *_c)
{
    trip_toconn(c, _c);
    return c->status;
}

/**
 * @brief Close the connection. Cleanup your data prior to calling.
 */
void
tripc_close(trip_connection_t *c, int gracems)
{
    // TODO
    c = c;
    gracems = gracems;
    //trip_toconn(c, _c);
}

/**
 * @return The next stream ID available.
 */
int
tripc_next_stream(trip_connection_t *_c)
{
    _c = _c; // TODO
    return 0; // TODO
}

/**
 * @return NULL if stream cannot be created.
 */
trip_stream_t *
tripc_open_stream(trip_connection_t *_c, int sid, int options)
{
    trip_toconn(c, _c);

    _trip_stream_t *s = tripm_alloc(sizeof(_trip_stream_t));

    do
    {
        if (s)
        {
            s->data = NULL;
            s->connection = c;
            s->id = sid;
            s->flags = options & _TRIPS_OPT_PUBMASK;
            s->listbeg = NULL;
            s->listend = NULL;
            streammap_add(&c->streams, s);
        }
    } while (false);

    return (trip_stream_t *)s;
}

int
tripc_get_errno(trip_connection_t *_c)
{
    trip_toconn(c, _c);
    return c->error;
}

const char *
tripc_get_errmsg(trip_connection_t *_c)
{
    trip_toconn(c, _c);

    if (c->error && !c->errmsg)
    {
        c->errmsg = strdup(strerror(c->error));
    }

    return c->errmsg;
}

