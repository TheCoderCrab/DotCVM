#!/bin/sh

# This builds in release mode

echo -e "\e[1m\e[32mInstalling DotCVM\e[0m\e[39m"

desktop_file="[Desktop Entry]
Name=DotC Virtual Machine
Comment=A Virtual Machine Using DotArch architecture
Exec=/opt/dotcvm/dotcvm
Terminal=false
Type=Application
Categories=Development;"

if [ ${EUID:-$(id -u)} -eq 0 ]; then # Should be run as root
    ./buildRelease.sh
    mkdir -p install/opt/dotcvm
    mkdir -p install/usr/share/applications
    echo "$desktop_file" > install/usr/share/applications/dotcvm.desktop
    cp build/Release/DotCVM install/opt/dotcvm/dotcvm
    cp -r install/* /
    [ -f /usr/bin/dotcvm ] || ln -s /opt/dotcvm/dotcvm /usr/bin/dotcvm
    rm -r install
else
    echo "Please run as root!"
    exit 0
fi

echo -e "\e[1m\e[32mInstalled to /opt/dotcvm/\e[0m\e[39m"