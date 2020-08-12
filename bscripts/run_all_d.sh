#!/bin/sh

./bscripts/build_debug.sh                                   && \
./bscripts/build_module_debug.sh standard_cpu               && \
./bscripts/build_module_debug.sh standard_memory            && \
./bscripts/build_module_debug.sh standard_io_bus            && \
./bscripts/build_module_debug.sh standard_interrupt_bus     && \
./bscripts/build_module_debug.sh standard_user_device       && \
./bscripts/build_module_debug.sh standard_debug_console     && \
cd build/debug/run                                          && \
../dotcvm

exit $?