#!/bin/sh

# Expects to be executed from main folder

[ -d build/release   ] || (echo -e "\e[1m\e[32mNothing to cleaned for release build\e[0m\e[39m" ; exit 0)
[ -d build/release   ] && echo -e "\e[1m\e[32mCleaning release build\e[0m\e[39m" && rm -r build/release