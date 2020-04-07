#ifndef _SIG_H_
#define _SIG_H_

int sig_block_all(void);

int sig_unblock_all(void);

int sig_send(int tid, int sig);

int sig_is_pending(void);

#endif
