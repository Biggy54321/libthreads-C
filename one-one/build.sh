#!/bin/zsh

gcc test.c thread_create.c thread_join.c thread_self.c thread_exit.c \
    thread_spinlock.c thread_kill.c 
