#!/bin/bash -e

SCRIPT_DIR=$(cd "$(dirname "$0")"; pwd)
cd "$SCRIPT_DIR"

echo "Create src/**/*.c => ./willani by gcc"
make clean
make

# compile
# ============================================================
echo -e "\nCompile src/**/*.c => asm/*.s"
mkdir -p asm
mkdir -p src_self

echo "
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
" > src_self/willani.h
cat src/willani.h \
  | grep -v -E '^#' \
  | sed -e 's/void error(char \*fmt, \.\.\.)/void error()/g' \
  >> src_self/willani.h

gcc src/read_file.c -S -o asm/read_file.s
gcc src/error.c -S -o asm/error.s
gcc src/preprocess.c -S -o asm/preprocess.s
gcc src/parse/enum.c -S -o asm/enum.s
gcc src/parse/expr.c -S -o asm/expr.s
gcc src/parse/function.c -S -o asm/function.s
gcc src/parse/log.c -S -o asm/log.s
gcc src/parse/new_node.c -S -o asm/new_node.s
gcc src/parse/parse.c -S -o asm/parse.s
gcc src/parse/read_type.c -S -o asm/read_type.s
gcc src/parse/scope.c -S -o asm/scope.s
gcc src/parse/stmt.c -S -o asm/stmt.s
gcc src/parse/strings.c -S -o asm/string.s
gcc src/parse/struct_tag.c -S -o asm/struct_tag.s
gcc src/parse/typedef.c -S -o asm/typedef.s
gcc src/parse/var.c -S -o asm/var.s
gcc src/parse/var_init.c -S -o asm/var_init.s

#gcc src/str_to_l.c -S -o asm/str_to_l.s
cat src_self/willani.h > src_self/str_to_l.c
cat src/str_to_l.c \
  | grep -v -E '^#' \
  >> src_self/str_to_l.c
./willani src_self/str_to_l.c > asm/str_to_l.s

#gcc src/main.c -S -o asm/main.s
cat src_self/willani.h > src_self/main.c
cat src/main.c \
  | grep -v -E '^#' \
  >> src_self/main.c
./willani src_self/main.c > asm/main.s

gcc src/tokenize.c -S -o asm/tokenize.s
<< COMMENTOUT
cat src_self/willani.h > src_self/tokenize.c
cat src/tokenize.c \
  | grep -v -E '^#' \
  | sed -e 's/Token head = {}/Token head/g' \
  >> src_self/tokenize.c
./willani src_self/tokenize.c > asm/tokenize.s
COMMENTOUT

#gcc src/tokenize_log.c -S -o asm/tokenize_log.s
cat src_self/willani.h > src_self/tokenize_log.c
cat src/tokenize_log.c \
  | grep -v -E '^#' \
  >> src_self/tokenize_log.c
./willani src_self/tokenize_log.c > asm/tokenize_log.s

gcc src/codegen.c -S -o asm/codegen.s
<< COMMENTOUT
cat src_self/willani.h > src_self/codegen.c
cat src/codegen.c \
  | grep -v -E '^#' \
  >> src_self/codegen.c
./willani src_self/codegen.c > asm/codegen.s
COMMENTOUT

gcc src/type.c -S -o asm/type.s
<< COMMENTOUT
cat src_self/willani.h > src_self/type.c
cat src/type.c \
  | grep -v -E '^#' \
  >> src_self/type.c
./willani src_self/type.c > asm/type.s
COMMENTOUT

# assemble & link
# ============================================================
rm ./willani
echo -e "\nLink obj/*.o => ./willani"
gcc asm/*.s -static -o willani

# test
# ============================================================
echo -e "\nRun tests"

rm -f test-*
echo 'int static_fn() { return 5; }' | gcc -xc -c -o test-01.o -
./willani test/test.c > test-00.s
gcc -static test-00.s test-01.o -o test-00.out

./test-00.out
echo 'test1 is finished!!'
./test/test2.sh
echo 'test2 is finished!!'
./test/test3.sh
echo 'test3 is finished!!'

