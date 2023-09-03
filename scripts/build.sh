#!/bin/sh

# Generate makefiles and binaries in the specified configuration
# https://stackoverflow.com/questions/394230/how-to-detect-the-os-from-a-bash-script

echo "OS type: $OSTYPE"

run_gmake () {
    if [[ -z $1 ]]; then
        vendor/premake/premake5 gmake2 && make -j4
    else
        vendor/premake/premake5 gmake2 && make config=$1 -j4
    fi
}

# Linux
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    run_gmake $1

# Windows
elif [[ "$OSTYPE" == "cygwin" ]]; then
    echo "@TODO: POSIX compatibility layer and Linux environment emulation for Windows"

elif [[ "$OSTYPE" == "msys" ]]; then
    run_gmake $1

# MAC
else
    echo "unsupported OS: $OSTYPE"
fi

exit 0
