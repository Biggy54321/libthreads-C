#ifndef _SIGMASK_H_
#define _SIGMASK_H_

#include <signal.h>

void sigmask_block_all(void);

void sigmask_unblock_all(void);

void sigmask_block(sigset_t *mask);

void sigmask_unblock(sigset_t *mask);

void sigmask_get_mask(sigset_t *mask);

#endif
