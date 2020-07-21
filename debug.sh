#!/bin/sh

# This is to build and run from one command
# File name has to be changed

# This will build in debug mode

# Expects to be executed from main dir


export MAINDIR=$PWD

$MAINDIR/clean.sh


cd build
$MAINDIR/build.sh $MAINDIR 1 && clear && $MAINDIR/run.sh

cd $MAINDIR
