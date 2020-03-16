#ifndef _SPIN_LOCK_H_
#define _SPIN_LOCK_H_

/**
 * Spinlock status macros
 */
#define SPIN_LOCK_NOT_TAKEN (1)
#define SPIN_LOCK_TAKEN     (0)

/**
 * Lock variable definition
 */
typedef int Lock;

void spin_lock(Lock *lock);

void spin_unlock(Lock *lock);

#endif
