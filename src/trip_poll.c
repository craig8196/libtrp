
#include "trip_poll.h"

#include <errno.h>
#include <sys/epoll.h>
#include <string.h>

#include "time.h"
#include "trip.h"
#include "libtrp_memory.h"
#include "util.h"


#define DEBUG_TRIPPOLL (1)
#if DEBUG_TRIPPOLL
#include <stdio.h>
#endif


static _trip_poll_t *
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

static int
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

/**
 * @return The next timeout for the next call to epoll.
 */
static int
next_timeout(_trip_router_t *r, int defaulttimeout, uint64_t now)
{
    if (r->poll->deadline <= now)
    {
        r->poll->deadline = TRIPTIME_END;
        return 0;
    }
    else if (r->mindeadline >= (now + 1024))
    {
        return 1024;
    }
    else
    {
        int timeout = (int)(r->poll->deadline - now);
        return timeout < defaulttimeout ? timeout : defaulttimeout;
    }
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

#if DEBUG_TRIPPOLL
    if (c)
    {
        printf("Error on epoll (%d): %s", c, strerror(c));
    }
#endif
}

/**
 * The framework should always be giving the nearest timeout.
 */
void
_trip_timeout_cb(trip_router_t *_r, long timeout)
{
    trip_torouter(r, _r);
    _trip_poll_t *w = r->poll;
    w->deadline = triptime_deadline(timeout);
}

/**
 * Sets the callbacks for the router and initializes
 * the polling fd.
 */
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

/**
 * Drive communications forward.
 *
 * ERRORS:
 * >EINVAL - likely trip_run_init was not called, or router has more error info
 * >EHOSTDOWN - router is in END state
 * >EINTR - epoll interrupted by signal handler, disable signals to avoid
 *          or check for this error
 * >other - check router for error, otherwise was error with epoll_wait
 *
 * @param maxtimeout - Zero or positive; negative indicates indefinite timeout.
 * @return Zero on success; errno otherwise.
 */
int
trip_run(trip_router_t *_r, int timeout)
{
    trip_torouter(r, _r);

    int c = 0;

    do
    {
        /* Check that we are setup to perform epolling. */
        if (!r->poll)
        {
            c = EINVAL;
            break;
        }

        /* Make sure that we not in an unproductive state. */
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

        /* Save the timeout from the user as a deadline. */
        uint64_t now = triptime_now();
        uint64_t deadline = timeout < 0 ? TRIPTIME_END : triptime_deadline(timeout);
        timeout = TRIPTIME_END == deadline ? 1024 : timeout;
        timeout = next_timeout(r, timeout, now);
#if DEBUG_TRIPPOLL
        printf("Next timeout: %d\n", timeout);
#endif
        
        for (;;)
        {
            /* Call epoll. */
            struct epoll_event eventlist[_TRIP_MAX_EVENTS];
            int nfds = epoll_wait(w->efd, eventlist, _TRIP_MAX_EVENTS, timeout);

            if (-1 == nfds)
            {
                /* Error occurred. */
                c = errno;
                break;
            }
            else if (0 == nfds)
            {
                /* Call timeout action. */
                c = trip_timeout(_r);
            }
            else
            {
                /* Call action for each file descriptor. */
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

            /* Break outer loop. */
            if (c)
            {
                break;
            }

            /* Update time. */
            now = triptime_now();
            if (now >= deadline && TRIPTIME_END != deadline)
            {
                /* Done, leave function. */
                break;
            }

            /* Resolve next timeout. */
            timeout = TRIPTIME_END == deadline ? 1024 : triptime_timeout(deadline, now);
            timeout = next_timeout(r, timeout, now);
        }
    } while (false);

    return c;
}

