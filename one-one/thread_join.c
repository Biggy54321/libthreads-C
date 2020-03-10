#include <linux/futex.h>
#include <unistd.h>
#include <stdlib.h>
#include <syscall.h>
#include <sys/mman.h>
#include <errno.h>

#include "./thread_join.h"

/**
 * @brief Futex syscall
 * @param[in] uaddr Pointer to the futex word
 * @param[in] futex_op Operation to be performed
 * @param[in] val Expected value of the futex word
 * @return 0 or errno
 */
static inline int _futex(int *uaddr, int futex_op, int val) {

    /* Use the system call wrapper around the futex system call */
    return syscall(SYS_futex, uaddr, futex_op, val, NULL, NULL, 0);
}

/**
 * @brief Deallocates the stack previously allocated
 * @param[in] stack_base Base pointer to the stack
 * @param[in] stack_limit Size of the stack in bytes
 */
static void _deallocate_stack(ptr_t stack_base, uint64_t stack_limit) {

    uint32_t page_size;

    /* Get the page size */
    page_size = getpagesize();

    /* Unmap the allocated stack along with the protection page */
    munmap(stack_base - page_size, stack_limit + page_size);
}

/**
 * @brief Waits for the specified target thread to stop
 * @param[in] thread Thread handle
 * @param[out] return_value Pointer to the return value
 */
ThreadReturn thread_join(Thread thread, ptr_t *return_value) {

    ThreadControlBlock *thread_block;
    int ret_val;

    /* Get the thread control block */
    thread_block = (ThreadControlBlock *)thread;

    /* Wait on the thread's futex word */
    ret_val = _futex(&thread_block->futex_word,
                     FUTEX_WAIT,
                     thread_block->thread_id);
    /* Check for errors */
    if ((ret_val == -1) && (errno != EAGAIN)) {

        return THREAD_FAIL;
    }

    /* Deallocate the stack */
    _deallocate_stack(thread_block->stack_base, thread_block->stack_limit);

    /* If the user requested for return value of the thread */
    if (return_value) {

        /* Set the return value */
        *return_value = thread_block->return_value;
    }

    /* Free the thread control block */
    free(thread_block);

    return THREAD_OK;
}
