#ifndef _HTHREAD_H_
#define _HTHREAD_H_

#include "./hthread_defs.h"

void hthread_init(int nb_kthds);

HThread hthread_create(void *(*start)(void *), void *arg, HThreadType type);

void hthread_join(HThread hthread, void **ret);

void hthread_exit(void *ret);

HThread hthread_self(void);

void hthread_mutex_init(HThreadMutex *mutex);

void hthread_mutex_lock(HThreadMutex *mutex);

void hthread_mutex_unlock(HThreadMutex *mutex);

void hthread_kill(HThread hthread, int sig_num);

void hthread_deinit(void);

#endif
