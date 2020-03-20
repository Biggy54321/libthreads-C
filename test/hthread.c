#define _GNU_SOURCE
#include <sched.h>
#include <assert.h>
#include <stdlib.h>

#include "./lib/utils.h"
#include "./lib/lock.h"
#include "./lib/stack.h"
#include "./hthread.h"
#include "./hthread_list.h"
#include "./hthread_kernel.h"

/* Next thread id */
static int _nxt_tid;
/* Lock for the global id */
static Lock _id_lock;

/**
 * @brief Initialize the hybrid thread library
 *
 * Initializes all the required globals. Creates the kernel threads required
 * for the library to schedule the many-many threads
 *
 * @param[in] nb_kthds Number of kernel threads many-many part
 */
void hthread_init(int nb_kthds) {

    /* Initialize the thread list */
    hthread_list_init();

    /* Initialize the first thread id */
    _nxt_tid = 0;
    /* Initialize the thread id lock */
    lock_init(&_id_lock);

    /* Initialize the kernel threads */
    hthread_kernel_threads_init(nb_kthds);
}

static int _one_one_start(void *arg) {

    struct _HThreadOneOne *hthread;

    /* Get the thread handle */
    hthread = (struct _HThreadOneOne *)get_fs();

    /* Get the current context */
    getcontext(hthread->ret_cxt);

    /* If the state of the thread is active */
    if (hthread->state == HTHREAD_STATE_ACTIVE) {

        /* Call the start routine */
        hthread->ret = hthread->start(hthread->arg);

        /* Set the state to inactive */
        hthread->state = HTHREAD_STATE_INACTIVE;
    }

    return 0;
}

/**
 * @brief Creates a one-one thread
 * @param[in] start Start routine
 * @param[in] arg Argument for start routine
 * @return Thread handle
 */
static HThread _one_one_thread_create(void *(*start)(void *), void *arg) {

    struct _HThreadOneOne *hthread;

    /* Allocate the thread control block */
    hthread = (struct _HThreadOneOne *)malloc(sizeof(struct _HThreadOneOne));
    /* Check for errors */
    assert(hthread);

    /* Lock the next id */
    lock_acquire(&_id_lock);

    /* Set the thread id */
    hthread->id = _nxt_tid++;

    /* Release the next id */
    lock_release(&_id_lock);

    /* Set the type */
    hthread->type = HTHREAD_TYPE_ONE_ONE;

    /* Set the state */
    hthread->state = HTHREAD_STATE_ACTIVE;

    /* Set the start function */
    hthread->start = start;

    /* Set the start function argument */
    hthread->arg = arg;

    /* Allocate the stack */
    stack_alloc(&hthread->stack);

    /* Allocate the context */
    hthread->ret_cxt = (ucontext_t *)malloc(sizeof(ucontext_t));
    /* Check for errors */
    assert(hthread->ret_cxt);

    /* Create a thread */
    hthread->tid = clone(_one_one_start,
                         hthread->stack.ss_sp + hthread->stack.ss_size,
                         CLONE_VM | CLONE_FS | CLONE_FILES |
                         CLONE_SIGHAND | CLONE_THREAD |
                         CLONE_SYSVSEM | CLONE_SETTLS,
                         hthread,
                         NULL,
                         hthread,
                         NULL);

    /* Return the thread handle */
    return (HThread)hthread;
}

void _many_many_start(void) {

    struct _HThreadManyMany *hthread;

    /* Get the thread handle */
    hthread = (struct _HThreadManyMany *)get_fs();

    /* Check for errors */
    assert(hthread);

    /* Call the start function */
    hthread->ret = hthread->start(hthread->arg);

    /* Switch the state */
    hthread->state = HTHREAD_STATE_INACTIVE;
}

/**
 * @brief Creates a many-many thread
 * @param[in] start Start routine
 * @param[in] arg Argument for start routine
 * @return Thread handle
 */
static HThread _many_many_thread_create(void *(*start)(void *), void *arg) {

    struct _HThreadManyMany *hthread;

    /* Allocate the thread control block */
    hthread = (struct _HThreadManyMany *)malloc(sizeof(struct _HThreadManyMany));
    /* Check for errors */
    assert(hthread);

    /* Lock the next id */
    lock_acquire(&_id_lock);

    /* Set the thread id */
    hthread->id = _nxt_tid++;

    /* Release the next id */
    lock_release(&_id_lock);

    /* Set the type */
    hthread->type = HTHREAD_TYPE_MANY_MANY;

    /* Set the state */
    hthread->state = HTHREAD_STATE_ACTIVE;

    /* Set the start function */
    hthread->start = start;

    /* Set the start function argument */
    hthread->arg = arg;

    /* Allocate the current context */
    hthread->curr_cxt = (ucontext_t *)malloc(sizeof(ucontext_t));
    /* Check for errors */
    assert(hthread->curr_cxt);

    /* Allocate the return context */
    hthread->ret_cxt = (ucontext_t *)malloc(sizeof(ucontext_t));
    /* Check for errors */
    assert(hthread->ret_cxt);

    /* Set the current context */
    getcontext(hthread->curr_cxt);

    /* Allocate the stack */
    stack_alloc(&(hthread->curr_cxt->uc_stack));

    /* Set the backlink */
    hthread->curr_cxt->uc_link = hthread->ret_cxt;

    /* Make the context */
    makecontext(hthread->curr_cxt, _many_many_start, 0);
}

/**
 * @brief Create a user thread
 *
 * Creates the user thread depending on the type of mapping.
 *
 * @param[in] start Start routine
 * @param[in] arg Argument for start routine
 * @param[in] type Type of mapping (use enum in definitions file)
 * @return Thread handle
 */
HThread hthread_create(void *(*start)(void *), void *arg, HThreadType type) {

    HThread hthread;

    /* Check for errors */
    assert(start);
    assert((type == HTHREAD_TYPE_ONE_ONE) || (type == HTHREAD_TYPE_MANY_MANY));

    /* Create the thread depending on its type */
    if (type == HTHREAD_TYPE_ONE_ONE) {

        /* Create a one-one thread */
        hthread = _one_one_thread_create(start, arg);
    } else {

        /* Create a many-many thread */
        hthread = _many_many_thread_create(start, arg);
        /* Add the thread to the list */
        hthread_list_add(hthread);
    }
}





















void hthread_join(HThread hthread, void **ret);

void hthread_exit(void *ret);

HThread hthread_self(void);

void hthread_mutex_init(HThreadMutex *mutex);

void hthread_mutex_lock(HThreadMutex *mutex);

void hthread_mutex_unlock(HThreadMutex *mutex);

/**
 * @brief Initialize the hybrid thread library
 *
 * Deinitializes the global data structures if any and frees the allocated
 * resources
 */
void hthread_deinit(void) {

    HThread hthread;

    /* Deinitialize the kernel threads */
    hthread_kernel_threads_deinit();

    /* Deinitialize the user threads from the thread list */
    while (!hthread_list_is_empty()) {

        /* Get the thread from the list */
        hthread = hthread_list_get();

        /* Free the thread */
        ??????????????????????
    }
}

