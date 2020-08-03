#!/bin/sh

[ -d build/modules   ] || (echo -e "\e[1m\e[32mNothing to cleaned for modules\e[0m\e[39m" ; exit 0)
[ -d build/modules   ] && echo -e "\e[1m\e[32mCleaning modules\e[0m\e[39m" && rm -r build/modules
