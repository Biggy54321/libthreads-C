#ifndef _THREAD_SPINLOCK_H_
#define _THREAD_SPINLOCK_H_

#include "./thread_types.h"

/**
 * Thread lock initialization function signature
 */
void thread_spinlock_init(ThreadSpinLock *spinlock);

/**
 * Thread lock function signature
 */
void thread_spinlock(ThreadSpinLock *spinlock);

/**
 * Thread unlock function signature
 */
void thread_spinunlock(ThreadSpinLock *spinlock);

#endif
