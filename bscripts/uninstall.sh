#!/bin/sh

echo -e "\e[1m\e[32mUninstalling dotcvm\e[0m\e[39m"

if [ ${EUID:-$(id -u)} -eq 0 ]; then # Should be run as root
    [ -d /opt/dotcvm ] && rm -r /opt/dotcvm/
    [ -f /usr/bin/dotcvm ] && rm -r /usr/bin/dotcvm
    [ -f /usr/share/applications/dotcvm.desktop ] && rm /usr/share/applications/dotcvm.desktop
    echo -e "\e[1m\e[32mUninstalled dotcvm from /opt/dotcvm/\e[0m\e[39m"
else
    echo "Please run as root!"
    exit 0
fi