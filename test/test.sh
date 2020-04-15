#!/bin/bash

# Gcc compilation flags
# GCC_COMPILATION_FLAGS="-DBLOCK_DEBUG_PRINTS"

# If the command line argument is only one
if [ $# -eq 1 ]
then
    # And the argument is help
    if [[ $1 == "help" ]]
    then
        echo "Usage: ./test.sh <lib_name> <mod_name> <cmd_args>"
        echo "lib_name: one-one/many-many/hybrid"
        echo "mod_name: create/exit/join/spinlock/mutex/signal/yield"
        echo "cmd_args: Integer argument to many-many and hybrid library"
    else
        echo "Run ./test.sh help for usage"
    fi
    exit
fi

# Check the number of command line arguments
if [ $# -lt 2 ]
then
    echo "Run ./test.sh help for usage"
    exit
fi

# Set the list of valid first command line arguments
VALID_FIRST_CMD_ARG=("one-one" "many-many" "hybrid")

# Install the requested library on the host system
if [[ " ${VALID_FIRST_CMD_ARG[*]} " == *"$1"* ]];
then
    cd ../$1/build/
    source ./install.sh
    cd ../../test/
else
    echo "First command line argument is invalid"
    exit
fi

# Set the test directory path according to the library type
if [[ $1 == "hybrid" ]]
then
    TEST_SRC_PATH="./tests_hybrid"
    # Set the list of valid second command line arguments
    VALID_SECOND_CMD_ARG=("create" "exit" "join" "spinlock" "signal" "yield" "equal" "once")

else
    TEST_SRC_PATH="./tests_one_many"
    # Set the list of valid second command line arguments
    VALID_SECOND_CMD_ARG=("create" "exit" "join" "spinlock" "mutex" "signal" "yield" "equal" "once")
fi

# Run the test code of the requested module
if [[ " ${VALID_SECOND_CMD_ARG[*]} " == *"$2"* ]];
then
    # Compile the test code for that module
    gcc $TEST_SRC_PATH/test_$2.c $TEST_SRC_PATH/print.c $GCC_COMPILATION_FLAGS -lthread
    # Run the executable
    ./a.out $3
else
    echo "Second command line argument is invalid"
    exit
fi

# Clean the executable
rm ./a.out
