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

    source $DEVKITPRO/switchvars.sh
    return;
}
export -f setup_devkitpro


setup_devkitpro