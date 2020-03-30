#ifndef _HTHREAD_SIG_H_
#define _HTHREAD_SIG_H_

#include <signal.h>

#include "./mods/list.h"
#include "./hthread_pub.h"

/**
 * Pending signal structure for many many threads
 */
typedef struct _Signal {

    /* Signal number */
    int sig;

    /* Linked list link */
    ListMember sig_mem;

} Signal;

void hthread_sigmask(int how, sigset_t *set, sigset_t *oldset);

void hthread_kill(HThread hthread, int sig);

#endif
