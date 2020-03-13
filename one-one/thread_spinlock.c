#include <syscall.h>
#include <unistd.h>
#include <linux/futex.h>
#include <stdatomic.h>
#include <errno.h>

#include "./thread_self.h"
#include "./thread_spinlock.h"

/**
 * Short macro for compare and swap primitive
 */
#define ATOMIC_XCHG(addr, old_val, new_val)                             \
    ({                                                                  \
        uint32_t __old_val = (old_val);                                 \
                                                                        \
        atomic_compare_exchange_strong((addr), &__old_val, (new_val));  \
    })

/**
 * Macros for lock status
 */
#define SPINLOCK_TAKEN     (0)
#define SPINLOCK_NOT_TAKEN (1)

/**
 * Number of processes to wake up after the lock is released
 */
#define NB_WAKEUP_PROCESSES (1)

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
 * @brief Initializes the spinlock
 * @param[out] spinlock Pointer to the spinlock instance
 * @note Reinitialization of a lock held by other threads may lead to
 *       unexpected results, hence this function must be called only once
 */
ThreadReturn thread_spinlock_init(ThreadSpinLock *spinlock) {

    /* Check for errors */
    if (!spinlock) {

        return THREAD_FAIL;
    }

    /* Set the lock word status to not taken */
    spinlock->lock_word = SPINLOCK_NOT_TAKEN;

    /* Set the owner to none */
    spinlock->owner_thread = NULL;

    return THREAD_OK;
}

/**
 * @brief Acquires the spinlock
 * @param[in/out] spinlock Pointer to the spinlock instance
 * @note The call is blocking and will return only if the lock is acquired
 */
ThreadReturn thread_spinlock(ThreadSpinLock *spinlock) {

    Thread thread;
    int futex_ret;

    /* Check for errors */
    if (!spinlock) {

        return THREAD_FAIL;
    }

    /* Get the thread handle */
    thread = thread_self();

    /* Check the thread already owns the lock */
    if (spinlock->owner_thread == thread) {

        /* Return as there is no need to lock */
        return THREAD_OK;
    }

    /* For eternity */
    while (1) {

        /* Check for the lock value atomically */
        if (ATOMIC_XCHG(&spinlock->lock_word,
                        SPINLOCK_NOT_TAKEN,
                        SPINLOCK_TAKEN)) {

            /* Set the owner of the lock */
            spinlock->owner_thread = thread;

            break;
        }

        /* Wait till the lock is not released by the owner */
        futex_ret = _futex(&spinlock->lock_word, FUTEX_WAIT, SPINLOCK_TAKEN);
        /* Check for errors */
        if ((futex_ret == -1) && (errno != EAGAIN)) {

            return THREAD_FAIL;
        }
    }

    return THREAD_OK;
}

/**
 * @brief Releases the spinlock
 * @param[in/out] spinlock Pointer to the spinlock instance
 */
ThreadReturn thread_spinunlock(ThreadSpinLock *spinlock) {

    int futex_ret;

    /* Check for errors */
    if (!spinlock) {

        return THREAD_FAIL;
    }

    /* Release the spinlock */
    if (ATOMIC_XCHG(&spinlock->lock_word,
                    SPINLOCK_TAKEN,
                    SPINLOCK_NOT_TAKEN)) {

        /* Set the owner thread to null */
        spinlock->owner_thread = NULL;

        /* Wake up the waiting threads */
        _futex(&spinlock->lock_word, FUTEX_WAKE, NB_WAKEUP_PROCESSES);
        /* Check for errors */
        if ((futex_ret == -1) && (errno != EAGAIN)) {

            return THREAD_FAIL;
        }
    }

    return THREAD_OK;
}
