#!/bin/bash

assert() {
  expected="$1"
  input="$2"

  ./willani "$input" > tmp.s
  cc -o tmp tmp.s
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
assert 2 '1;p=100/(10+10);p-3;'
assert 2 '1;poyopoyo=100/(10+10);poyopoyo-3;'
assert 7 '1;p=100/(10+10);i=p+2;i;'
assert 8 'foo123=3; bar=5; foo123+bar;'

echo OK
