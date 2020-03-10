#ifndef _THREAD_JOIN_H_
#define _THREAD_JOIN_H_

#include "./thread_types.h"

/**
 * Thread join signature
 */
ThreadReturn thread_join(Thread thread, uint32_t *return_value);

#endif
