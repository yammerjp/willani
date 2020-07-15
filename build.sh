#!/bin/bash -e

SCRIPT_DIR=$(cd "$(dirname "$0")"; pwd)
cd "$SCRIPT_DIR"

# ==========================================================================
# Compile souce files by willani
# ==========================================================================
mkdir -p asm

echo -e "\nCompile souce files by willani\n"

find src_self -type f | grep -E '*\.c' | while read -r C_OVERWRITED
do

ASM=$(echo "$C_OVERWRITED" | sed 's#^src_self/#asm/#g' | sed 's#c$#s#g')

echo "./willani $C_OVERWRITED -o $ASM"
./willani "$C_OVERWRITED" -o "$ASM"

done

# Assemble and Link
# ==========================================================================
echo -e "\nAssemble and link\n"
gcc -static -O0 -o willani asm/*.s
