#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

/**
 * Time slice for each kernel thread in milliseconds
 */
#define KERNEL_THREAD_TIME_SLICE_ms (2000u)

int scheduler(void *argument);

#endif
