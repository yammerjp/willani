#!/bin/bash -e

echo 'extern int a; int main() { return a; }' > test-10.c
echo 'int a;' > test-11.c
./willani test-10.c -o test-10.s
./willani test-11.c -o test-11.s
gcc -static test-10.s test-11.s -o test-10.out
./test-10.out
echo 'test2 ok'
