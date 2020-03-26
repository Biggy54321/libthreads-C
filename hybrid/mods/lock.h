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

void lock_init(Lock *lock);

void lock_acquire(Lock *lock);

void lock_release(Lock *lock);

#endif
