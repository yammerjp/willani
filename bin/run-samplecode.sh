#!/bin/bash

REPO_DIR=$(cd "$(dirname "$0")/.."; pwd)
cd "$REPO_DIR"

echo "Build willani by gcc"
make
echo "Build samplecode by willani"
cat samplecode | xargs -0 ./willani > tmp.s
gcc tmp.s -o tmp > /dev/null
echo "Run samplecode"
./tmp
echo -e "\nsamplecode exitcode: $?\n"
# rm tmp.s tmp
