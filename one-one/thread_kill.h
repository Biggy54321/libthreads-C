#ifndef _THREAD_KILL_H_
#define _THREAD_KILL_H_

#include "./thread_types.h"

/**
 * Thread kill function signature
 */
ThreadReturn thread_kill(Thread thread, int sig_num);

#endif
