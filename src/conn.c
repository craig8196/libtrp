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

#include "trip.h"
#include "conn.h"
#include "stream.h"
#include "message.h"

#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <sys/epoll.h>


void
_tripc_close_stream(_trip_connection_t *c, _trip_stream_t *s)
{
    c = c; // TODO
    s = s; // TODO
}

int
_tripc_message_zone(_trip_connection_t *c)
{
    return c->zone;
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
    uint32_t id = c->nextmsgid;
    ++c->nextmsgid;

    if (c->nextmsgid > c->router->max_message_id)
    {
        _tripc_update_zones(c);
    }

    return id;
}

int
_tripc_read(_trip_connection_t *c, size_t len, void *buf)
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
            }
            break;
        case _TRIPC_STATE_CHAL:
            {
            }
            break;
        case _TRIPC_STATE_PING:
            {
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

int
_tripc_timeout(_trip_connection_t *c)
{
    switch (c->state)
    {
        case _TRIPC_STATE_START:
            {
            }
            break;
        case _TRIPC_STATE_OPEN:
            {
            }
            break;
        case _TRIPC_STATE_CHAL:
            {
            }
            break;
        case _TRIPC_STATE_PING:
            {
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

int
_tripc_send(_trip_connection_t *c, size_t len, void *buf)
{
    len = len; buf = buf;
    switch (c->state)
    {
        case _TRIPC_STATE_START:
            {
            }
            break;
        case _TRIPC_STATE_OPEN:
            {
            }
            break;
        case _TRIPC_STATE_CHAL:
            {
            }
            break;
        case _TRIPC_STATE_PING:
            {
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
 * Initialize the connection information with defaults.
 */
void
_tripc_init(_trip_connection_t *c, _trip_router_t *r, bool incoming)
{
    memset(c, 0, sizeof(*c));
    c->router = r;
    c->incoming = incoming;
    streammap_init(&c->streams, r->max_streams);
}

/**
 * Destroy and free memory.
 */
void
_tripc_destroy(_trip_connection_t *c)
{
    streammap_destroy(&c->streams);
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
    if (seq < c->seqfloor)
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
    uint32_t offset = seq - c->seqfloor;
    if (offset > c->window)
    {
        c->seqfloor += offset - c->window;
    }
}

int
_tripc_seg(_trip_connection_t *c, unsigned char control, int len, const unsigned char *buf)
{
    c = c;
    control = control;
    len = len;
    buf = buf;
    return -1;
}

/* CONNECTION INTERNAL */

/**
 * @brief Change states and do proper create/destroy when entering/leaving.
 */
int
_tripc_set_state(_trip_connection_t *c, enum _tripc_state state)
{
    /* When leaving the state, do this. */
    switch(state)
    {
        case _TRIPC_STATE_START:
            /* Do nothing */
            break;
        case _TRIPC_STATE_OPEN:
            break;
        case _TRIPC_STATE_CHAL:
            break;
        case _TRIPC_STATE_PING:
            break;
        case _TRIPC_STATE_READY:
            break;
        case _TRIPC_STATE_READY_PING:
            break;
        case _TRIPC_STATE_CLOSE:
            break;
        case _TRIPC_STATE_END:
            break;
        case _TRIPC_STATE_ERROR:
            break;
        default:
            break;
    }

    /* When entering the state, do this. */
    switch (state)
    {
        case _TRIPC_STATE_START:
            /* Do nothing, shouldn't get here. */
            break;
        case _TRIPC_STATE_OPEN:
            break;
        case _TRIPC_STATE_CHAL:
            break;
        case _TRIPC_STATE_PING:
            break;
        case _TRIPC_STATE_READY:
            break;
        case _TRIPC_STATE_READY_PING:
            break;
        case _TRIPC_STATE_CLOSE:
            break;
        case _TRIPC_STATE_END:
            break;
        case _TRIPC_STATE_ERROR:
            break;
        default:
            break;
    }

    return c->error;
}

/**
 * @brief Add the message to the send Q.
 */
void
_tripc_send_add(_trip_connection_t *c, _trip_msg_t *m)
{
    if (c->sendend[m->priority])
    {
        c->sendend[m->priority]->next = m;
        c->sendend[m->priority] = m;
    }
    else
    {
        c->sendbeg[m->priority] = m;
        c->sendend[m->priority] = m;
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
    c->sendwhich = (c->sendwhich + 1) & c->sendmask;

    int which = c->sendwhich ? _TRIP_MESSAGE_PRIORITY : _TRIP_MESSAGE_WHENEVER;

    if (c->sendbeg[which])
    {
        return c->sendbeg[which];
    }
    else
    {
        which = 0x01 & (which + 1);
        if (c->sendbeg[which])
        {
            return c->sendbeg[which];
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
    if (m == c->sendbeg[m->priority])
    {
        if (c->sendbeg[m->priority] == c->sendend[m->priority])
        {
            c->sendbeg[m->priority] = NULL;
            c->sendend[m->priority] = NULL;
        }
        else
        {
            c->sendbeg[m->priority] = m->next;
        }

        m->next = NULL;
    }

}


/* CONNECTION SHARED */

/**
 * @brief Start the connection. Timeouts and state will be set.
 */
void
_tripc_start(_trip_connection_t *c)
{
    if (_TRIPC_STATE_START != c->state)
    {
        _tripc_set_error(c, EINVAL);
        // TODO close connection
        return;
    }

    enum _tripc_state state = _TRIPC_STATE_OPEN;
    if (c->incoming)
    {
        state = _TRIPC_STATE_CHAL;
    }

    _tripc_set_state(c, state);
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

