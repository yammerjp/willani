#!/bin/bash

SCRIPT_DIR=$(cd "$(dirname "$0")"; pwd)
cd "$SCRIPT_DIR"

make
cat samplecode | xargs -0 ./willani > tmp.s
gcc tmp.s -o tmp.out > /dev/null 2&>1
# ./tmp.out
# rm tmp.s tmp.out
