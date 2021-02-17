#!/bin/bash -e

SCRIPT_DIR=$(cd "$(dirname "$0")"; pwd)
cd "$SCRIPT_DIR"

make clean
# ==========================================================================
# Create 1st generation compiler
# ==========================================================================
echo -e "Create 1st generation compiler"
./build.sh gcc
cp willani willani-1st

echo -e "Run test of 1st generation compiler"
./build.sh test

# ==========================================================================
# Create 2nd generation compiler
# Patch and Compile souce files by willani
# ==========================================================================
echo -e "Create 2nd generation compiler"

./build.sh self
cp willani willani-2nd
mkdir -p asm-2nd/parse
cp src/parse/*.s asm-2nd/parse
cp src/*.s asm-2nd

echo -e "Run test of 2nd generation compiler"
./build.sh test

# ==========================================================================
# Create 3rd generation compiler
# ==========================================================================
echo -e "Create 3rd generation compiler"
./build.sh self
cp willani willani-3rd
mkdir -p asm-3rd/parse
cp src/parse/*.s asm-3rd/parse
cp src/*.s asm-3rd

echo -e "Run test of 3rd generation compiler"
./build.sh test

# ==========================================================================
# Compare 2nd generation assembly codes with 3rd generation one
# ==========================================================================
echo -e "Compare 2nd generation assembly codes with 3rd generation one"
echo -e "diff asm-2nd asm-3rd"
diff asm-2nd asm-3rd
if [ "$?" == "0" ]; then
  echo "There is no difference!"
else
  echo "There is a difference..."
fi
