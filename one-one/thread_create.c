#define _GNU_SOURCE

#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "./thread_create.h"

/**
 * @brief Returns the stack limit of a thread in bytes
 * @return Size in bytes
 */
static uint64_t _get_stack_limit(void) {

    struct rlimit stack_limit;

    /* Get the stack resource limit */
    getrlimit(RLIMIT_STACK, &stack_limit);

    /* Return the current limit */
    return stack_limit.rlim_cur;
}

/**
 * @brief Allocates the memory required for the thread stack
 * @param[in] stack_limit Size of the stack required
 * @return Pointer to the new stack base if successful
 * @return NULL if unsuccessful
 */
static ptr_t _allocate_stack(uint64_t stack_limit) {

    ptr_t stack_base;
    uint32_t page_size;

    /* Get the page size */
    page_size = getpagesize();

    /* Memory map the stack of the requested size plus page size */
    stack_base = mmap(NULL,
                      stack_limit + page_size,
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK,
                      -1, 0);
    /* Check for errors */
    if (stack_base == MAP_FAILED) {

        return NULL;
    }

    /* Allocate the stack guard */
    if (mprotect(stack_base, page_size, PROT_NONE) == -1) {

        /* Unmap the previously mapped region */
        munmap(stack_base, stack_limit + page_size);

        return NULL;
    }

    /* Return the base of the stack after the lowest page */
    return stack_base + page_size;
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
 * @brief The thread wrapper function which initiates the actual start
 *        function
 * @param[in] Address to the thread control block
 * @return Integer
 */
int _thread_wrapper(void *argument) {

    ThreadControlBlock *thread_block;

    /* Get the address of the thread control block */
    thread_block = (ThreadControlBlock *)argument;

    /* Set the exit jump location */
    if (!setjmp(thread_block->exit_env)) {

        /* Launch the thread */
        thread_block->return_value = thread_block->start_routine(
                thread_block->argument);
    }

    return 0;
}

/**
 * @brief Creates a thread of execution
 * @param[out] thread Pointer to the thread handle
 * @param[in] start_routine Start function of the thread
 * @param[in] argument Pointer to the argument
 * @return Thread return status enumeration
 */
ThreadReturn thread_create(
        Thread *thread,
        thread_start_t start_routine,
        ptr_t argument) {

    ThreadControlBlock *thread_block;
    ptr_t stack_top;

    /* Check if the arguments are valid */
    RETURN_FAIL_IF(!thread);
    RETURN_FAIL_IF(!start_routine);

    /* Allocate the thread control block */
    thread_block = (ThreadControlBlock *)malloc(THREAD_CONTROL_BLOCK_SIZE);
    /* Check for errors */
    RETURN_FAIL_IF(!thread_block);

    /* Set the thread handle */
    *thread = (Thread)thread_block;

    /* Initialize the start routine */
    thread_block->start_routine = start_routine;
    /* Initialize the start routine arguments */
    thread_block->argument = argument;

    /* Get the maximum stack limit */
    thread_block->stack_limit = _get_stack_limit();
    /* Allocate the stack */
    thread_block->stack_base = _allocate_stack(thread_block->stack_limit);
    /* Check for errors */
    if (!thread_block->stack_base) {

        /* Free the allocated memory */
        free(thread_block);

        return THREAD_FAIL;
    }

    /* Get the stack top */
    stack_top = thread_block->stack_base + thread_block->stack_limit;
    /* Clone the thread */
    thread_block->thread_id = clone(_thread_wrapper,
                                    stack_top,
                                    CLONE_VM | CLONE_FS | CLONE_FILES |
                                    CLONE_SIGHAND | CLONE_THREAD |
                                    CLONE_SYSVSEM | CLONE_PARENT_SETTID |
                                    CLONE_CHILD_CLEARTID | CLONE_SETTLS,
                                    thread_block,
                                    &thread_block->futex_word,
                                    thread_block,
                                    &thread_block->futex_word);
    /* Check for errors */
    if (thread_block->thread_id == -1) {

        /* Free the thread control block */
        free(thread_block);
        /* Deallocate the stack */
        _deallocate_stack(thread_block->stack_base, thread_block->stack_limit);

        return THREAD_FAIL;
    }

    /* Return success */
    return THREAD_OK;
}
