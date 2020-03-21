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
static int _nxt_id;
/* Lock for the global id */
static Lock _lock;

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
    hthread = BASE(get_fs());

    /* Check for errors */
    assert(hthread);

    /* Get the current context */
    getcontext(ONE_ONE(hthread)->ret_cxt);

        /* If the state of the thread is active */
    if (hthread->state == HTHREAD_STATE_INIT) {

        /* Set the thread state to active */
        hthread->state = HTHREAD_STATE_ACTIVE;

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
static HThread _one_one_create(void *(*start)(void *), void *arg) {

    HThread hthread;
    void *stack_top;

    /* Allocate the thread control block */
    hthread = BASE(malloc(HTHREAD_ONE_ONE_TLS_SIZE));
    /* Check for errors */
    assert(hthread);

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
    stack_alloc(&ONE_ONE(hthread)->stack);

    /* Allocate the context */
    ONE_ONE(hthread)->ret_cxt = (ucontext_t *)malloc(sizeof(ucontext_t));
    /* Check for errors */
    assert(ONE_ONE(hthread)->ret_cxt);

    /* Find the stack top */
    stack_top = ONE_ONE(hthread)->stack.ss_sp + ONE_ONE(hthread)->stack.ss_size;

    /* Create a thread */
    ONE_ONE(hthread)->tid = clone(_one_one_start,
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
    hthread = BASE(get_fs());

    /* Check for errors */
    assert(hthread);

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
    hthread = BASE(malloc(HTHREAD_MANY_MANY_TLS_SIZE));
    /* Check for errors */
    assert(hthread);

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
    MANY_MANY(hthread)->curr_cxt = (ucontext_t *)malloc(sizeof(ucontext_t));
    /* Check for errors */
    assert(MANY_MANY(hthread)->curr_cxt);

    /* Allocate the return context */
    MANY_MANY(hthread)->ret_cxt = (ucontext_t *)malloc(sizeof(ucontext_t));
    /* Check for errors */
    assert(MANY_MANY(hthread)->ret_cxt);

    /* Set the current context */
    getcontext(MANY_MANY(hthread)->curr_cxt);

    /* Allocate the stack */
    stack_alloc(&(MANY_MANY(hthread)->curr_cxt->uc_stack));

    /* Set the backlink */
    MANY_MANY(hthread)->curr_cxt->uc_link = MANY_MANY(hthread)->ret_cxt;

    /* Make the context */
    makecontext(MANY_MANY(hthread)->curr_cxt, _many_many_start, 0);

    return BASE(hthread);
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
    stack_free(&ONE_ONE(hthread)->stack);

    /* Free the return context */
    free(ONE_ONE(hthread)->ret_cxt);

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

    /* Check for errors */
    assert(hthread);

    /* Free the stack */
    stack_free(&(MANY_MANY(hthread)->curr_cxt->uc_stack));

    /* Free the current context */
    free(MANY_MANY(hthread)->curr_cxt);

    /* Free the return context */
    free(MANY_MANY(hthread)->ret_cxt);

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

    /* Initialize the first thread id */
    _nxt_id = 0;

    /* Initialize the thread id lock */
    lock_init(&_lock);

    /* Initialize the thread list */
    hthread_list_init();

    /* Initialize the kernel threads */
    hthread_kernel_threads_init(nb_kthds);
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

            /* Add the thread to the list */
            hthread_list_add(hthread);
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
    hthread = BASE(get_fs());

    /* Check for error */
    assert(hthread);

    /* Set the return value */
    hthread->ret = ret;

    /* Change the thread state */
    hthread->state = HTHREAD_STATE_INACTIVE;

    /* Exit according to the thread type */
    switch (hthread->type) {

        case HTHREAD_TYPE_ONE_ONE:

            /* Jump to the exit context */
            setcontext(ONE_ONE(hthread)->ret_cxt);
            break;

        case HTHREAD_TYPE_MANY_MANY:

            /* Jump to the exit context */
            setcontext(MANY_MANY(hthread)->ret_cxt);
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

    return BASE(get_fs());
}

/**
 * @brief Kill a thread
 *
 * Send a signal to the target thread
 *
 * @param[in] hthread Target thread handle
 * @param[in] sig_num Signal number
 */
void hthread_kill(HThread hthread, int sig_num) {

    /* Check for errors */
    assert(hthread);

    /* Depending on the target thread type */
    switch (hthread->type) {

        case HTHREAD_TYPE_ONE_ONE:

            /* Wait till the thread becomes runnable */
            while (hthread->state != HTHREAD_STATE_ACTIVE);

            /* Send the signal to the thread */
            tgkill(getpid(), ONE_ONE(hthread)->tid, sig_num);
            break;

        case HTHREAD_TYPE_MANY_MANY:

            /*  */
            break;

        default:
            break;
    }
}

/**
 * @brief Initialize the mutex lock
 *
 * Initializes the mutex lock to the base value. When the lock is initialized
 * it is first not acquired
 *
 * @param[in] mutex Pointer to the mutex instance
 */
void hthread_mutex_init(HThreadMutex *mutex) {

    /* Check for errors */
    assert(mutex);

    /* Set the lock owner to no one (i.e. NULL) */
    mutex->owner = NULL;

    /* Initialize the spinlock */
    lock_init(&mutex->lock);
}

/**
 * @brief Acquires the mutex lock
 *
 * Acquires the mutex lock and sets the owner of the lock to the calling thread.
 * The function does not return unless the lock is acquired
 *
 * @param[in] mutex Pointer to the mutex instance
 */
void hthread_mutex_lock(HThreadMutex *mutex) {

    /* Check for errors */
    assert(mutex);

    /* Acquire the lock */
    lock_acquire(&mutex->lock);

    /* Set the owner to the current thread */
    mutex->owner = (HThread)get_fs();
}

/**
 * @brief Releases the mutex lock
 *
 * Releases the mutex lock and sets the owner of the lock to no one.
 *
 * @param[in] mutex Pointer to the mutex instance
 */
void hthread_mutex_unlock(HThreadMutex *mutex) {

    /* Check for errors */
    assert(mutex);

    /* Set the owner to no one */
    mutex->owner = NULL;

    /* Release the lock  */
    lock_release(&mutex->lock);
}

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

        /* Depending on the type */
        switch (hthread->type) {

            case HTHREAD_TYPE_ONE_ONE:
                /* Free the one one thread completely */
                _one_one_free(hthread, 1);
                break;

            case HTHREAD_TYPE_MANY_MANY:
                /* Free the many many thread completely */
                _many_many_free(hthread, 1);
                break;

            default:
                break;
        }
    }
}

/**
 * Each thread has its own signal mask. (no provision provided yet in tcb)
 *
 * When the one-one thread is being loaded the getcontext is not getting
 * initialized before the signal handler is running. Which is causing the
 * program to segfault. (we can set the context of the function)
 *
 * Handle blocking and unblocking of the signal masks to handle this
 *
 * Do something for handling signal handling in many-many
 *
 * The cloned kernel thread inherits the sig mask of the parent thread
 *
 * ucontext_t has uc_sigmask - Use that for many-many case to provide local
 *                             sigmask to each user thread
 */
