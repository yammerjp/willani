#!/bin/bash

echo 'int ret3(){return 3;}int ret5(){return 5;} int add(int a,int b){return a+b;}' | gcc -xc -c  -o tmp2.o -
echo 'int sum(int a,int b,int c,int d,int e,int f){return a+b+c+d+e+f;}' | gcc -xc -c  -o tmp3.o -

assert() {
  expected="$1"
  input="$2"

  ./willani "$input" > tmp.s
  gcc -static -o tmp tmp.s tmp2.o tmp3.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 '0;'
assert 42 '42;'
assert 21 '5+20-4;'
assert 41 ' 12 + 34 - 5; '
assert 47 '5+6*7 ;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 10 '-10+20;'

assert 0 '0==1;'
assert 1 '42==42;'
assert 1 '0!=1;'
assert 0 '42!=42;'

assert 1 '0<1;'
assert 0 '1<1;'
assert 0 '2<1;'
assert 1 '0<=1;'
assert 1 '1<=1;'
assert 0 '2<=1;'

assert 1 '1>0;'
assert 0 '1>1;'
assert 0 '1>2;'
assert 1 '1>=0;'
assert 1 '1>=1;'
assert 0 '1>=2;'
assert 101 '2;3;0;101;'
assert 2 'i=2;'
assert 5 '1;p=100/(10+10);'
assert 7 '1;p=100/(10+10);i=p+2;i;'
assert 2 '1;p=100/(10+10);p-3;'
assert 2 '1;poyopoyo=100/(10+10);poyopoyo-3;'
assert 8 'foo123=3; bar=5; foo123+bar;'
assert 8 'foo_123=3; bar=5; foo_123+bar;'
assert 10 'return 10;'
assert 3 'returnp=3; return returnp; 150;'
assert 0 'a = 3 ; if(a<0) return 1; return 0;'
assert 3 'if (0) return 2; return 3;'
assert 3 'if (1-1) return 2; return 3;'
assert 2 'if (1) return 2; return 3;'
assert 2 'if (2-1) return 2; return 3;'
assert 3 'if (0) return 2; else return 3;'
assert 2 'if (2-1) return 2; else return 3;'
assert 10 'i=0;while(i<10)i=i+1; return i;'
assert 10 'i=0;j=0;while(i<10) { i=i+1; j = i;} return i;'
assert 7 '{1;p=100/(10+10);i=p+2;i;}'
assert 100 'j=0;for(i=0;i<50;i=i+1){j= j+2;}return j;'
assert 3 'return ret3();'
assert 5 'return ret5();'
assert 3 'return add(1,2);'
assert 21 'return sum(1,2,3,4,5,6);'

echo OK
