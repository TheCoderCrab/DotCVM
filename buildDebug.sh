#!/bin/sh

# This is to build with debug mode from main dir
# Expects to be executed from Main dir

export MAINDIR=$PWD

mkdir -p build/Debug
cd build/Debug

$MAINDIR/build/build.sh $MAINDIR 1

cd $MAINDIR
