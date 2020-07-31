#!/bin/sh

# Expects to be executed from main folder

echo -e "\e[1m\e[32mBuilding in debug mode\e[0m\e[39m"

cmake -S . -B build/debug -DDEBUG=1

cd build/debug
make