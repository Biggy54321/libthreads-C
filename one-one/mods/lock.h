#ifndef _LOCK_H_
#define _LOCK_H_

/**
 * Lock handle
 */
typedef int Lock;

/**
 * Lock status
 */
#define LOCK_ACQUIRED     (0u)
#define LOCK_NOT_ACQUIRED (1u)

/**
 * Lock initializer
 */
#define LOCK_INITIALIZER  (LOCK_NOT_ACQUIRED)

int lock_acquire(Lock *lock);

int lock_release(Lock *lock);

#endif
