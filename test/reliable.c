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
 * @brief Setup an echo router and client to demonstrate libtrp.
 *
 * The interface to the TRIP router is similar to libcurl multi interface.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libtrp.h"
#include "../src/time.h"
#include "reliable_packet.h"


/* DATA */

typedef struct
{
    bool running;
    bool stop;
    uint64_t stopdeadline;
    trip_router_t *router;
    trip_packet_t *reliable;
} mydata_t;

/* CALLBACKS */

/**
 * Determine if we are going to accept this connection.
 * Since we just have a private echo router we'll accept any connection request.
 *
 * Possible information:
 * Allow flag to signal that this source is valid.
 * Routing buffer (binary).
 * Source address.
 *   Source address as binary.
 *   Source address as string.
 * Set signature keys.
 * Set decrypt OPEN keys.
 */
void
router_handle_screen(trip_router_t *r, trip_screen_t *screen)
{
    r = r;
    screen->allow = true;
}

const unsigned char MSG[] = "echo";
const size_t MSGLEN = sizeof(MSG) - 1;

/**
 * Handle connection event.
 */
void
router_handle_connection(trip_connection_t *c)
{
    switch (tripc_status(c))
    {
        case TRIPC_STATUSI_OPEN:
            {
                /* Sweet the server received the connection. */
                // TODO next step here
            }
            break;
        case TRIPC_STATUSI_CLOSED:
            {
            }
            break;
        case TRIPC_STATUSI_KILLED:
            {
            }
            break;
        case TRIPC_STATUSI_ERROR:
            {
                printf("CONNECTION ERROR: %s\n", tripc_get_errmsg(c));
            }
            break;
        case TRIPC_STATUSO_OPEN:
            {
                /* Sweet, we have a connection out. */
                if (!c->data)
                {
                    int opts = TRIPS_OPT_PRIORITY
                        | TRIPS_OPT_ORDERED
                        | TRIPS_OPT_RELIABLE;
                    c->data = tripc_open_stream(c, 0, opts);
                    trips_send(c->data, MSGLEN, MSG);
                }
            }
            break;
        case TRIPC_STATUSO_CLOSED:
            {
                /* Closed normally by user. */
            }
            break;
        case TRIPC_STATUSO_KILLED:
            {
                /* Closed because router is shutting down. */
            }
            break;
        case TRIPC_STATUSO_ERROR:
            {
                printf("CONNECTION ERROR: %s\n", tripc_get_errmsg(c));
            }
            break;
        default:
            abort();
    }
}

/**
 * Handle stream event.
 */
void
router_handle_stream(trip_stream_t *s)
{
    switch (trips_status(s))
    {
        case TRIPS_STATUSI_OPEN:
            break;
        case TRIPS_STATUSI_CLOSED:
            break;
        case TRIPS_STATUSI_KILLED:
            break;
        case TRIPS_STATUSI_ERROR:
            break;
        case TRIPS_STATUSO_OPEN:
            break;
        case TRIPS_STATUSO_CLOSED:
            break;
        case TRIPS_STATUSO_KILLED:
            break;
        case TRIPS_STATUSO_ERROR:
            break;
        default:
            abort();
    }
}

/**
 * Handle message event on stream.
 */
static void
router_handle_message(trip_stream_t *s, enum trip_message_status status, size_t len, const unsigned char *buf)
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

static void
router_init(mydata_t *data)
{
    data->running = true;
    data->stop = false;
    data->stopdeadline = triptime_deadline(1000);

    /* Set router to have default presets. */
    trip_router_t *router = data->router = trip_new(TRIP_PRESET_ROUTER);

    /* Set our data. */
    trip_packet_t *reliable = reliable_new();
    data->reliable = reliable;
    trip_setopt(router, TRIPOPT_USER_DATA, data);

    /* Set callbacks. */
    trip_setopt(router, TRIPOPT_SCREEN_CB, router_handle_screen);
    trip_setopt(router, TRIPOPT_CONNECTION_CB, router_handle_connection);
    trip_setopt(router, TRIPOPT_STREAM_CB, router_handle_stream);
    trip_setopt(router, TRIPOPT_MESSAGE_CB, router_handle_message);

    /* Set packet interface. */
    trip_setopt(router, TRIPOPT_PACKET, reliable);

    /* Set security options. */
    /* WARNING: Not recommended to disable all security. */
    trip_setopt(router, TRIPOPT_ALLOW_PLAIN_OPEN, data);
    trip_setopt(router, TRIPOPT_ALLOW_PLAIN_ISIG, data);
    trip_setopt(router, TRIPOPT_ALLOW_PLAIN_OSIG, data);
    trip_setopt(router, TRIPOPT_ALLOW_PLAIN_COMM, data);

    trip_run_init(router);
    trip_start(router);
}

static void
router_destroy(mydata_t *data)
{
    reliable_free(data->reliable);
    trip_free(data->router);
}

int main()
{
    mydata_t data = { 0 };

    router_init(&data);
    printf("Entering run loop...\n");
    uint64_t now;
    int err;
    trip_open_connection(data.router, &data, 0, NULL);
    while (!(err = trip_run(data.router, 100)))
    {
        printf("Timeout...\n");

        now = triptime_now();
        if (now >= data.stopdeadline)
        {
            data.stop = true;
        }

        if (data.stop)
        {
            trip_stop(data.router);
            data.stop = false;
        }
    }
    printf("Err: %s\n", strerror(err));
    printf("Exiting run loop...\n");
    router_destroy(&data);

    return 0;
}

