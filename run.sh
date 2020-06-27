#!/bin/sh

./build.sh

cd build
java -Djava.library.path=. -cp . AppMain
