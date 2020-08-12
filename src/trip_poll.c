
#include "trip_poll.h"

#include <errno.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <string.h>

#include "time.h"
#include "trip.h"
#include "libtrp_memory.h"
#include "util.h"


_trip_poll_t *
_trip_poll_new()
{
    _trip_poll_t *w = tripm_alloc(sizeof(_trip_poll_t));

    if (!w)
    {
        return w;
    }

    int c = 0;

    w->efd = epoll_create1(0);
    w->deadline = triptime_now();
    if (-1 == w->efd)
    {
        c = EINVAL;
    }

    if (c)
    {
        if (w)
        {
            tripm_free(w);
        }
        w = NULL;
    }

    return w;
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
_trip_watch_cb(trip_router_t *_r, int fd, int events, void * UNUSED(data))
{
    trip_torouter(r, _r);
    _trip_poll_t *w = r->poll;

    int c;
    struct epoll_event ev = { 0 };

    ev.events = _trip_fd_events_to_epoll(events);
    ev.data.fd = fd;

    if (TRIP_REMOVE != events)
    {
        c = epoll_ctl(w->efd, EPOLL_CTL_ADD, fd, &ev);
        if (EEXIST == c)
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
_trip_timeout_cb(trip_router_t *_r, long timeout)
{
    trip_torouter(r, _r);
    _trip_poll_t *w = r->poll;
    w->deadline = triptime_deadline(timeout);
}

int
trip_run_init(trip_router_t *_r)
{
    trip_torouter(r, _r);

    int c = 0;

    if (!r->poll)
    {
        r->watch = _trip_watch_cb;
        r->timeout = _trip_timeout_cb;
        r->poll = _trip_poll_new();
        if (!r->poll)
        {
            r->watch = NULL;
            r->timeout = NULL;
            c = ENOMEM;
        }
    }

    return c;
}

int
trip_run(trip_router_t *_r, int maxtimeout)
{
    trip_torouter(r, _r);

    int c = 0;

    do
    {
        if (!r->poll)
        {
            c = EINVAL;
            break;
        }

        if (_TRIPR_STATE_END == r->state)
        {
            c = EHOSTDOWN;
            break;
        }

        if (_TRIPR_STATE_ERROR == r->state)
        {
            c = r->error;
            break;
        }

        _trip_poll_t *w = r->poll;

        uint64_t now = triptime_now();
        int timeout = maxtimeout;
        printf("Max timeout: %d\n", timeout);
        {
#if 1
            int tmp = timerwheel_get(&r->wheel);
#else
            int tmp = triptime_timeout(w->deadline, now);
#endif
            printf("Othertimeout: %d\n", tmp);
            if (tmp < timeout)
            {
                timeout = tmp;
            }
        }
        uint64_t deadline = triptime_deadline(timeout);
        
        for (;;)
        {
            printf("Max timeout: %d\n", timeout);
            struct epoll_event eventlist[_TRIP_MAX_EVENTS];
            int nfds = epoll_wait(w->efd, eventlist, _TRIP_MAX_EVENTS, timeout);

            if (-1 == nfds)
            {
                c = errno;
                break;
            }
            else if (0 == nfds)
            {
                c = trip_action(_r, TRIP_SOCKET_TIMEOUT, 0);
            }
            else
            {
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

            timeout = triptime_timeout(deadline, now);
        }
    } while (false);

    return c;
}

