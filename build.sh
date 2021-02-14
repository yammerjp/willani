#!/bin/bash -e

#CC="./willani"
#CFLAGS="-S"
CC="willani"
if [ "$1" = "gcc" ]; then
  CC="gcc"
fi

SCRIPT_DIR=$(cd "$(dirname "$0")"; pwd)
cd "$SCRIPT_DIR"

# ==========================================================================
# Compile souce files by willani
# ==========================================================================
echo -e "\nCompile souce files by willani\n"

find src -type f | grep -E '*\.c' | while read -r C_FILE
do
  ASM=$(echo "$C_FILE" | sed 's#c$#s#g')
  if [ "$CC" = "willani" ]; then
    echo "./willani \"$C_FILE\" -o \"$ASM\""
    ./willani "$C_FILE" -o "$ASM"
  else
    echo "gcc -S \"$C_FILE\" -o \"$ASM\""
    gcc -S "$C_FILE" -o "$ASM"
  fi
done

# Assemble and Link
# ==========================================================================
echo -e "\nAssemble and link\n"
gcc -static -O0 -o willani src/*.s asm/*.s src/parse/*.s
