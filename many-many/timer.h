#ifndef _TIMER_H_
#define _TIMER_H_

#include "./types.h"

void timer_set(Timer *timer, void (*event_func)(int), long millisecs);

void timer_start(Timer *timer);

void timer_stop(Timer *timer);

#endif
