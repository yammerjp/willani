#!/bin/bash

REPO_DIR=$(cd "$(dirname "$0")/.."; pwd)
cd "$REPO_DIR"

echo "Build willani by gcc"
make
echo "Build sample.c by willani"
cat sample.c | xargs -0 ./willani > tmp.s
gcc tmp.s -static -o tmp > /dev/null
echo "Run sample.c"
./tmp
echo -e "\nsample.c exitcode: $?\n"
# rm tmp.s tmp
