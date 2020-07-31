#!/bin/sh

# This builds in release mode

echo -e "\e[1m\e[32mInstalling dotcvm\e[0m\e[39m"

desktop_file="[Desktop Entry]
Name=DotCVM_DEBUG
Comment=A Virtual Machine Using DotArch architecture DEBUG_MODE
Exec=/opt/dotcvm_debug/dotcvm
Terminal=false
Type=Application
Categories=Development;"

if [ ${EUID:-$(id -u)} -eq 0 ]; then # Should be run as root
    bscripts/build_release.sh
    mkdir -p install/opt/dotcvm
    mkdir -p install/usr/share/applications
    echo "$desktop_file" > install/usr/share/applications/dotcvm_debug.desktop
    cp build/debug/dotcvm install/opt/dotcvm_debug/dotcvm
    cp -r install/* /
    [ -f /usr/bin/dotcvm_debug ] || ln -s /opt/dotcvm/dotcvm /usr/bin/dotcvm_debug
    rm -r install
    echo -e "\e[1m\e[32mInstalled to /opt/dotcvm_debug/\e[0m\e[39m"
else
    echo "Please run as root!"
    exit 0
fi