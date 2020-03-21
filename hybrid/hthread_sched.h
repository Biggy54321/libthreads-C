#ifndef _HTHREAD_SCHED_H_
#define _HTHREAD_SCHED_H_

/* Time slice for each user thread in milliseconds */
#define TIME_SLICE_ms (10u)

void hthread_sched_start(void);

int hthread_sched_dispatch(void *arg);

void hthread_sched_yield(int arg);

void hthread_sched_stop(void);

#endif
