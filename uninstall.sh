#!/bin/sh


echo -e "\e[1m\e[32mUninstalling dotcvm\e[0m\e[39m"

if [ ${EUID:-$(id -u)} -eq 0 ]; then # Should be run as root
    [ -d /opt/dotcvm ] && rm -r /opt/dotcvm/
    [ -f /usr/share/applications/dotcvm.desktop ] && rm /usr/share/applications/dotcvm.desktop
else
    echo "Please run as root!"
    exit 0
fi
echo -e "\e[1m\e[32mUninstalled\e[0m\e[39m"