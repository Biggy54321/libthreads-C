#ifndef _THREAD_EXIT_H_
#define _THREAD_EXIT_H_

#include "./thread_types.h"
#include "./thread_self.h"

/**
 * Thread exit signature
 */
void thread_exit(ptr_t return_value);

#endif
