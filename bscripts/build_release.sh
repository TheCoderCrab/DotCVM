#!/bin/sh

# Expects to be executed from main folder

echo -e "\e[1m\e[32mBuilding in release mode\e[0m\e[39m"

cmake -S . -B build/release

cd build/release
make


export EXIT_CODE=$?

cp dotcvm run/dotcvm

exit $EXIT_CODE

