#include <assert.h>

#include "./utils.h"
#include "./timer.h"

/* Convert milliseconds to nanoseconds */
#define _MILLISECS_TO_NANOSECS(msec) ((msec) * 1000000)
/* Get seconds in milliseconds */
#define _SECS_IN_MILLISECS(msec)     (_MILLISECS_TO_NANOSECS(msec) / 1000000000)
/* Get remainder nanoseconds in total milliseconds */
#define _NANOSECS_IN_MILLISECS(msec) (_MILLISECS_TO_NANOSECS(msec) % 1000000000)

/**
 * @brief Initialize the timer
 *
 * Sets the required handler to be executed after the given time expires. The
 * time should be specified in milliseconds. The function uses SIGALRM signal
 * and hence should be prevented from use internally
 *
 * @param[out] timer Pointer to the timer instance
 * @param[in] event Event to be executed after the timer expires
 * @param[in] millisecs Timer out expiration period in milliseconds
 */
void timer_set(Timer *timer, void (*event_func)(int), long millisecs) {

    struct sigaction sig_act;

    /* Check for errors */
    assert(timer);
    assert(event_func);

    /* Initialize the interval of timeout */
    timer->interval.it_interval.tv_nsec = 0;
    timer->interval.it_interval.tv_sec = 0;
    /* Initialize the expiration period of timeout */
    timer->interval.it_value.tv_nsec = _NANOSECS_IN_MILLISECS(millisecs);
    timer->interval.it_value.tv_sec = _SECS_IN_MILLISECS(millisecs);

    /* Initialize the signal event */
    timer->event.sigev_notify = SIGEV_THREAD_ID;
    timer->event.sigev_signo = SIGALRM;
    timer->event._sigev_un._tid = KERNEL_THREAD_ID;

    /* Initialize the signal handler i.e. event function */
    sig_act.sa_handler = event_func;
    /* Initialize the signal mask while the function to all blocks */
    sigfillset(&sig_act.sa_mask);
    /* Set the flags to none */
    sig_act.sa_flags = 0;
    /* Set the action for the signal */
    sigaction(SIGALRM, &sig_act, NULL);
}

/**
 * @brief Starts the timer
 *
 * It starts the previously set timer, by allocating a timer corresponding
 * to previously set event
 *
 * @param[in] timer Pointer to the timer instance
 */
void timer_start(Timer *timer) {

    /* Check for errors */
    assert(timer);

    /* Allocate the timer */
    timer_create(CLOCK_REALTIME, &timer->event, &timer->timerid);

    /* Set the timer */
    timer_settime(timer->timerid, 0, &timer->interval, NULL);
}

/**
 * @brief Stop the timer
 *
 * Deallocates a previously allocated timer. The timer should have been
 * allocated using timer_start() first
 *
 * @param[in] timer Pointer to the timer instance
 */
void timer_stop(Timer *timer) {

    /* Check for errors */
    assert(timer);

    /* Deallocate the timer */
    timer_delete(timer->timerid);
}
