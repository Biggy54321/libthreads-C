#define _GNU_SOURCE
#include <sched.h>

#include "./mods/list.h"
#include "./mods/lock.h"
#include "./mods/stack.h"
#include "./mods/utils.h"
#include "./mmrll.h"
#include "./mmsched.h"
#include "./thread.h"
#include "./thread_descr.h"

/* Next user thread identifier */
static int _nxt_utid;
/* Next user thread identifier lock */
static Lock _nxt_utid_lk;

/* Main thread descriptor */
static Thread _main_thread;

/**
 * @brief Get next thread id
 *
 * Returns the thread id to be used for the next submitted user thread
 *
 * @return Integer id
 */
static int _get_nxt_utid(void) {

    int utid;

    /* Acquire the utid lock */
    lock_acquire(&_nxt_utid_lk);

    /* Get the id */
    utid = _nxt_utid++;

    /* Release the lock */
    lock_release(&_nxt_utid_lk);

    return utid;
}

static void _main_thread_init(void) {

    /* Allocate the thread descriptor */
    _main_thread = alloc_mem(struct Thread);

    /* Set the user thread id */
    _main_thread->utid = 0;

    /* Set the thread type */
    _main_thread->type = THREAD_TYPE_ONE_ONE;

    /* Set the thread state */
    _main_thread->state = THREAD_STATE_RUNNING;

    /* Set the wait word */
    _main_thread->wait = 1;

    /* Set the kernel thread id */
    _main_thread->ktid = KERNEL_THREAD_ID;

    /* Initialize the lock */
    lock_init(&_main_thread->mem_lock);

    /* Set the joining thread */
    _main_thread->join_thread = NULL;
}

void thread_init(int nb_kernel_threads) {

    /* Initialize the user thread id */
    _nxt_utid = 1;

    /* Initialize the user thread id lock */
    lock_init(&_nxt_utid_lk);

    /* Initialize the main thread */
    _main_thread_init();

    /* Initialize the many-many threads ready list */
    mmrll_init();

    /* Initialize the many-many scheduler */
    mmsched_init(nb_kernel_threads);
}

static int _one_one_start(void *arg) {

    Thread thread;

    /* Get the thread handle */
    thread = thread_self();

    /* Launch the thread start function */
    thread->ret = thread->start(thread->arg);

    /* Set the state as exited */
    thread->state = THREAD_STATE_EXITED;

    return 0;
}

static void _many_many_start(void) {

    Thread thread;

    /* Get the thread handle */
    thread = thread_self();

    /* Launch the thread start function */
    thread->ret = thread->start(thread->arg);

    /* Set the state as exited */
    thread->state = THREAD_STATE_EXITED;
}

void thread_create(Thread *thread, thread_start_t start, ptr_t arg,
                     ThreadType type) {

    /* Check for errors */
    assert(thread);
    assert(start);
    assert((type == THREAD_TYPE_ONE_ONE) || (type == THREAD_TYPE_MANY_MANY));

    /* Allocate the thread descriptor */
    (*thread) = alloc_mem(struct Thread);

    /* Set the user thread id */
    (*thread)->type = type;

    /* Set the state */
    (*thread)->state = THREAD_STATE_RUNNING;

    /* Set the start routine */
    (*thread)->start = start;

    /* Set the argument */
    (*thread)->arg = arg;

    /* Initialize the joining thread */
    (*thread)->join_thread = NULL;

    /* Initialize the member lock */
    lock_init(&(*thread)->mem_lock);

    /* Depending on the thread type */
    switch (type) {

        case THREAD_TYPE_ONE_ONE:

            /* Allocate the stack */
            stack_alloc(&(*thread)->stack);

            /* Create the kernel thread */
            (*thread)->ktid = clone(_one_one_start,
                                    (*thread)->stack.ss_sp +
                                    (*thread)->stack.ss_size,
                                    CLONE_VM | CLONE_FS | CLONE_FILES |
                                    CLONE_SIGHAND | CLONE_THREAD |
                                    CLONE_SYSVSEM | CLONE_SETTLS |
                                    CLONE_CHILD_CLEARTID |
                                    CLONE_PARENT_SETTID,
                                    NULL,
                                    &(*thread)->wait,
                                    (*thread),
                                    &(*thread)->wait);

            /* Check for errors */
            assert((*thread)->ktid != -1);

            break;

        case THREAD_TYPE_MANY_MANY:

            /* Initialize the wait word */
            (*thread)->wait = 1;

            /* Allocate the current and return contexts */
            (*thread)->curr_cxt = alloc_mem(ucontext_t);
            (*thread)->ret_cxt = alloc_mem(ucontext_t);

            /* Initialize the user thread context */
            getcontext((*thread)->curr_cxt);

            /* Allocate the stack */
            stack_alloc(&((*thread)->curr_cxt->uc_stack));

            /* Set the return context */
            (*thread)->curr_cxt->uc_link = (*thread)->ret_cxt;

            /* Make the context */
            makecontext((*thread)->curr_cxt, _many_many_start, 0);

            /* Initialize the pending signals */
            (*thread)->pend_sig = 0;

            /* Acquire the many ready list lock */
            mmrll_lock();
            /* Add the current thread to the list  */
            mmrll_enqueue((*thread));
            /* Release the many ready list lock */
            mmrll_unlock();

            break;

        default:
            break;
    }
}

void thread_join(Thread thread, ptr_t *ret) {

    Thread curr_thread;

    /* Check for errors */
    assert(thread);
    assert(thread->state != THREAD_STATE_JOINED);

    /* Get the current thread handle */
    curr_thread = thread_self();

    /* Check for deadlock with itself */
    assert(curr_thread != thread);

    /* Check for deadlock with target thread */
    assert(curr_thread->join_thread != thread);

    /* Acquire the member lock */
    lock_acquire(&thread->mem_lock);

    /* Check if the thread already has another joining thread */
    assert(!thread->join_thread);

    /* Set the joining thread */
    thread->join_thread = curr_thread;

    /* Release the member lock */
    lock_release(&thread->mem_lock);

    /* Update the current thread state */
    curr_thread->state = THREAD_STATE_WAIT_JOIN;

    /* Wait for the thread completion */
    while (!atomic_cas(&thread->wait, 0, 1));

    /* Update the current thread state */
    curr_thread->state = THREAD_STATE_RUNNING;

    /* If the return value is requested */
    if (ret) {

        /* Get the return value from the thread local storage */
        *ret = thread->ret;
    }

    /* Update the state of the target thread */
    thread->state = THREAD_STATE_JOINED;

    /* Depending on the thread type */
    switch (thread->type) {

        case THREAD_TYPE_ONE_ONE:

            /* Free the stack */
            stack_free(&thread->stack);

            break;

        case THREAD_TYPE_MANY_MANY:

            /* Free the stack */
            stack_free(&thread->curr_cxt->uc_stack);

            /* Free the contexts */
            free(thread->curr_cxt);
            free(thread->ret_cxt);

            break;

        default:
            break;
    }

    /* Free the thread descriptor */
    free(thread);
}

void thread_exit(ptr_t ret) {

    Thread thread;

    /* Get the thread handle */
    thread = thread_self();

    /* Check if thread is not dead or exited */
    assert(thread->state != THREAD_STATE_EXITED);
    assert(thread->state != THREAD_STATE_JOINED);

    /* Set the return value */
    thread->ret = ret;

    /* Set the thread state as exited */
    thread->state = THREAD_STATE_EXITED;

    /* Depending on the thread type */
    switch (thread->type) {

        case THREAD_TYPE_ONE_ONE:

            /* Exit using the syscall */
            sys_exit(0);
            break;

        case THREAD_TYPE_MANY_MANY:

            /* Return to the scheduler */
            setcontext(thread->ret_cxt);
            break;

        default:
            break;
    }
}

Thread thread_self(void) {

    /* If the calling thread is the main thread */
    if (KERNEL_THREAD_ID == _main_thread->ktid) {

        /* Return the main thread handle */
        return _main_thread;
    } else {

        /* Return the value of FS register */
        return (Thread)get_fs();
    }
}

void thread_deinit(void) {

    /* Deinitialize the many-many scheduler */
    mmsched_deinit();
}

