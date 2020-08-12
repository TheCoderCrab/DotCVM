#!/bin/sh

./bscripts/build_release.sh                                     && \
./bscripts/build_module_release.sh standard_cpu                 && \
./bscripts/build_module_release.sh standard_memory              && \
./bscripts/build_module_release.sh standard_io_bus              && \
./bscripts/build_module_release.sh standard_interrupt_bus       && \
./bscripts/build_module_release.sh standard_user_device         && \
./bscripts/build_module_release.sh standard_debug_console       && \
cd build/release/run                                            && \
../dotcvm

exit $?