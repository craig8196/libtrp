
#include "timerwheel.h"

#include <stddef.h>

#include "libtrp_memory.h"
#include "time.h"


#define DEBUG_TIMERWHEEL (1)

#if DEBUG_TIMERWHEEL
#include <stdio.h>
#endif

void
timerwheel_init(timerwheel_t *tw)
{
    tw->head = NULL;
}

void
timerwheel_destroy(timerwheel_t *tw)
{
    timer_entry_t *curr = tw->head;
    timer_entry_t *next = NULL;

    while (curr)
    {
        next = curr->next;
        tripm_free(curr);
        curr = next;
    }

    tw->head = NULL;
}

int
timerwheel_get(timerwheel_t *tw)
{
    int timeout = 1024;
    uint64_t now = triptime_now();
    timer_entry_t **prev = &tw->head;
    timer_entry_t *curr = tw->head;
    timer_entry_t *next = NULL;

    while (curr)
    {
        next = curr->next;
        if (now >= curr->deadline || curr->canceled)
        {
            /* Resolve */
            *prev = next;
            if (curr->cb && !curr->canceled)
            {
                curr->cb(curr->data);
            }

            tripm_free(curr);

            if (*prev != next)
            {
                /* Value inserted. */
                next = *prev;
            }
        }
        else
        {
            /* Skip. */
            int tmptimeout = triptime_timeout(curr->deadline, now);
            if (tmptimeout < timeout)
            {
                timeout = tmptimeout;
            }
            prev = &curr->next;
        }
        curr = next;
    }

    return timeout;
}

timer_entry_t *
timerwheel_add(timerwheel_t *tw, int timeout, void *data, timer_cb_t *cb)
{
#if DEBUG_TIMERWHEEL
    printf("%s: timeout(%d)\n", __func__, timeout);
#endif
    timer_entry_t *te = tripm_alloc(sizeof(timer_entry_t));

    if (te)
    {
        te->deadline = triptime_deadline(timeout);
        te->data = data;
        te->cb = cb;
        te->next = tw->head;
        te->canceled = false;

        tw->head = te;
    }

    return te;
}

void
timerwheel_cancel(timer_entry_t *te)
{
    te->canceled = true;
}


