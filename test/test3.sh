#!/bin/bash -e

echo 'static int a; int main() { return a; }' > test-20.c
echo 'static int a;' > test-21.c
./willani test-20.c -o test-20.s
./willani test-21.c -o test-21.s
gcc -static test-20.s test-21.s -o test-20.out
./test-20.out
echo 'test3 ok'
