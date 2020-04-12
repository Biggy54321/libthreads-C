#!/bin/zsh

# Clean the binary directory
rm -rf ../bin/*

# Remove the header files from the include directory
sudo rm /usr/include/thread.h

# Remove the library files from the lib directory
sudo rm /usr/lib/libthread.a /usr/lib/libthread.so
