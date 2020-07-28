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
#include "reliable_packet.h"


/* DATA */

typedef struct
{
    bool running;
    bool stop;
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
 * Routing.
 * Source address.
 * Source address binary.
 * Source address as string.
 * Signature.
 * Decrypt OPEN keys.
 */
void
router_handle_screen(trip_screen_t *screen)
{
    screen->allow = true;
}

/**
 * Handle connection event.
 */
void
router_handle_connection(trip_connection_t *c)
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
router_handle_stream(trip_stream_t *s)
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

    trip_router_t *router = data->router = trip_new(TRIP_PRESET_ROUTER);
    trip_packet_t *reliable = reliable_new();
    data->reliable = reliable;
    trip_setopt(router, TRIPOPT_USER_DATA, data);
    trip_setopt(router, TRIPOPT_SCREEN_CB, router_handle_screen);
    trip_setopt(router, TRIPOPT_CONNECTION_CB, router_handle_connection);
    trip_setopt(router, TRIPOPT_STREAM_CB, router_handle_stream);
    trip_setopt(router, TRIPOPT_MESSAGE_CB, router_handle_message);
    trip_setopt(router, TRIPOPT_PACKET, router_handle_message);
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
    while (data.running)
    {
        if (data.stop)
        {
            trip_stop(data.router);
        }
        else
        {
            trip_run(data.router, 10000);
        }
    }
    router_destroy(&data);

    return 0;
}

