#!/bin/sh

javac src/*.java -d build

cd build
javah AppMain
cd ..

mv build/AppMain.h include/j_app_main.h
[ -f build/AppMain_Screen.h ] && rm build/AppMain_Screen.h

cmake -B build -DDEBUG=$1

cd build
make
cd ..


