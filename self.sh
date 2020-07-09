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
# Create 1.5th generation compiler
# ==========================================================================
echo -e "\nCreate 1.5th generation compiler\n"
mkdir -p asm
mkdir -p src_self

# Compile souce files by gcc
# ==========================================================================

echo -e "\nCompile souce files by gcc\n"
find src -type f | grep -E '*\.c' | while read -r C_SOURCE
do
  ASM=$(echo "$C_SOURCE" | sed 's#^src/parse/#asm/#g' | sed 's#^src/#asm/#g' | sed 's#c$#s#g')
  gcc "$C_SOURCE" -S -o "$ASM"
done

# Patch and Compile souce files by willani
# ==========================================================================
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
EOS

cat src/willani.h \
  | grep -v -E '^#' \
  | sed -e 's/void error(char \*fmt, \.\.\.)/void error()/g' \
  >> src_self/willani.h

cat src_self/willani.h > src_self/parse.h
cat src/parse/parse.h \
  | grep -v -E '^#' \
  >> src_self/parse.h

# source
SELFHOST_FILES=$(cat << EOS
src/main.c
src/str_to_l.c
src/tokenize_log.c
src/type.c
src/tokenize.c
src/codegen.c
src/parse/enum.c
src/parse/function.c
src/parse/node_log.c
src/parse/parse.c
src/parse/parse_log.c
src/parse/scope.c
src/parse/strings.c
src/parse/struct_tag.c
src/parse/typedef.c
src/parse/var.c
src/parse/stmt.c
src/parse/expr.c
src/preprocess.c
EOS
)
# --- souce files compiled by gcc ---
# src/read_file.c
# src/error.c
# src/parse/read_type.c
# src/parse/new_node.c
# src/parse/var_init.c

echo "$SELFHOST_FILES" | while read -r C_SOURCE
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

echo "./willani $C_OVERWRITED > $ASM"
./willani "$C_OVERWRITED" > "$ASM"

done

# Assemble and Link
# ==========================================================================
echo -e "\nAssemble and link\n"
gcc -static -g -O0 -o willani asm/*.s

# Run test
# ==========================================================================
echo -e "\nRun test\n"
make test
