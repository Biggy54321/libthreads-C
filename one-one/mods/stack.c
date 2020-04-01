#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "./stack.h"

/* Page size */
#define _PAGE_SIZE              (getpagesize())
/* Stack protection flags */
#define _STACK_PROT_FLAGS       (PROT_READ | PROT_WRITE)
/* Stack map flags */
#define _STACK_MAP_FLAGS        (MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK)
/* Stack guard flags */
#define _STACK_GUARD_PROT_FLAGS (PROT_NONE)

/**
 * @brief Get stack limit
 *
 * Returns the maximum stack size that can be allocated for a thread in bytes
 *
 * @return Size in bytes if success
 * @return -1 if error
 */
static long _stack_limit(void) {

    struct rlimit stack_limit;

    /* Get the stack resource limit */
    if (!getrlimit(RLIMIT_STACK, &stack_limit)) {

        /* Return the current limit */
        return stack_limit.rlim_cur;
    } else {

        /* Return error number */
        return -1;
    }
}

/**
 * @brief Allocates stack
 *
 * Memory maps a region in virtual address space to be used a stack. Prevents
 * uncontrolled growth of the stack by allocating stack guard equal to page size
 * at the end of the stack
 *
 * @param[out] stack_base Pointer to the stack base
 * @param[out] stack_limit Pointer to the stack limit
 * @return -1 if error
 * @return 0 if success
 */
int stack_alloc(void **stack_base, long *stack_limit) {

    /* Check for errors */
    if (!stack_base) {

        return -1;
    }
    if (!stack_limit) {

        return -1;
    }

    /* Get the stack limit */
    *stack_limit = _stack_limit();

    /* Check for errors */
    if (*stack_limit == -1) {

        return -1;
    }

    /* Memory map the stack */
    *stack_base = mmap(NULL,
                       *stack_limit + _PAGE_SIZE,
                       _STACK_PROT_FLAGS,
                       _STACK_MAP_FLAGS,
                       -1, 0);

    /* Check for errors */
    if (*stack_base == MAP_FAILED) {

        return -1;
    }

    /* Set the stack guard */
    if (mprotect(*stack_base, _PAGE_SIZE, _STACK_GUARD_PROT_FLAGS) == -1) {

        return -1;
    }

    /* Update the new base of the stack */
    *stack_base += _PAGE_SIZE;

    return 0;
}

/**
 * @brief Dellocates the stack
 *
 * Deallocates the stack previously allocated
 *
 * @param[in] stack_base Pointer to the stack base
 * @param[in] stack_limit Pointer to the stack limit
 * @return -1 if fail
 * @return 0 if success
 */
int stack_free(void **stack_base, long *stack_limit) {

    /* Check for errors */
    if (!stack_base) {

        return -1;
    }
    if (!stack_limit) {

        return -1;
    }

    /* Unmap the previously mapped stack region */
    if (munmap(*stack_base - _PAGE_SIZE, *stack_limit + _PAGE_SIZE) == -1) {

        return -1;
    }

    return 0;
}
