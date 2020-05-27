#!/bin/bash

REPO_DIR=$(cd "$(dirname "$0")/.."; pwd)
cd "$REPO_DIR"

make
cat samplecode | xargs -0 ./willani > tmp.s
gcc -g tmp.s -o tmp
gdb tmp