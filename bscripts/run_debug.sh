#!/bin/sh

# Expects to be executed from main folder
echo -e "\e[1m\e[32mRunning in debug mode\e[0m\e[39m"
mkdir -p build/debug/run
bscripts/build_debug.sh && cd build/debug/run && ../dotcvm
