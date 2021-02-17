#!/bin/bash -e

SCRIPT_DIR=$(cd "$(dirname "$0")"; pwd)
cd "$SCRIPT_DIR"

function compile_by_gcc() {
  echo -e "\nCompile souce files by gcc\n"
  find src -type f | grep -E '*\.c' | while read -r C_FILE
  do
    ASM=$(echo "$C_FILE" | sed 's#c$#s#g')
    echo "gcc -S \"$C_FILE\" -o \"$ASM\""
    gcc -S "$C_FILE" -o "$ASM"
  done

  echo -e "\nAssemble and link\n"
  gcc -static -O0 -o willani src/*.s asm/*.s src/parse/*.s
}

function compile_by_self() {
  echo -e "\nCompile souce files by willani\n"
  find src -type f | grep -E '*\.c' | while read -r C_FILE
  do
    ASM=$(echo "$C_FILE" | sed 's#c$#s#g')
    echo "./willani \"$C_FILE\" -o \"$ASM\""
    ./willani "$C_FILE" -o "$ASM"
  done

  echo -e "\nAssemble and link\n"
  gcc -static -O0 -o willani src/*.s asm/*.s src/parse/*.s
}

function test_willani() {
	rm -f test-*
	echo 'int static_fn() { return 5; }' | gcc -xc -c -o test-01.o -
	./willani test/test.c -o test-00.s
	gcc -static test-00.s test-01.o -o test-00.out
	./test-00.out
	echo 'test1 is finished!!'
	./test/test2.sh
	echo 'test2 is finished!!'
	./test/test3.sh
	echo 'test3 is finished!!'
}

if [ "$1" = "gcc" ]; then
  compile_by_gcc
elif [ "$1" = "self" ]; then
  compile_by_self
elif [ "$1" = "test" ]; then
  test_willani
fi

