#ifndef _HTHREAD_PRIV_H_
#define _HTHREAD_PRIV_H_

#include <ucontext.h>
#include <signal.h>

#include "./mods/list.h"
#include "./mods/lock.h"
#include "./hthread_pub.h"

/**
 * One-one TLS
 */
struct HThreadOneOne {

    /* Base TLS members */
    HTHREAD_BASE_MEMBERS;

    /* Kernel thread id */
    int tid;

    /* Thread stack */
    stack_t stack;

    /* Return context */
    ucontext_t *ret_cxt;

};

/**
 * Many-many TLS
 */
struct HThreadManyMany {

    /* Base TLS members */
    HTHREAD_BASE_MEMBERS;

    /* Current context */
    ucontext_t *curr_cxt;

    /* Return context */
    ucontext_t *ret_cxt;

    /* Pending signals list */
    List sig_list;

    /* Lock for the above list */
    Lock sig_lock;

    /* Ready list link */
    ListMember rdy_list_mem;

};

/* Upcast the general thread handle to one one thread handle */
#define ONE_TLS(hthread)       ((struct HThreadOneOne *)(hthread))
/* Upcast the general thread handle to many many thread handle */
#define MANY_TLS(hthread)      ((struct HThreadManyMany *)(hthread))
/* Downcast the any specific handle to general handle */
#define BASE_TLS(hthread)      ((struct HThread *)(hthread))

#endif
