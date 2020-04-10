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

static void _many_many_start(void) {

    Thread thread;

    /* Get the thread handle */
    thread = thread_self();

    /* Launch the thread start function */
    thread->ret = thread->start(thread->arg);

    /* Set the state as exited */
    thread->state = THREAD_STATE_EXITED;
}

void thread_create(Thread *thread, thread_start_t start, ptr_t arg) {

    /* Check for errors */
    assert(thread);
    assert(start);

    /* Allocate the thread descriptor */
    (*thread) = alloc_mem(struct Thread);

    /* Set the user thread id */
    (*thread)->utid = _get_nxt_utid();

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

    /* Free the stack */
    stack_free(&thread->curr_cxt->uc_stack);

    /* Free the contexts */
    free(thread->curr_cxt);
    free(thread->ret_cxt);

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

    /* Return to the scheduler */
    setcontext(thread->ret_cxt);
}

Thread thread_self(void) {

    /* Return the value of FS register */
    return (Thread)get_fs();
}

int main(int argc, char *argv[]) {

    Thread main_thread;
    int nb_kernel_threads;

    if (argc < 2) {

        nb_kernel_threads = 1;
    } else {

        nb_kernel_threads = atoi(argv[1]);
    }

    /* Initialize the global user thread id */
    _nxt_utid = 0;

    /* Initialize the global use thread id lock */
    lock_init(&_nxt_utid_lk);

    /* Initialize the many-many ready list */
    mmrll_init();

    /* Initialize the schedulers */
    mmsched_init(nb_kernel_threads);

    /* Create the main thread */
    thread_create(&main_thread, thread_main, NULL);

    /* Wait for its completion */
    thread_join(main_thread, NULL);

    /* Deinitialize the schedulers */
    mmsched_deinit();

    return 0;
}
