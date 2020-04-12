#ifndef _MMSCHED_H_
#define _MMSCHED_H_

#include <signal.h>

#include "./mods/list.h"

/**
 * Scheduler state
 */
typedef struct Scheduler {

    /* Kernel thread id */
    int ktid;

    /* Wait word */
    int wait;

    /* Thread stack */
    stack_t stack;

    /* List member */
    ListMember sll_mem;

} Scheduler;

/* Many-many thread time slice (in milli seconds) */
#define MMSCHED_TIME_SLICE_ms (10u)

void mmsched_init(int nb_scheds);

void mmsched_deinit(void);

#endif
