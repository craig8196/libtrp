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
 */
#include <stdio.h>
#include <stdlib.h>
#include <uv.h>
#include <curl/curl.h>

#include "libtrp.h"


/* DATA */

typedef struct
{
    uv_loop_t *loop;

    trip_router_t *client;
    trip_connection_t *conn;

    uv_timer_t client_timeout;
} mydata_t;

/* CALLBACKS */

void
client_handle_connection(trip_connection_t *c)
{
    switch (tripc_status(c))
    {
        case TRIPC_STATUS_OPEN:
            {
                int opts = TRIPS_OPT_PRIORITY | TRIPS_OPT_CHUNK
                           | TRIPS_OPT_ORDERED | TRIPS_OPT_RELIABLE;
                trip_stream_t *s = tripc_open_stream(c, 0, opts);
                const char *msg = "Hello, world!";
                size_t len = strlen(msg);
                trips_send(s, len, (const unsigned char *)msg);
            }
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

void
client_handle_stream(trip_stream_t *s)
{
    switch (trips_status(c))
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

static void
client_handle_message(trip_stream_t *s, size_t len, const unsigned char *buf)
{
    printf("Client received message: %s\n", (const char *)buf);
}

/* TIMEOUT HANDLING */

static void on_timeout(uv_timer_t *req)
{
    trip_router_t *router = (trip_router_t *)req->data;
    trip_action(router, TRIP_TIMEOUT, 0);
}

static int
router_handle_timeout(trip_router_t *router, long ms)
{
    mydata_t *data = router->data;
    uv_timer_t *t = &data->client_timeout;
    t->data = router;

    if (ms < 0)
    {
        uv_timer_stop(&t->timeout);
        if (0 == ms)
        {
            trip_action(router, TRIP_TIMEOUT, flags);
        }
    }
    else
    {
        uv_timer_start(&t->timer, on_timeout, ms, 0);
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

static void
on_watch(uv_poll_t *req, int status, int events)
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

mysocket_t *
mysocket_new(trip_socket_t s, uv_loop_t *loop, trip_router_t *router)
{
    mysocket_t *c = malloc(sizeof(mysocket_t));
    c->sockfd = s;
    uv_poll_init_socket(loop, &c->poll_handle, sockfd);
    c->poll_handle.data = c;
    c->router = router;
    return c;
}

void
mysocket_free(mysocket_t *c)
{
    uv_poll_stop(&c->poll_handle);
    free(c);
}

static int
router_handle_watch(trip_router_t *router, trip_socket_t s, int action, void *sp)
{
    mysocket_t *context = NULL;
    mydata_t *data = (mydata_t *)router->data;
    int events = 0;

    switch (action)
    {
        case TRIP_REMOVE:
            if (socketp)
            {
                mysocket_free((mysocket_t*)sp);
                trip_assign(router, s, NULL);
            }
            break;
        case TRIP_IN:
        case TRIP_OUT:
        case TRIP_INOUT:
            context = sp ? (mysocket_t *)sp : mysocket_new(s, data->loop);

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
client_init(mydata_t *data)
{
    trip_router_t *client =  = data->client = trip_new(TRIP_PRESET_CLIENT);
    trip_setopt(client, TRIPOPT_USER_DATA, data);
    trip_setopt(client, TRIPOPT_WATCH_CB, router_handle_watch);
    trip_setopt(client, TRIPOPT_TIMEOUT_CB, router_handle_timeout);
    trip_setopt(client, TRIPOPT_CONNECTION_CB, client_handle_connection);
    trip_setopt(client, TRIPOPT_STREAM_CB, client_handle_stream);
    trip_setopt(client, TRIPOPT_MESSAGE_CB, client_handle_message);
    trip_start(client);

    const char *location = "localhost";
    size_t len = strlen(location);
    trip_connection_t *conn = NULL;
    conn = trip_open_connection(client);
    conn->data = data;
    /*
     * What is this? TODO
    conn->locationlen = len;
    conn->location = location;
    conn->key = data->openpub;
    conn->sign = data->signpub;
    */
}

static void
client_destroy(mydata_t *data)
{
    trip_free(data->client);
}

int main(int argc, char **argv)
{
    mydata_t data = { 0 };

    data.loop = uv_default_loop();

    uv_timer_init(data.loop, &data.client_timeout);

    client_init(&data);
    // TODO start the client
    uv_run(loop, UV_RUN_DEFAULT);
    client_destroy(&data);
  
    return 0;
}
