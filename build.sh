#!/bin/sh

# Expects to be executed from build dir
# $1 = main dir
# $2 = debug mode

cmake -S $1 -B . -DDEBUG=$2

make


