#!/bin/sh

# Generate makefiles and binaries in the specified configuration

if [[ -z $1 ]]; then
    vendor/premake/premake5 gmake && make
else
    vendor/premake/premake5 gmake && make config=$1
fi

exit 0
