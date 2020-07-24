#!/bin/sh

# This builds in debug mode

desktop_file="[Desktop Entry]
Name=DotCVM_DEBUG
Comment=A Virtual Machine Using DotArch architecture DEBUG_MODE
Exec=/opt/dotcvm_debug/dotcvm
Terminal=false
Type=Application
Categories=Development;"

if [ ${EUID:-$(id -u)} -eq 0 ]; then # Should be run as root
    echo "I don't know why you are installing the debug version, but I don't care"
    ./buildDebug.sh
    mkdir -p install/opt/dotcvm_debug
    mkdir -p install/usr/share/applications
    echo "$desktop_file" > install/usr/share/applications/dotcvm_debug.desktop
    cp build/Debug/DotCVM install/opt/dotcvm_debug/dotcvm
    cp -r install/* /
    [ -f /usr/bin/dotcvm ] || ln -s /opt/dotcvm_debug/dotcvm /usr/bin/dotcvm_debug
    rm -r install
else
    echo "Please run as root!"
    exit 0
fi
