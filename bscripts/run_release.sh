#!/bin/sh

# Expects to be executed from main folder

bscripts/build_release.sh

mkdir -p build/release/run
cd build/release/run

echo -e "\e[1m\e[32mRunning in release mode\e[0m\e[39m"

../dotcvm
