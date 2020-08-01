#!/bin/bash

# Set the required vars
BIN_DIR="../bin"
HOST_SYS_INC_DIR="/usr/include"
HOST_SYS_LIB_DIR="/usr/lib"

# Check if the binary directory exists
if [ -e $BIN_DIR ]
then
    # Delete the entire directory
    rm -rf $BIN_DIR
fi

# Check if the library header exists in the host system's include directory
if [ -e $HOST_SYS_INC_DIR/thread.h ]
then
    # Remove the header
    sudo rm $HOST_SYS_INC_DIR/thread.h
fi

# Check if the static library exists in the host system's library directory
if [ -e $HOST_SYS_LIB_DIR/libthread.a ]
then
    # Remove the library
    sudo rm $HOST_SYS_LIB_DIR/libthread.a
fi

# Check if the shared object exists in the host system's library directory
if [ -e $HOST_SYS_LIB_DIR/libthread.so ]
then
    # Remove the library
    sudo rm $HOST_SYS_LIB_DIR/libthread.so
fi
