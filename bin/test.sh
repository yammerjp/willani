#!/bin/bash

REPO_DIR=$(cd "$(dirname "$0")/.."; pwd)
cd "$REPO_DIR"

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

assert 0    'int main(){return 0;}'
assert 42   'int main(){return 42;}'
assert 21   'int main(){return 5+20-4;}'
assert 41   'int main(){return  12 + 34 - 5; }'
assert 47   'int main(){return 5+6*7 ;}'
assert 15   'int main(){return 5*(9-6);}'
assert 4    'int main(){return (3+5)/2;}'
assert 10   'int main(){return -10+20;}'

assert 0    'int main(){return 0==1;}'
assert 1    'int main(){return 42==42;}'
assert 1    'int main(){return 0!=1;}'
assert 0    'int main(){return 42!=42;}'

assert 1    'int main(){return 0<1;}'
assert 0    'int main(){return 1<1;}'
assert 0    'int main(){return 2<1;}'
assert 1    'int main(){return 0<=1;}'
assert 1    'int main(){return 1<=1;}'
assert 0    'int main(){return 2<=1;}'

assert 1    'int main(){return 1>0;}'
assert 0    'int main(){return 1>1;}'
assert 0    'int main(){return 1>2;}'
assert 1    'int main(){return 1>=0;}'
assert 1    'int main(){return 1>=1;}'
assert 0    'int main(){return 1>=2;}'
assert 101  'int main(){2;3;0;return 101;}'
assert 2    'int main(){int i; return i=2;}'
assert 5    'int main(){int p; 1;p=100/(10+10);return p;}'
assert 7    'int main(){int p;int i;1;p=100/(10+10);i=p+2;return i;}'
assert 2    'int main(){int p;1;p=100/(10+10);return p-3;}'
assert 2    'int main(){int poyopoyo;1;poyopoyo=100/(10+10);return poyopoyo-3;}'
assert 8    'int main(){int foo123; int bar;foo123=3; bar=5; return foo123+bar;}'
assert 8    'int main(){int foo_123; int bar;foo_123=3; bar=5; return foo_123+bar;}'
assert 10   'int main(){return 10;}'
assert 3    'int main(){int returnp; returnp=3; return returnp; 150;}'
assert 0    'int main(){int a;a = 3 ; if(a<0) return 1; return 0;}'
assert 3    'int main(){if (0) return 2; return 3;}'
assert 3    'int main(){if (1-1) return 2; return 3;}'
assert 2    'int main(){if (1) return 2; return 3;}'
assert 2    'int main(){if (2-1) return 2; return 3;}'
assert 3    'int main(){if (0) return 2; else return 3;}'
assert 2    'int main(){if (2-1) return 2; else return 3;}'
assert 10   'int main(){int i;i=0;while(i<10)i=i+1; return i;}'
assert 10   'int main(){int i;int j;i=0;j=0;while(i<10) { i=i+1; j = i;} return i;}'
assert 7    'int main(){{int p;int i;1;p=100/(10+10);i=p+2;i;}}'
assert 100  'int main(){int i; int j;j=0;for(i=0;i<50;i=i+1){j= j+2;}return j;}'
assert 3    'int main(){return ret3();}'
assert 5    'int main(){return ret5();}'
assert 3    'int main(){return add(1,2);}'
assert 21   'int main(){return sum(1,2,3,4,5,6);}'
assert 8    'int main(){return sub(10,2);}int sub(int i,int j){return i-j;}'
assert 18   'int main(){return calc(10,2);}int calc(int i,int j){int k; k =10;return i-j+k;}'
assert 4    'int main(){int i;i=0; while(i<4){ i = i +2; }return i;} '
assert 55   'int main(){return f(10);}int f(int n){if(n<=1)return n;return n+f(n-1);}'
assert 3    'int main() { int x; x=3; return *&x; }'
assert 3    'int main() { int x; int *y; int **z; x=3; y=&x; z=&y; return **z; }'
assert 5    'int main() { int x; int y; x=3; y=5; return *(&x-1); }'
assert 3    'int main() { int x; int y;  x=3; y=5; return *(&y+1); }'
assert 5    'int main() { int x; int *y; x=3; y=&x; *y=5; return x; }'
assert 7    'int main() { int x; int y; x=3; y=5; *(&x-1)=7; return y; }'
assert 7    'int main() { int x; int y; x=3; y=5; *(&y+1)=7; return x; }'

assert 8    'int main() { long x; return sizeof(x); }'
assert 8    'int main() { long x; return sizeof x; }'
assert 8    'int main() { long *x; return sizeof(x); }'
assert 8    'int main() { long x; return sizeof(x=2); }'
assert 8    'int main() { return sizeof(long); }'
assert 3    'int main() { long x; x=3; return *&x; }'

assert 4    'int main() { int x; return sizeof(x); }'
assert 4    'int main() { int x; return sizeof x; }'
assert 8    'int main() { int *x; return sizeof(x); }'
assert 4    'int main() { int x; return sizeof(x=2); }'
assert 4    'int main() { return sizeof(int); }'
assert 8    'int main() { return sizeof(int*); }'
assert 8    'int main() { return sizeof(int**); }'

assert 4    'int main() { int *x; *x = 4; return *x;}'
assert 12   'int main() { int x[3]; return sizeof(x); }'
assert 4    'int main() { int x[1]; *x = 4;return *x;}'
assert 4    'int main() { int x[2]; *x = 4;return *x;}'
assert 4    'int main() { int x[2]; *(x+1) = 4;return *(x+1);}'
assert 144  'int main() { int x[3]; *x = 4; *(x+1) = 9; *(x+2) = 16; return *(x+1) * *(x+2);}'
assert 0    'int main() { int x[2][3]; int *y; y=x; *y=0; return **x; }'
assert 1    'int main() { int x[2][3]; int *y; y=x; *(y+1)=1; return *(*x+1); }'
assert 2    'int main() { int x[2][3]; int *y; y=x; *(y+2)=2; return *(*x+2); }'
assert 3    'int main() { int x[2][3]; int *y; y=x; *(y+3)=3; return **(x+1); }'
assert 4    'int main() { int x[2][3]; int *y; y=x; *(y+4)=4; return *(*(x+1)+1); }'
assert 5    'int main() { int x[2][3]; int *y; y=x; *(y+5)=5; return *(*(x+1)+2); }'

assert 3    'int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return *x; }'
assert 4    'int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return *(x+1); }'
assert 5    'int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return *(x+2); }'
assert 5    'int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return *(x+2); }'
assert 5    'int main() { int x[3]; *x=3; x[1]=4; 2[x]=5; return *(x+2); }'

assert 0    'int main() { int x[2][3]; int *y;y=x; y[0]=0; return x[0][0]; }'
assert 1    'int main() { int x[2][3]; int *y;y=x; y[1]=1; return x[0][1]; }'
assert 2    'int main() { int x[2][3]; int *y;y=x; y[2]=2; return x[0][2]; }'
assert 3    'int main() { int x[2][3]; int *y;y=x; y[3]=3; return x[1][0]; }'
assert 4    'int main() { int x[2][3]; int *y;y=x; y[4]=4; return x[1][1]; }'
assert 5    'int main() { int x[2][3]; int *y;y=x; y[5]=5; return x[1][2]; }'
assert 5    'int gvar; int *gvarp; int g[2][3]; int main() { int x[2][3]; int *y;y=x; y[5]=5; return x[1][2]; } int gva;'

assert 0    'int x; int main() { return x; }'
assert 3    'int x; int main() { x=3; return x; }'
assert 7    'int x; int y; int main() { x=3; y=4; return x+y; }'
assert 7    'int x; int y; int main() { x=3; y=4; return x+y; }'
assert 0    'int x[4]; int main() { x[0]=0; x[1]=1; x[2]=2; x[3]=3; return x[0]; }'
assert 1    'int x[4]; int main() { x[0]=0; x[1]=1; x[2]=2; x[3]=3; return x[1]; }'
assert 2    'int x[4]; int main() { x[0]=0; x[1]=1; x[2]=2; x[3]=3; return x[2]; }'
assert 3    'int x[4]; int main() { x[0]=0; x[1]=1; x[2]=2; x[3]=3; return x[3]; }'

assert 4    'int x; int main() { return sizeof(x); }'
assert 16   'int x[4]; int main() { return sizeof(x); }'
assert 1    'int main() { char x;x=1; return x; }'
assert 1    'int main() { char x;x=1; char y;y=2; return x; }'
assert 2    'int main() { char x;x=1; char y;y=2; return y; }'

assert 1    'int main() { char x; return sizeof(x); }'
assert 10   'int main() { char x[10]; return sizeof(x); }'
assert 1    'int main() { return sub_char(7, 3, 3); } int sub_char(char a, char b, char c) { return a-b-c; }'

assert 8    'int main() { int k = 8; return k; }'
echo OK
