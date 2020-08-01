#!/bin/bash

# Set the required vars
BIN_DIR="../bin"
SRC_DIR="../src"
HOST_SYS_INC_DIR="/usr/include"
HOST_SYS_LIB_DIR="/usr/lib"

# Create the binary directoy
mkdir $BIN_DIR -p

# For each C file
for C_FILE in `ls -d $SRC_DIR/*.c $SRC_DIR/mods/*.c`
do
    # Get the object file
    gcc -fpic -Wall -c $C_FILE

    # If compilation resulted in error then exit
    if [ $? -ne 0 ]
    then
        # Print error
        echo "libthreads-C: Compilation error occurred"
        # Clean and exit
        source ./clean.sh
        exit
    fi
done

# Move the object files to the bin directory
mv ./*.o $BIN_DIR

# Create a static library from the object files
ar rcs $BIN_DIR/libthread.a $BIN_DIR/*.o

# If library creation failed then clean exit
if [ $? -ne 0 ]
then
    # Print error
    echo "libthreads-C: Static library creation failed"
    # Clean and exit
    source ./clean.sh
    exit
fi

# Create a shared library from the object files
gcc -shared -o $BIN_DIR/libthread.so $BIN_DIR/*.o

# If library creation failed then clean exit
if [ $? -ne 0 ]
then
    # Print error
    echo "libthreads-C: Shared object creation failed"
    # Clean and exit
    source ./clean.sh
    exit
fi

# Check if the include directory exists
if [ ! -e $HOST_SYS_INC_DIR ]
then
    # Print error
    echo "libthreads-C: Host system include directory not found"
    # Clean and exit
    source ./clean.sh
    exit
fi

# Copy the main header to the include directory
sudo cp $SRC_DIR/thread.h $HOST_SYS_INC_DIR

# Check if the library directory exists
if [ ! -e $HOST_SYS_LIB_DIR ]
then
    # Print error
    echo "libthreads-C: Host system library directory not found"
    # Clean and exit
    source ./clean.sh
    exit
fi

# Copy the library files to the lib directory
sudo cp $BIN_DIR/libthread.a $HOST_SYS_LIB_DIR
sudo cp $BIN_DIR/libthread.so $HOST_SYS_LIB_DIR
