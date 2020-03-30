#define _GNU_SOURCE
#include <sched.h>

#include "./mods/utils.h"
#include "./mods/stack.h"
#include "./mods/list.h"
#include "./mods/lock.h"
#include "./mm_rdy_list.h"
#include "./mm_sched.h"
#include "./hthread_priv.h"
#include "./hthread_cntl.h"
#include "./hthread_sig.h"

/* Next thread id */
static int _nxt_id = 0;
/* Lock for the global id */
static Lock _lock = LOCK_INITIALIZER;

/**
 * @brief Get next thread id
 *
 * Returns the thread id to be used for the next submitted user thread
 *
 * @return Integer id
 */
static int _get_nxt_id(void) {

    int id;

    /* Acquire the lock */
    lock_acquire(&_lock);

    /* Get the id */
    id = _nxt_id++;

    /* Release the lock */
    lock_release(&_lock);

    return id;
}

/**
 * @brief Start function for one one user threads
 *
 * Actual start function for a one one user thread. This function invokes the
 * start function requested by the user
 *
 * @param[in] arg Not used
 * @return Integer not used
 */
static int _one_one_start(void *arg) {

    HThread hthread;

    /* Get the thread handle */
    hthread = hthread_self();

    /* Get the current context */
    getcontext(ONE_TLS(hthread)->ret_cxt);

    /* If the state of the thread is active */
    if (hthread->state == HTHREAD_STATE_INIT) {

        /* Set the state as inactive */
        hthread->state = HTHREAD_STATE_ACTIVE;

        /* Call the start routine */
        hthread->ret = hthread->start(hthread->arg);

        /* Set the state as inactive */
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
static HThread _one_one_create(void *(*start)(void *), void *arg) {

    HThread hthread;
    void *stack_top;

    /* Allocate the thread control block */
    hthread = BASE_TLS(alloc_mem(struct HThreadOneOne));

    /* Get the thread id */
    hthread->id = _get_nxt_id();

    /* Set the type */
    hthread->type = HTHREAD_TYPE_ONE_ONE;

    /* Set the state */
    hthread->state = HTHREAD_STATE_INIT;

    /* Set the start function */
    hthread->start = start;

    /* Set the start function argument */
    hthread->arg = arg;

    /* Allocate the stack */
    stack_alloc(&ONE_TLS(hthread)->stack);

    /* Allocate the context */
    ONE_TLS(hthread)->ret_cxt = alloc_mem(ucontext_t);

    /* Find the stack top */
    stack_top = ONE_TLS(hthread)->stack.ss_sp + ONE_TLS(hthread)->stack.ss_size;

    /* Create a thread */
    ONE_TLS(hthread)->tid = clone(_one_one_start,
                                  stack_top,
                                  CLONE_VM | CLONE_FS | CLONE_FILES |
                                  CLONE_SIGHAND | CLONE_THREAD |
                                  CLONE_SYSVSEM | CLONE_SETTLS |
                                  CLONE_CHILD_CLEARTID |
                                  CLONE_PARENT_SETTID,
                                  NULL,
                                  &hthread->wait,
                                  hthread,
                                  &hthread->wait);
    /* Check for errors */
    assert(ONE_TLS(hthread)->tid != -1);

    /* Return the thread handle */
    return hthread;
}

/**
 * @brief Start function for many many user threads
 *
 * Actual start function for a many many user thread. This function invokes the
 * start function requested by the user
 */
static void _many_many_start(void) {

    HThread hthread;

    /* Get the thread handle */
    hthread = hthread_self();

    /* Set the thread state as active */
    hthread->state = HTHREAD_STATE_ACTIVE;

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
static HThread _many_many_create(void *(*start)(void *), void *arg) {

    HThread hthread;

    /* Allocate the thread control block */
    hthread = BASE_TLS(alloc_mem(struct HThreadManyMany));

    /* Get the next thread id */
    hthread->id = _get_nxt_id();

    /* Set the type */
    hthread->type = HTHREAD_TYPE_MANY_MANY;

    /* Set the state */
    hthread->state = HTHREAD_STATE_INIT;

    /* Set the start function */
    hthread->start = start;

    /* Set the start function argument */
    hthread->arg = arg;

    /* Initialize the wait word */
    hthread->wait = 1;

    /* Allocate the current context */
    MANY_TLS(hthread)->curr_cxt = alloc_mem(ucontext_t);

    /* Allocate the return context */
    MANY_TLS(hthread)->ret_cxt = alloc_mem(ucontext_t);

    /* Set the current context */
    getcontext(MANY_TLS(hthread)->curr_cxt);

    /* Allocate the stack */
    stack_alloc(&(MANY_TLS(hthread)->curr_cxt->uc_stack));

    /* Set the backlink */
    MANY_TLS(hthread)->curr_cxt->uc_link = MANY_TLS(hthread)->ret_cxt;

    /* Make the context */
    makecontext(MANY_TLS(hthread)->curr_cxt, _many_many_start, 0);

    /* Initialize the pending signals list */
    MANY_TLS(hthread)->sig_list = LIST_INITIALIZER;

    /* Initialize the signal lock */
    MANY_TLS(hthread)->sig_lock = LOCK_INITIALIZER;

    return hthread;
}

/**
 * @brief Free one one thread resources
 *
 * Deallocates all the resources of the thread. Also free the thread local
 * storage for the one one thread
 *
 * @param[in] hthread Thread handle
 * @param[in] free_tcb Whether to free the tcb as well
 */
void _one_one_free(HThread hthread, int free_tcb) {

    /* Check for errors */
    assert(hthread);

    /* Free the stack */
    stack_free(&ONE_TLS(hthread)->stack);

    /* Free the return context */
    free(ONE_TLS(hthread)->ret_cxt);

    /* If tcb is to be freed */
    if (free_tcb) {

        /* Free the thread handle */
        free(hthread);
    }
}

/**
 * @brief Free many many thread resources
 *
 * Deallocates all the resources of the thread. Also free the thread local
 * storage for the many many thread
 *
 * @param[in] hthread Thread handle
 * @param[in] free_tcb Whether to free the tcb as well
 */
void _many_many_free(HThread hthread, int free_tcb) {

    Signal *sig;

    /* Check for errors */
    assert(hthread);

    /* Free the stack */
    stack_free(&(MANY_TLS(hthread)->curr_cxt->uc_stack));

    /* Free the current context */
    free(MANY_TLS(hthread)->curr_cxt);

    /* Free the return context */
    free(MANY_TLS(hthread)->ret_cxt);

    /* Lock the signal list lock */
    lock_acquire(&MANY_TLS(hthread)->sig_lock);

    /* While the list is not empty */
    while (!list_is_empty(&MANY_TLS(hthread)->sig_list)) {

        /* Get the signal */
        sig = list_dequeue(&MANY_TLS(hthread)->sig_list, Signal, sig_mem);

        /* Free the signal */
        free(sig);
    }

    /* Lock the signal list lock */
    lock_release(&MANY_TLS(hthread)->sig_lock);

    /* If tcb is to be freed */
    if (free_tcb) {

        /* Free the thread handle */
        free(hthread);
    }
}

/**
 * @brief Initialize the hybrid thread library
 *
 * Initializes all the required globals. Creates the kernel threads required
 * for the library to schedule the many-many threads
 *
 * @param[in] nb_kthds Number of kernel threads many-many part
 */
void hthread_init(int nb_kthds) {

    /* Check for errors */
    assert(nb_kthds);

    /* Initialize the schedulers */
    mm_sched_init(nb_kthds);
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
    assert((type == HTHREAD_TYPE_ONE_ONE) ||
           (type == HTHREAD_TYPE_MANY_MANY));

    /* Create the thread depending on its type */
    switch (type) {

        case HTHREAD_TYPE_ONE_ONE:

            /* Create a one-one thread */
            hthread = _one_one_create(start, arg);
            break;

        case HTHREAD_TYPE_MANY_MANY:

            /* Create a many-many thread */
            hthread = _many_many_create(start, arg);

            /* Lock the thread list */
            mm_rdy_list_lock();

            /* Add the thread to the list */
            mm_rdy_list_add(hthread);

            /* Unlock the thread list */
            mm_rdy_list_unlock();
            break;

        default:
            break;
    }

    return hthread;
}

/**
 * @brief Join with the target thread
 *
 * Waits for the target thread to complete its execution
 *
 * @param[in] hthread Target user thread to be joined
 * @param[out] ret Pointer to the return value
 */
void hthread_join(HThread hthread, void **ret) {

    /* Check for errors */
    assert(hthread);

    /* Wait while the wait word is not cleared */
    while (!atomic_cas(&hthread->wait, 0, 1));

    /* If the return value is requested */
    if (ret) {

        /* Get the return value from the thread local storage */
        *ret = hthread->ret;
    }

    /* Update the state of the thread */
    hthread->state = HTHREAD_STATE_JOINED;

    /* Depending on the thread type */
    switch (hthread->type) {

        case HTHREAD_TYPE_ONE_ONE:

            /* Free the one one thread paritially */
            _one_one_free(hthread, 0);
            break;

        case HTHREAD_TYPE_MANY_MANY:

            /* Free the many many thread paritially */
            _many_many_free(hthread, 0);
            break;

        default:
            break;
    }

    /* Now do not clear the thread local storage as it might result in
     segmentation fault, as other threads may be spinning on*/
}

/**
 * @brief Exit from thread
 *
 * Exits from the calling user thread and sets the exit return value in its
 * local storage
 *
 * @param[in] ret Return value
 */
void hthread_exit(void *ret) {

    HThread hthread;

    /* Get the thread handle */
    hthread = hthread_self();

    /* Set the return value */
    hthread->ret = ret;

    /* Check if the thread is active */
    if (hthread->state != HTHREAD_STATE_ACTIVE) {

        return;
    }

    /* Change the state to inactive */
    hthread->state = HTHREAD_STATE_INACTIVE;

    /* Exit according to the thread type */
    switch (hthread->type) {

        case HTHREAD_TYPE_ONE_ONE:

            /* Jump to the exit context */
            setcontext(ONE_TLS(hthread)->ret_cxt);
            break;

        case HTHREAD_TYPE_MANY_MANY:

            /* Jump to the exit context */
            setcontext(MANY_TLS(hthread)->ret_cxt);
            break;

        default:
            break;
    }
}

/**
 * @brief Return thread handle
 *
 * Reads and returns the value of the FS register which stores the address of
 * the thread local storage for the currently running thread
 *
 * @return Thread handle
 * @note The value returned by the multiple invokation of the same thread will
 *       result in the same address
 */
HThread hthread_self(void) {

    /* Read the value of the FS register */
    return BASE_TLS(get_fs());
}

/**
 * @brief Initialize the hybrid thread library
 *
 * Deinitializes the global data structures if any and frees the allocated
 * resources
 */
void hthread_deinit(void) {

    /* Deinitialize the schedulers */
    mm_sched_deinit();
}
