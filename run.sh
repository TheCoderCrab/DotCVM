#!/bin/sh

# Expects to be executed from build dir

java -Djava.library.path=. -cp . AppMain
