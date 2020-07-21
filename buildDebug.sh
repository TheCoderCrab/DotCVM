#!/bin/sh

# This is to build with debug mode from main dir
# Expects to be executed from Main dir

export MAINDIR=$PWD

cd build

$MAINDIR/build.sh $MAINDIR 1

cd $MAINDIR
