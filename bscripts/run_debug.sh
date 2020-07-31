#!/bin/sh

# Expects to be executed from main folder

bscripts/build_debug.sh

mkdir -p build/debug/run
cd build/debug/run

echo -e "\e[1m\e[32mRunning in debug mode\e[0m\e[39m"

../dotcvm
