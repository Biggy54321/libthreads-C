#include <time.h>

#include <signal.h>

/**
 * One shot timer
 */
typedef struct _Timer {

    /* Timer structure */
    timer_t timerid;

    /* Interval timeout structure */
    struct itimerspec interval;

    /* Signal event structure */
    struct sigevent event;

} Timer;

void timer_set(Timer *timer, void (*event_func)(int), long millisecs);

void timer_start(Timer *timer);

void timer_stop(Timer *timer);
