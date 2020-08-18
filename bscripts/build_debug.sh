#!/bin/sh

# Expects to be executed from main folder

echo -e "\e[1m\e[32mBuilding in debug mode\e[0m\e[39m"

cmake -S . -B build/debug -DDEBUG=1 -DGDB="$1"


cd build/debug
make

export EXIT_CODE=$?

cp dotcvm run/dotcvm

exit $EXIT_CODE
