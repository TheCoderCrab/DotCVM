#!/bin/sh

# This is to build with debug mode from main dir
# Expects to be executed from Main dir

echo -e "\e[1m\e[32mBuilding in Debug mode\e[0m\e[39m"

export MAINDIR=$PWD

mkdir -p build/Debug
cd build/Debug

$MAINDIR/build/build.sh $MAINDIR 1

