#!/bin/sh

dirname=`dirname $0`

tmp="${dirname#?}"

if [ "${dirname%$tmp}" != "/" ]; then

dirname=$PWD/$dirname

fi

LD_LIBRARY_PATH=$dirname/lib

export LD_LIBRARY_PATH

echo $LD_LIBRARY_PATH

$dirname/lfs "$@"