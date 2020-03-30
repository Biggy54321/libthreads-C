#ifndef _MM_SCHED_H_
#define _MM_SCHED_H_

#include <signal.h>

#include "./mods/list.h"

/**
 * Scheduler state
 */
typedef struct Scheduler {

    /* Kernel thread id */
    int tid;

    /* Wait word */
    int wait;

    /* Thread stack */
    stack_t stack;

    /* List member */
    ListMember sched_mem;

} Scheduler;

#define MM_SCHED_TIME_SLICE (10u)

void mm_sched_init(int nb_scheds);

void mm_sched_deinit(void);

#endif
