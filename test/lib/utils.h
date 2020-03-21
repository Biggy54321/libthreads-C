#define _GNU_SOURCE
#include <unistd.h>
#include <stdatomic.h>
#include <syscall.h>
#include <asm/prctl.h>
#include <sys/prctl.h>
#include <linux/futex.h>
#include <sys/time.h>

/* Get thread id function declaration to prevent warning */
pid_t gettid(void);

/* Kernel thread id */
#define KERNEL_THREAD_ID            (gettid())

/**
 * @brief Atomic compare and swap
 * @param[in] addr Address of lock variable
 * @param[in] old_val Old value expected in the lock variable
 * @param[in] new_value New value to be set in the lock variable
 * @return 1 if the value at addr is updated
 * @return 0 if the value at addr is not updated
 */
static inline int atomic_cas(int *addr, int old_val, int new_val) {

    /* Use the atomic function to compare and exchange */
    return atomic_compare_exchange_strong(addr, &old_val, new_val);
}

/**
 * @brief Futex syscall
 * @param[in] uaddr Pointer to the futex word
 * @param[in] futex_op Operation to be performed
 * @param[in] val Expected value of the futex word
 * @return 0 or errno
 */
static inline int futex(int *uaddr, int futex_op, int val) {

    /* Use the system call wrapper around the futex system call */
    return syscall(SYS_futex, uaddr, futex_op, val, NULL, NULL, 0);
}

/**
 * @brief Set FS register value
 * @param[in] addr Address to be set
 */
static inline void set_fs(long addr) {

    /* Use the syscall wrapper */
    syscall(SYS_arch_prctl, ARCH_SET_FS, addr);
}

/**
 * @brief Get FS register value
 * @return Value of the FS register (long)
 */
static inline long get_fs(void) {

    long addr;

    /* Use the syscall wrapper */
    syscall(SYS_arch_prctl, ARCH_GET_FS, &addr);

    return addr;
}
