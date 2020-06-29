#!/bin/bash -e

echo 'extern int a; int main() { return a; }' > test10.c
echo 'int a;' > test11.c
./willani test10.c > test10.s
./willani test11.c > test11.s
gcc -static test10.s test11.s -o test10.out
./test10.out
echo 'test2 ok'
