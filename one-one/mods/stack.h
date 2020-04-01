#ifndef _STACK_H_
#define _STACK_H_

int stack_alloc(void **stack_base, long *stack_limit);

int stack_free(void **stack_base, long *stack_limit);

#endif
