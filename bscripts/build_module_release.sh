#!/bin/sh

# Expects to be executed from main directory

export MAIN_DIR=$PWD
mkdir -p build/release/run/modules/$1/
mkdir -p build/modules/$1/release

cmake -S modules/$1 -B build/modules/$1/release

cd build/modules/$1/release
make

export EXIT_CODE=$?

cd $MAIN_DIR
cp modules/$1/module_folder/* build/release/run/modules/$1/
cp build/modules/$1/release/libdotcvm_$1.so build/release/run/modules/$1/$1.linux.so

exit $EXIT_CODE
