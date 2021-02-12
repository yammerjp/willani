#!/bin/bash -e

SCRIPT_DIR=$(cd "$(dirname "$0")"; pwd)
cd "$SCRIPT_DIR"

# ==========================================================================
# Compile souce files by willani
# ==========================================================================
echo -e "\nCompile souce files by willani\n"

find src -type f | grep -E '*\.c' | while read -r C_FILE
do
  ASM=$(echo "$C_FILE" | sed 's#c$#s#g')
  echo "./willani \"$C_FILE\" -o \"$ASM\""
  ./willani "$C_FILE" -o "$ASM"
done

# Assemble and Link
# ==========================================================================
echo -e "\nAssemble and link\n"
gcc -static -O0 -o willani src/*.s src/parse/*.s
