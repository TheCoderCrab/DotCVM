#!/bin/sh

# Expects to be executed from main folder

[ -d build/debug   ] || echo -e "\e[1m\e[32mNothing to cleaned for debug build\e[0m\e[39m"
[ -d build/debug   ] && echo -e "\e[1m\e[32mCleaning debug build\e[0m\e[39m" && rm -r build/debug