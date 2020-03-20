#ifndef _SCHED_H_
#define _SCHED_H_

#include "./types.h"

/* Number of kernel threads in the model */
#define NB_OF_SCHEDS (1u)
/* Time slice of user thread on each scheduler */
#define TIME_SLICE_ms (2000u)

void sched_init(void);

void sched_deinit(void);

#endif
