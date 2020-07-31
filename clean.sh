#!/bin/sh

# Expects to be executed from main dir

[ -d build/Debug   ] && echo -e "\e[1m\e[32mCleaning debug build\e[0m\e[39m" && rm -r build/Debug
[ -d build/Release ] && echo -e "\e[1m\e[32mCleaning release build\e[0m\e[39m" && rm -r build/Release
