#ifndef _LOCK_H_
#define _LOCK_H_

#include "./types.h"

void lock_acquire(Lock *lock);

void lock_release(Lock *lock);

#endif
