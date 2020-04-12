#!/bin/zsh

# Compile the position independent code for each source file
for C_FILE in `ls -d ../src/*.c ../src/mods/*.c`
do
    gcc -fpic -c $C_FILE
done

# Move the object files to the bin directory
mv ./*.o ../bin/

# Create a static library from the object files
ar rcs ../bin/libthread.a ../bin/*.o

# Create a shared library from the object files
gcc -shared -o ../bin/libthread.so ../bin/*.o

# Copy the main header to the include directory
sudo cp ../src/thread.h /usr/include/

# Copy the library files to the lib directory
sudo cp ../bin/libthread.a /usr/lib
sudo cp ../bin/libthread.so /usr/lib
