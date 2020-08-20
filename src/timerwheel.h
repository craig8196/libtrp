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
 * @file timerwheel.h
 * @author Craig Jacobson
 * @brief Timer wheel for efficient timeouts.
 *
 * TODO add feature to detect entropy or wasted memory for cancelled timeouts.
 */
#ifndef _LIBTRP_TIMERWHEEL_H_
#define _LIBTRP_TIMERWHEEL_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdbool.h>
#include <stdint.h>


typedef void timer_cb_t(void *);

typedef struct timer_entry_s
{
    uint64_t deadline;
    void *data;
    timer_cb_t *cb;
    struct timer_entry_s *next;
    bool canceled;
} timer_entry_t;

typedef struct timerwheel_s
{
    timer_entry_t *head;
} timerwheel_t;

void
timerwheel_init(timerwheel_t *tw);
void
timerwheel_destroy(timerwheel_t *tw);
timer_entry_t *
timerwheel_walk(timerwheel_t *tw);
timer_entry_t *
timerwheel_walk_with(timerwheel_t *tw, uint64_t now);
timer_entry_t *
timerwheel_add(timerwheel_t *tw, int timeout, void *data, timer_cb_t *cb);
void
timerwheel_cancel(timer_entry_t *te);


#ifdef __cplusplus
}
#endif
#endif /* _LIBTRP_TIMERWHEEL_H_ */


