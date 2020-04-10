#!/bin/zsh

cc ./mods/*.c ./*.c -lrt -ggdb -DNB_KERNEL_THREADS=1
