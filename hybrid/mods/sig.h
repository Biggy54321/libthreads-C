#ifndef _SIG_H_
#define _SIG_H_

void sig_block_all(void);

void sig_unblock_all(void);

void sig_send(int tid, int sig);

int sig_is_pending(void);

#endif
