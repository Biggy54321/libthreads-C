#define _GNU_SOURCE
#include <sched.h>
#include <assert.h>
#include <stdlib.h>

#include "./mods/utils.h"
#include "./mods/lock.h"
#include "./mods/stack.h"
#include "./mods/sig.h"
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
    hthread = BASE(alloc_mem(struct _HThreadOneOne));

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
    ONE_ONE(hthread)->ret_cxt = alloc_mem(ucontext_t);

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
    /* Check for errors */
    assert(ONE_ONE(hthread)->tid != -1);

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
    hthread = BASE(alloc_mem(struct _HThreadManyMany));

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
    MANY_MANY(hthread)->curr_cxt = alloc_mem(ucontext_t);

    /* Allocate the return context */
    MANY_MANY(hthread)->ret_cxt = alloc_mem(ucontext_t);

    /* Set the current context */
    getcontext(MANY_MANY(hthread)->curr_cxt);

    /* Allocate the stack */
    stack_alloc(&(MANY_MANY(hthread)->curr_cxt->uc_stack));

    /* Set the backlink */
    MANY_MANY(hthread)->curr_cxt->uc_link = MANY_MANY(hthread)->ret_cxt;

    /* Make the context */
    makecontext(MANY_MANY(hthread)->curr_cxt, _many_many_start, 0);

    /* Initialize the pending signals list */
    list_init(&MANY_MANY(hthread)->pend_sig);

    /* Initialize the signal lock */
    lock_init(&MANY_MANY(hthread)->sig_lock);

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

    Signal *signal;

    /* Check for errors */
    assert(hthread);

    /* Free the stack */
    stack_free(&(MANY_MANY(hthread)->curr_cxt->uc_stack));

    /* Free the current context */
    free(MANY_MANY(hthread)->curr_cxt);

    /* Free the return context */
    free(MANY_MANY(hthread)->ret_cxt);

    /* Lock the signal list lock */
    lock_acquire(&MANY_MANY(hthread)->sig_lock);

    /* While the list is not empty */
    while (!list_is_empty(&MANY_MANY(hthread)->pend_sig)) {

        /* Get the signal */
        signal = list_dequeue(&MANY_MANY(hthread)->pend_sig, Signal, list_mem);

        /* Free the signal */
        free(signal);
    }

    /* Lock the signal list lock */
    lock_release(&MANY_MANY(hthread)->sig_lock);

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

            /* Lock the thread list */
            hthread_list_lock();

            /* Add the thread to the list */
            hthread_list_add(hthread);

            /* Unlock the thread list */
            hthread_list_unlock();
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
 * @brief Update the signal mask
 *
 * Changes the current signal mask of thread
 *
 * @param[in] how What action to perform
 * @param[in] set Pointer to the signal set to be worked upon
 * @param[out] oldset Pointer to the signal set which will store the old signal
 *             mask. Will store the previously block signal set
 */
void hthread_sigmask(int how, sigset_t *set, sigset_t *oldset) {

    HThread hthread;

    /* Check for errors */
    assert(set);
    assert(oldset);

    /* Remove SIGALRM from the signal set (just becuz i use it) */
    sigdelset(set, SIGALRM);

    /* Call the signal process mask function */
    sigprocmask(how, set, oldset);
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

    Signal *signal;

    /* Check for errors */
    assert(hthread);

    /* Wait till the target thread is not properly initialized */
    while (hthread->state == HTHREAD_STATE_INIT);

    /* Depending on the target thread type */
    switch (hthread->type) {

        case HTHREAD_TYPE_ONE_ONE:

            /* Send the signal to the thread */
            sig_send(ONE_ONE(hthread)->tid, sig_num);
            break;

        case HTHREAD_TYPE_MANY_MANY:

            /* Create a new signal */
            signal = alloc_mem(Signal);

            /* Initialize the signal */
            signal->sig = sig_num;

            /* Lock the signal list */
            lock_acquire(&MANY_MANY(hthread)->sig_lock);

            /* Add the signal to the thread's list of deliverables */
            list_enqueue(&MANY_MANY(hthread)->pend_sig, signal, list_mem);

            /* Unlock the signal list */
            lock_release(&MANY_MANY(hthread)->sig_lock);
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

    /* Lock the list */
    hthread_list_lock();

    /* Deinitialize all the many many threads */
    while (!hthread_list_is_empty()) {

        /* Get the thread from the list */
        hthread = hthread_list_get();

        /* Free the many many thread completely */
        _many_many_free(hthread, 1);
    }

    /* Lock the list */
    hthread_list_unlock();
}

/**
 * The one-one thread control blocks are not being freed at the hthread_deinit()
 * call
 */

/**
 * Sigmask function will be same as sigprocmask for both types of the threads
 * Switching of the sigmask in case of all the user threads is handled
 * respectively
 *
 * In kill -
 * 1. One-one - Directly signal can be sent to the user thread as it is
 *              mapped to the same kernel thread all the time
 * 2. Many-many - Signal cannot be sent directly, as the user thread floats on
 *                the kernel threads. Hence before and after interrupt the
 *                same user thread may be mapped on different kernel thread
 *
 * For many-many -
 * 1. hthread_kill() will add to a list of signals which should be delivered
 *    in the next scheduling turn of the thread.
 * 2. These signals will be set by the kernel thread scheduler on itself just
 *    before scheduling the user thread on it using tgkill().
 * 3. However the handlers corresponding to the set signals will not be called
 *    in the scheduler context, as scheduler blocks all the signals. Hence all
 *    the signals will be pending in the scheduler context.
 * 4. When the user context is set on the kernel thread, its signal mask will
 *    come into play and the signals which are not blocked will result in
 *    the invokation of their handlers.
 * 5. However problem arises when the user thread is handling a signal and
 *    timer interrupt occurs and the user thread switches to scheduler. The
 *    problem arises because the same user thread may now be scheduled on
 *    some other kernel thread later on. But we have already set the signals
 *    which were to be delivered to the user thread on the previous kernel
 *    thread.
 * 6. Hence we can make the user thread run on the same kernel thread till
 *    any signals are pending to be delivered to the user thread.
 * 7. Even if we do the above mentioned things, one corner case remains which
 *    is if we have many signals to be delivered and we set them. Now one of
 *    them called exit, but other signal handlers have not executed yet. So
 *    in this case should we let all the signals to run and then exit or just
 *    exit. If we just exit then the pending signals will be invoked in the user
 *    thread which was not killed by those signals. However as the linux kernel
 *    does not provide any such provision for directing the signals to the user
 *    space threads, this problem will pertain.
 */

/**
 * Do not schedule the user threads which are waiting for a thread which is
 * already joined.
 * Do not schedule the user threads which are waiting for an active thread to
 * join.
 * Make use of queue for active join?
 *
 * @brief
 * 1. The target thread is surely over when its wait variable becomes zero from
 *    nonzero
 * 2. When a thread tries to join with the target thread, then there are two
 *    cases which will be checked atomically:
 *    a. Target thread is not finished: In this case the calling thread
 *       will change its state to wait (This will cause one-one thread to
 *       wait using a futex and many-many thread to be removed from the ready
 *       queue). Hence a thread in wait state will not be scheduled either by
 *       the library or the kernel. When the target thread gets finished it
 *       will check the wait list if it is nonempty then it will make the first
 *       task runnable again.
 *    b. Target thread is finished: In this case the calling thread will
 *       not wait.
 */

/**
 * Should i really improve the mutex locks to use queues?
 */
