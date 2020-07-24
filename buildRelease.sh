#!/bin/sh

# This is to build with release mode from main dir
# Expects to be executed from Main dir

echo -e "\e[1m\e[32mBuilding in release mode\e[0m\e[39m"

export MAINDIR=$PWD

mkdir -p build/Release
cd build/Release

$MAINDIR/build/build.sh $MAINDIR 0

