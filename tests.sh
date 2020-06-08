#!/bin/bash

./willani tests.c > test.s
gcc -g --static test.s
./a.out

