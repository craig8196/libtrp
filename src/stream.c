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
 * @file stream.c
 * @author Craig Jacobson
 * @brief Stream code.
 */
#include "libtrp.h"
#include "trip.h"
#include "conn.h"
#include "stream.h"

#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <sys/epoll.h>


/* STREAM INTERNALS */

/**
 * @return Priority 1/0.
 */
static int
_trips_priority(_trip_stream_t *s)
{
    return s->flags & TRIPS_OPT_PRIORITY;
}

/**
 * @brief Add message to queue of messages being sent.
 */
void
_trips_msg_add(_trip_stream_t *s, _trip_msg_t *m)
{
    if (s->listend)
    {
        s->listend->next = m;
        s->listend = m;
    }
    else
    {
        s->listbeg = m;
        s->listend = m;
    }
    m->next = NULL;
}

/**
 * @brief Remove the message from the queue. Identified by the pointer.
 */
void
_trips_msg_remove(_trip_stream_t *s, _trip_msg_t *mess)
{
    _trip_msg_t *p = NULL;
    _trip_msg_t *m = s->listbeg;

    while (m)
    {
        if (m == mess)
        {
            if (p)
            {
                p->next = m->next;

                if (m == s->listend)
                {
                    s->listend = p;
                }
                else
                {
                    /* Mid list. */
                }
            }
            else
            {
                if (m == s->listend)
                {
                    s->listbeg = NULL;
                    s->listend = NULL;
                }
                else
                {
                    s->listbeg = m->next;
                }
            }

            m->next = NULL;

            break;
        }

        p = m;
        m = m->next;
    }
}

/* STREAM SHARED */

/**
 * @brief Message incoming on stream.
 */
void
_trips_message(_trip_stream_t *s, int len, unsigned char *buf)
{
    // TODO
    s = s;
    len = len;
    buf = buf;
    //s->message_cb((trip_stream_t *)s, TRIPM_RECV, (size_t)len, buf);
}

/**
 * @brief Message was sent and confirmed received per stream requirements.
 */
void
_trips_done_message(_trip_stream_t *s, _trip_msg_t *m)
{
    _trips_msg_remove(s, m);

    //s->message_cb((trip_stream_t *)s, TRIPM_SENT, m->len, m->buf);
    _tripc_free_message(s->connection, m);
}

/**
 * @brief Stream being destroyed. Free resources.
 * @warn Not all messages may have been sent.
 */
void
_trips_destroy(_trip_stream_t *s)
{
    _trip_msg_t *m = s->listbeg;
    _trip_msg_t *n = NULL;

    while (m)
    {
        n = m->next;

        //s->message_cb((trip_stream_t *)s, TRIPM_KILL, m->len, m->buf);
        _tripc_free_message(s->connection, m);

        m = n;
    }
}

/* STREAM PUBLIC */

/**
 * @brief Check if the stream is open for writing.
 */
bool
trips_isopen(trip_stream_t *_s)
{
    trip_tostream(s, _s);
    return !(s->flags & _TRIPS_OPT_CLOSED);
}

/**
 * @brief User flagging the stream as finished.
 */
void
trips_close(trip_stream_t *_s)
{
    trip_tostream(s, _s);

    if (s->connection)
    {
        _tripc_close_stream(s->connection, s);
    }
    else
    {
        /* Stream is invalid. */
    }
}

/**
 * @brief Send a message on the stream.
 * @return Zero on success in passing to framework; error otherwise.
 */
int
trips_send(trip_stream_t *_s, size_t len, unsigned char *buf)
{
    trip_tostream(s, _s);

    int code = 0;

    do
    {
        if (!s->connection)
        {
            code = ENOTCONN;
            break;
        }

        if (!buf || !len || len > INT_MAX || len > s->connection->max_message_size)
        {
            code = EINVAL;
            break;
        }

        if (s->flags & _TRIPS_OPT_CLOSED)
        {
            code = ESHUTDOWN;
            break;
        }

        if (s->flags & (_TRIPS_OPT_BACKFLOW | _TRIPS_OPT_STALLED))
        {
            code = EWOULDBLOCK;
            break;
        }

        _trip_msg_t *m = _tripc_new_message(s->connection);
        
        if (!m)
        {
            code = ENOMEM;
            break;
        }

        m->stream = s;
        m->next = NULL;
        m->priority = _trips_priority(s);
        m->len = len;
        m->buf = buf;
        m->parts = NULL;

        _tripc_send_add(s->connection, m);
        _trips_msg_add(s, m);
    } while (false);

    return code;
}

