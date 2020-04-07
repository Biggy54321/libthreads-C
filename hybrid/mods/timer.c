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
 * @param[in] action Signal action after the timeout
 * @param[in] millisecs Timer out expiration period in milliseconds
 * @return 0 if success
 * @return -1 if failure
 */
int timer_set(Timer *timer, struct sigaction action, long millisecs) {

    /* Check for errors */
    if (!timer) {

        return -1;
    }

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

    /* Set the required action for the given timeout */
    if (sigaction(SIGALRM, &action, NULL) == -1) {

        return -1;
    }

    return 0;
}

/**
 * @brief Starts the timer
 *
 * It starts the previously set timer, by allocating a timer corresponding
 * to previously set event
 *
 * @param[in] timer Pointer to the timer instance
 * @return 0 if success
 * @return -1 if failure
 */
int timer_start(Timer *timer) {

    /* Check for errors */
    if (!timer) {

        return -1;
    }

    /* Allocate the timer */
    if (timer_create(CLOCK_REALTIME, &timer->event, &timer->timerid) == -1) {

        return -1;
    }

    /* Set the timer */
    if (timer_settime(timer->timerid, 0, &timer->interval, NULL) == -1) {

        return -1;
    }

    return 0;
}

/**
 * @brief Stop the timer
 *
 * Deallocates a previously allocated timer. The timer should have been
 * allocated using timer_start() first
 *
 * @param[in] timer Pointer to the timer instance
 * @return 0 if success
 * @return -1 if failure
 */
int timer_stop(Timer *timer) {

    /* Check for errors */
    if (!timer) {

        return -1;
    }

    /* Deallocate the timer */
    if (timer_delete(timer->timerid) == -1) {

        return -1;
    }

    return 0;
}
