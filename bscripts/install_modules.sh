 
#!/bin/sh

echo -e "\e[1m\e[32mInstalling dotcvm standard modules\e[0m\e[39m"

mkdir -p $HOME/.dotcvm/modules
cp -r modules/* $HOME/.dotcvm/modules
echo -e "\e[1m\e[32mDotCVM standard modules installed\e[0m\e[39m"
