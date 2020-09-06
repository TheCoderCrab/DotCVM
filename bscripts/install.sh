#!/bin/sh

# Arg 0 should be the installation directory
# Arg 1 is a boolean wether or not to create a menu entry

if [ "$#" -ge 1 ]; then
    DC_INSTALL_DIR=$1
else
    DC_INSTALL_DIR="/opt/dotcvm/"
fi

if [ "$#" -ge 2 ]; then
    DC_MENU_ENTR=$2
else
    DC_MENU_ENTR=0
fi


echo -e "\e[1m\e[32mInstalling dotcvm\e[0m\e[39m"

desktop_file="[Desktop Entry]
Name=DotC Virtual Machine
Comment=An Extensible Virtual Machine
Exec=$DC_INSTALL_DIR/dotcvm
Terminal=false
Type=Application
Categories=Development;"

if [ ${EUID:-$(id -u)} -eq 0 ]; then # Should be run as root
    mkdir -p $DC_INSTALL_DIR
    cp dotcvm "$DC_INSTALL_DIR/dotcvm"
    cp uninstall.sh "$DC_INSTALL_DIR/uninstall.sh"
    if [ $DC_MENU_ENTR -eq 1 ]; then
        echo -e "\e[1m\e[32mCreating menu entry\e[0m\e[39m"
        mkdir -p /usr/share/applications
        echo "$desktop_file" > /usr/share/applications/dotcvm.desktop
    fi
    [ -f /usr/bin/dotcvm ] && rm /usr/bin/dotcvm
    ln -s "$DC_INSTALL_DIR/dotcvm" /usr/bin/dotcvm
    echo -e "\e[1m\e[32mInstalled to $DC_INSTALL_DIR\e[0m\e[39m"

else
    echo "Please run as root!"
    exit 0
fi 
