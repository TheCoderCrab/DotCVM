 
#!/bin/sh

echo -e "\e[1m\e[32mInstalling dotcvm standard modules\e[0m\e[39m"

mkdir -p $HOME/.local/share/dotcvm/modules
cp -r modules/* $HOME/.local/share/dotcvm/modules
echo -e "\e[1m\e[32mDotCVM standard modules installed\e[0m\e[39m"
