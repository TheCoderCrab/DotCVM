#!/bin/sh

# This is to build with release mode from main dir
# Expects to be executed from Main dir

export MAINDIR=$PWD

mkdir -p build/Release
cd build/Release

$MAINDIR/build/build.sh $MAINDIR 0

cd $MAINDIR
