#ifndef _THREAD_SIG_H_
#define _THREAD_SIG_H_

#include <signal.h>

#include "./thread_types.h"

ThreadReturn thread_sigmask(int how, sigset_t *set, sigset_t *oldset);

ThreadReturn thread_kill(Thread thread, int sig_num);

#endif
