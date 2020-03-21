#ifndef _HTHREAD_KERNEL_H_
#define _HTHREAD_KERNEL_H_

#include "./hthread_defs.h"

void hthread_kernel_threads_init(int nb_kthds);

void hthread_kernel_threads_deinit(void);

#endif
