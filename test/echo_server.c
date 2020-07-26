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
 * @file test_echo.h
 * @author Craig Jacobson
 * @brief Setup an echo server and client to demonstrate libtrp.
 *
 * The interface to the TRIP router is similar to libcurl multi interface.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "libtrp.h"


/* DATA */

typedef struct
{
    uv_loop_t *loop;
    trip_router_t *server;
    uv_timer_t timeout; /* server timeout */
} mydata_t;

/* CALLBACKS */

/**
 * Determine if we are going to accept this connection.
 * Since we just have a private echo server we'll accept any connection request.
 *
 * Possible information:
 * Allow flag to signal that this source is valid.
 * Routing.
 * Source address.
 * Source address binary.
 * Source address as string.
 * Signature.
 * Decrypt OPEN keys.
 */
void
server_handle_screen(trip_screen_t *screen)
{
    screen->allow = true;
}

/**
 * Handle connection event.
 */
void
server_handle_connection(trip_connection_t *c)
{
    switch (tripc_status(c))
    {
        case TRIPC_STATUS_OPEN:
            break;
        case TRIPC_STATUS_CLOSED:
            break;
        case TRIPC_STATUS_KILLED:
            break;
        case TRIPC_STATUS_ERROR:
            break;
        default:
            abort();
    }
}

/**
 * Handle stream event.
 */
void
server_handle_stream(trip_stream_t *s)
{
    switch (trips_status(s))
    {
        case TRIPS_STATUS_OPEN:
            break;
        case TRIPS_STATUS_CLOSED:
            break;
        case TRIPS_STATUS_KILLED:
            break;
        case TRIPS_STATUS_ERROR:
            break;
        default:
            abort();
    }
}

/**
 * Handle message event on stream.
 */
static void
server_handle_message(trip_stream_t *s, enum trip_message_status status, size_t len, const unsigned char *buf)
{
    printf("Server received message: %s\n", (const char *)buf);

    switch (status)
    {
        case TRIPM_RECV:
            {
                /* Open stream if not already open. */
                trip_stream_t *stream = s->data;
                if (!stream)
                {
                    trip_connection_t *c = s->connection;
                    stream = tripc_open_stream(c, trips_id(s), trips_type(s));
                    s->data = stream;
                }

                /* Copy message.
                 * Buffers received won't last past this call.
                 */
                unsigned char *copybuf = malloc(len);
                memcpy(copybuf, buf, len);

                /* Send message back. */
                trips_send(stream, len, copybuf);
            }
            break;
        case TRIPM_SENT:
            {
                /* Free our message. */
                free((void *)buf);
            }
            break;
        case TRIPM_KILL:
            {
                /* Free our message if not sent. */
                free((void *)buf);
            }
            break;
        default:
            abort();
    }
}

/* TIMEOUT HANDLING */

/**
 * Callback for libuv to call upon timeout.
 */
static void on_timeout(uv_timer_t *req)
{
    trip_router_t *router = (trip_router_t *)req->data;
    trip_action(router, TRIP_SOCKET_TIMEOUT, 0);
}

/**
 * Set or clear the timeout needed by the router.
 */
static int
router_handle_timeout(trip_router_t *router, long ms)
{
    mydata_t *data = router->data;
    uv_timer_t *t = &data->timeout;
    t->data = router;

    if (ms < 0)
    {
        uv_timer_stop(&data->timeout);
    }
    else
    {
        if (0 == ms)
        {
            trip_action(router, TRIP_SOCKET_TIMEOUT, 0);
        }
        else
        {
            uv_timer_start(&data->timeout, on_timeout, ms, 0);
        }
    }

    return 0;
}

/* SOCKET HANDLING */

typedef struct mysocket_s
{
  uv_poll_t poll_handle;
  trip_socket_t sockfd;
  trip_router_t *router;
} mysocket_t;

/**
 * Callback for libuv to call when an event is encountered.
 */
static void
on_watch(uv_poll_t *req, int status, int events)
{
    if (!status)
    {
        int flags = 0;
        mysocket_t *c = (mysocket_t *)req->data;

        if (events & UV_READABLE)
        {
            flags |= TRIP_IN;
        }
        if (events & UV_WRITABLE)
        {
            flags |= TRIP_OUT;
        }

        trip_action(c->router, c->sockfd, flags);
    }
    else
    {
        abort();
    }
}


/**
 * Init our socket data in context of libuv.
 */
mysocket_t *
mysocket_new(trip_socket_t s, uv_loop_t *loop, trip_router_t *router)
{
    mysocket_t *c = malloc(sizeof(mysocket_t));
    c->sockfd = s;
    uv_poll_init_socket(loop, &c->poll_handle, c->sockfd);
    c->poll_handle.data = c;
    c->router = router;
    return c;
}

/**
 * Uninit our socket data and libuv handle.
 */
void
mysocket_free(mysocket_t *c)
{
    uv_poll_stop(&c->poll_handle);
    free(c);
}

/**
 * Handle watch request for socket used by router.
 * Here we use libuv to watch the socket and notify us of events.
 */
static int
router_handle_watch(trip_router_t *router, trip_socket_t s, int action, void *sp)
{
    mysocket_t *context = NULL;
    mydata_t *data = (mydata_t *)router->data;
    int events = 0;

    switch (action)
    {
        case TRIP_REMOVE:
            if (sp)
            {
                /* Avoid memory leaks. */
                mysocket_free((mysocket_t*)sp);
                trip_assign(router, s, NULL);
            }
            break;
        case TRIP_IN:
        case TRIP_OUT:
        case TRIP_INOUT:
            context = sp ? (mysocket_t *)sp : mysocket_new(s, data->loop, router);

            trip_assign(router, s, context);

            if (action != TRIP_IN)
            {
                events |= UV_WRITABLE;
            }
            if (action != TRIP_OUT)
            {
                events |= UV_READABLE;
            }

            uv_poll_start(&context->poll_handle, events, on_watch);
            break;
        default:
            abort();
    }

    return 0;
}

/* SETUP */

static void
server_init(mydata_t *data)
{
    trip_router_t *server = data->server = trip_new(TRIP_PRESET_SERVER);
    trip_setopt(server, TRIPOPT_USER_DATA, data);
    trip_setopt(server, TRIPOPT_WATCH_CB, router_handle_watch);
    trip_setopt(server, TRIPOPT_TIMEOUT_CB, router_handle_timeout);
    trip_setopt(server, TRIPOPT_SCREEN_CB, server_handle_screen);
    trip_setopt(server, TRIPOPT_CONNECTION_CB, server_handle_connection);
    trip_setopt(server, TRIPOPT_STREAM_CB, server_handle_stream);
    trip_setopt(server, TRIPOPT_MESSAGE_CB, server_handle_message);
    trip_start(server);
}

static void
server_destroy(mydata_t *data)
{
    trip_free(data->server);
}

int main()
{
    mydata_t data = { 0 };

    data.loop = malloc(sizeof(uv_loop_t));
    uv_loop_init(data.loop);
    uv_timer_init(data.loop, &data.timeout);

    server_init(&data);
    uv_run(data.loop, UV_RUN_DEFAULT);
    server_destroy(&data);

    uv_loop_close(data.loop);
    free(data.loop);
  
    return 0;
}

