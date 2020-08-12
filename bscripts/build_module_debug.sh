#!/bin/sh

# Expects to be executed from main directory

export MAIN_DIR=$PWD
mkdir -p build/debug/run/modules/$1/
mkdir -p build/modules/$1/debug

cmake -S modules/$1 -B build/modules/$1/debug -DDEBUG=1

cd build/modules/$1/debug
make
export EXIT_CODE=$?
cd $MAIN_DIR
cp modules/$1/module_folder/* build/debug/run/modules/$1/
cp build/modules/$1/debug/libdotcvm_$1.so build/debug/run/modules/$1/$1.linux.so

exit $EXIT_CODE
