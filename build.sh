#!/bin/sh

# Expects to be executed from build dir
# $1 = main dir
# $2 = debug mode

javac $1/src/*.java -d . 

javah AppMain

mv AppMain.h $1/include/j_app_main.h
[ -f AppMain_Screen.h ] && rm AppMain_Screen.h

cmake -S $1 -B . -DDEBUG=$2

make


