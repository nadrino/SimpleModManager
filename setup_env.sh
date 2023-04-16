#! /bin/bash

function setup_devkitpro()
{
    echo "Seting up DevKitPro..." >&2
    export DEVKITPRO=/opt/devkitpro
    export DEVKITA64=${DEVKITPRO}/devkitA64
    export DEVKITARM=${DEVKITPRO}/devkitARM
    export DEVKITPPC=${DEVKITPRO}/devkitPPC
    export PORTLIBS_PREFIX=${DEVKITPRO}/portlibs/switch

    export PATH=${DEVKITPRO}/tools/bin:$PATH
    export PATH=${DEVKITA64}/bin/:$PATH

    return;
}; export -f setup_devkitpro

function send_file(){

  HOST='192.168.1.70:5000'
  lftp -e "cd $1; put $2; bye" $HOST

}; export -f send_file


setup_devkitpro
