#!/bin/sh

# Delete generated binaries of the specified configuration

if [[ $1 == "release" ]]; then
    make config=release clean
elif [[ $1 == "debug" ]]; then
    make config=debug clean
else
    make config=release clean
    make config=debug clean
fi

exit 0
