#!/bin/sh

# Runs the engine in the specified configuration

if [[ $1 == "release" ]]; then
    bin/release/bloss1/bloss1
else
    bin/debug/bloss1/bloss1
fi

exit 0
