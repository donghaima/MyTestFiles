/*
 * Test many timers with different durations.
 *
 * Compile with:
 *  gcc -g -static -I/users/doma/local/include -o many_timer many_timer.c -L/users/doma/local/lib -levent
 */

#include <sys/types.h>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/time.h>

#include <event.h>


/* The structure should be able to allow us to check if the timer is 
   fired correctly, i.e. if timer_duration == (expired_time - start_time) */
struct test_timer
{
    uint32_t id;
    time_t timer_duration;     /* configured timer duration */
    time_t start_time;         /* recorded when the timer is being setup */
    time_t expired_time;       /* recorded from the callback routine,
                                  i.e. when the timer is expired */
    struct event *p_timeout;   /* so that we can re-arm this timer */
};


#define REARM 1
void timeout_cb(int fd, short event, void *arg)
{
    struct timeval tv;
    struct test_timer *tt = arg;

    assert(tt);

    tt->expired_time = time(NULL);

    printf("Timer %d: duration=%d, called at %d: %d -- event=%d\n",
           tt->id, tt->timer_duration, tt->expired_time, 
           tt->expired_time - tt->start_time, event);

    if(REARM)
    {
        /* Rest the timer start time */
        tt->start_time = time(NULL);

        timerclear(&tv);
        tv.tv_sec = tt->timer_duration;
        event_add(tt->p_timeout, &tv);
    }
}


#define MAX_TEST_TIMER   10000
#define MAX_TIMEOUT      30

int main (int argc, char **argv)
{
    struct test_timer tt[MAX_TEST_TIMER];

    struct event timeout;
    struct timeval tv;
    int i;

    /* Initalize the event library */
    event_init();

    /* Seed rand() */
    srand( (unsigned)time( NULL ) );

    /* Set up all the test timers */
    for (i=0; i<MAX_TEST_TIMER; i++)
    {
        tt[i].p_timeout = (struct event*)malloc(sizeof(struct event));
        tt[i].id = i;
        /* Set the timer duration to a random value in range [1, MAX_TIMEOUT] */
        tt[i].timer_duration = 1 + (int) ((float)MAX_TIMEOUT * rand() / (RAND_MAX + 1.0));
        tt[i].start_time = time(NULL);

        /* Initalize the timer */
        evtimer_set(tt[i].p_timeout, timeout_cb, &tt[i]);

        timerclear(&tv);
        tv.tv_sec = tt[i].timer_duration;
        event_add(tt[i].p_timeout, &tv);

        printf("Added Timer %d: duration=%d\n", tt[i].id, tt[i].timer_duration);
    }
 
    event_dispatch();

    return (0);
}

