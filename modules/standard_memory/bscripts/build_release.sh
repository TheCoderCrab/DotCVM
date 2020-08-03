#!/bin/sh

# Expects to be executed from main directory

export MAIN_DIR=$PWD
mkdir -p build/release/run/modules/std_mem/
mkdir -p build/modules/standard_memory/release

cmake -S modules/standard_memory -B build/modules/standard_memory/release

cd build/modules/standard_memory/release
make

cd $MAIN_DIR
cp modules/standard_memory/* build/release/run/modules/std_mem/
cp build/modules/standard_memory/release/libdotcvm_standard_memory.so build/release/run/modules/std_mem/std_mem.linux.so
