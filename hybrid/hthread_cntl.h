#ifndef _HTHREAD_CNTL_H_
#define _HTHREAD_CNTL_H_

#include "./hthread_pub.h"

void hthread_init(int nb_kthds);

HThread hthread_create(void *(*start)(void *), void *arg, HThreadType type);

void hthread_join(HThread hthread, void **ret);

void hthread_exit(void *ret);

HThread hthread_self(void);

void hthread_deinit(void);

#endif
