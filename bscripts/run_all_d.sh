#!/bin/sh

export gdbinit_file="handle SIGINT pass nostop noprint
handle SIGQUIT pass nostop noprint
handle SIGSEGV pass nostop noprint
run
"

./bscripts/build_debug.sh                                   && \
./bscripts/build_module_debug.sh standard_cpu               && \
./bscripts/build_module_debug.sh standard_memory            && \
./bscripts/build_module_debug.sh standard_io_bus            && \
./bscripts/build_module_debug.sh standard_interrupt_bus     && \
./bscripts/build_module_debug.sh standard_user_device       && \
./bscripts/build_module_debug.sh standard_debug_console     && \
cd build/debug/run                                          && \

if [ "$1" -eq 1 ]; then
echo "$gdbinit_file" > gdbinit
( echo c ; cat ) | gdb -x gdbinit ../dotcvm
else
../dotcvm
fi

exit $?