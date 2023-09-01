#!/bin/sh

# Generate makefiles and binaries in the specified configuration

if [[ -z $1 ]]; then
    vendor/premake/premake5 gmake && make -j4
else
    vendor/premake/premake5 gmake && make config=$1 -j4
fi

exit 0
