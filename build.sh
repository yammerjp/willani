#!/bin/bash -e

SCRIPT_DIR=$(cd "$(dirname "$0")"; pwd)
cd "$SCRIPT_DIR"

output_filename="$1"

# ==========================================================================
# Compile souce files by willani
# ==========================================================================
mkdir -p asm

echo -e "\nCompile souce files by willani\n"

find src_self -type f | grep -E '*\.c' | while read -r C_SOURCE
do

ASM=$(echo "$C_SOURCE" | sed 's#^src_self/#asm/#g' | sed 's#c$#s#g')

echo "./willani $C_SOURCE -o $ASM"
./willani "$C_SOURCE" -o "$ASM"

done

# Assemble and Link
# ==========================================================================
echo -e "\nAssemble and link\n"
gcc -static -O0 -o "$output_filename" asm/*.s
