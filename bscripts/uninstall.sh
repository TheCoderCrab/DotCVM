#!/bin/sh

# Arg 0 should be the installation directory

if [ "$#" -ge 1 ]; then
    DC_INSTALL_DIR=$1
else
    DC_INSTALL_DIR="/opt/dotcvm/"
fi


echo -e "\e[1m\e[32mUninstalling dotcvm\e[0m\e[39m"

if [ ${EUID:-$(id -u)} -eq 0 ]; then # Should be run as root
    [ -f /usr/share/applications/dotcvm.desktop ] && rm /usr/share/applications/dotcvm.desktop
    [ -f /usr/bin/dotcvm ] && rm /usr/bin/dotcvm
    [ -d $HOME/.dotcvm ] && rm -r "$HOME/.dotcvm"
    echo -e "\e[1m\e[32mDone\e[0m\e[39m"
    [ -d "$DC_INSTALL_DIR" ] && rm -r "$DC_INSTALL_DIR" # This is here because the uninstall 
                                                        # script is probably inside the install
                                                        # directory so it should be removed last
else
    echo "Please run as root!"
    exit 0
fi 
