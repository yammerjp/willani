#!/bin/bash -e

SCRIPT_DIR=$(cd "$(dirname "$0")"; pwd)
cd "$SCRIPT_DIR"

# ==========================================================================
# Patch souce files to compile willani
# ==========================================================================
rm -rf src_self
mkdir -p src_self
echo  "Patch souce files to compile by willani"
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
void memcpy();
EOS

cat src/willani.h \
  | grep -v -E '^#' \
  >> src_self/willani.h

echo -e "#include \"willani.h\"\n" > src_self/parse.h
cat src/parse/parse.h \
  | grep -v -E '^#' \
  >> src_self/parse.h

# source
find src -type f | grep -E '*\.c' | while read -r C_SOURCE
do
  C_OVERWRITED=$(echo "$C_SOURCE" | sed 's#^src/parse/#src_self/#g' | sed 's#^src/#src_self/#g')
  cat "$C_SOURCE" > "$C_OVERWRITED"
done

echo "Directory 'src_self/' is created"
