#!/bin/bash -e

SCRIPT_DIR=$(cd "$(dirname "$0")"; pwd)
cd "$SCRIPT_DIR"

# ==========================================================================
# Create 1st generation compiler
# ==========================================================================
echo -e "\nCreate 1st generation compiler\n"
make clean
make

# ==========================================================================
# Create 2nd generation compiler
# Patch and Compile souce files by willani
# ==========================================================================
echo -e "\nCreate 2nd generation compiler\n"
mkdir -p asm
mkdir -p src_self

echo -e "\nPatch and Compile souce files by willani\n"

# header
cat << EOS > src_self/willani.h
typedef _Bool bool;
typedef long FILE;
int fprintf();
int strlen();
int strncmp();
int isalpha();
int calloc();
int fopen();
int fclose();
int isspace();
int strstr();
long strtol();
#define true 1
#define false 0
#define NULL 0
int printf();
int fprintf();
extern int stdout;
extern int stderr;
int exit();
#define SEEK_END 2
#define SEEK_SET 0
int fseek();
typedef int size_t;
int ftell();
int fread();
EOS

cat src/willani.h \
  | grep -v -E '^#' \
  >> src_self/willani.h

cat src_self/willani.h > src_self/parse.h
cat src/parse/parse.h \
  | grep -v -E '^#' \
  >> src_self/parse.h

# source
find src -type f | grep -E '*\.c' | while read -r C_SOURCE
do

C_OVERWRITED=$(echo "$C_SOURCE" | sed 's#^src/parse/#src_self/#g' | sed 's#^src/#src_self/#g')
ASM=$(echo "$C_OVERWRITED" | sed 's#^src_self/#asm/#g' | sed 's#c$#s#g')

if [[ "$C_SOURCE" =~  ^src/parse/ ]]; then
  cat src_self/parse.h > "$C_OVERWRITED"
else
  cat src_self/willani.h > "$C_OVERWRITED"
fi

cat "$C_SOURCE" \
  | grep -v -E '^#' \
  >> "$C_OVERWRITED"

echo "./willani $C_OVERWRITED -o $ASM"
./willani "$C_OVERWRITED" -o "$ASM"

done

# Assemble and Link
# ==========================================================================
echo -e "\nAssemble and link\n"
gcc -static -g -O0 -o willani asm/*.s

# Run test
# ==========================================================================
echo -e "\nRun test\n"
make test
