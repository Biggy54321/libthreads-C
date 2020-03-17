#include <unistd.h>

#include "./timer.h"

/**
 * Convert milliseconds to nanoseconds
 */
#define _MILLISECS_TO_NANOSECS(msec) ((msec) * 1000000)
/**
 * Get seconds in milliseconds
 */
#define _SECS_IN_MILLISECS(msec) (_MILLISECS_TO_NANOSECS(msec) / 1000000000)
/**
 * Get remainder nanoseconds in total milliseconds
 */
#define _NANOSECS_IN_MILLISECS(msec) (_MILLISECS_TO_NANOSECS(msec) % 1000000000)

/**
 * @brief Initialize the timer for the given event
 * @param[out] timer Pointer to the timer instance
 * @param[in] event Event to be executed after the timer expires
 * @param[in] millisecs Timer out expiration period in milliseconds
 */
void timer_set(Timer *timer, void (*event_func)(int), long millisecs) {

    /* Initialize the interval of timeout */
    timer->interval.it_interval.tv_nsec = 0;
    timer->interval.it_interval.tv_sec = 0;
    /* Initialize the expiration period of timeout */
    timer->interval.it_value.tv_nsec = _NANOSECS_IN_MILLISECS(millisecs);
    timer->interval.it_value.tv_sec = _SECS_IN_MILLISECS(millisecs);

    /* Initialize the signal event */
    timer->event.sigev_notify = SIGEV_THREAD_ID;
    timer->event.sigev_signo = SIGALRM;
    timer->event._sigev_un._tid = gettid();

    /* Initialize the signal handler i.e. event function */
    signal(SIGALRM, event_func);
}

/**
 * @brief Starts the timer according to the initialized values
 * @param[in] timer Pointer to the timer instance
 */
void timer_start(Timer *timer) {

    /* Allocate the timer */
    timer_create(CLOCK_REALTIME, &timer->event, &timer->timerid);

    /* Set the timer */
    timer_settime(timer->timerid, 0, &timer->interval, NULL);
}

/**
 * @brief Starts the timer according to the initialized values
 * @param[in] timer Pointer to the timer instance
 */
void timer_stop(Timer *timer) {

    /* Deallocate the timer */
    timer_delete(timer->timerid);
}
