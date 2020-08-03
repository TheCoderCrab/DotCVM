#!/bin/sh

# Expects to be executed from main directory

export MAIN_DIR=$PWD
mkdir -p build/debug/run/modules/std_mem/
mkdir -p build/modules/standard_memory/debug

cmake -S modules/standard_memory -B build/modules/standard_memory/debug -DDEBUG=1

cd build/modules/standard_memory/debug
make

cd $MAIN_DIR
cp modules/standard_memory/module_folder/* build/debug/run/modules/std_mem/
cp build/modules/standard_memory/debug/libdotcvm_standard_memory.so build/debug/run/modules/std_mem/std_mem.linux.so
