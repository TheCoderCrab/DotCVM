#!/bin/sh

./build.sh 0 #Builds without debug

cd build
java -Djava.library.path=. -cp . AppMain
