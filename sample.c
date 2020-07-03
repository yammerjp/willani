#define if(A,B,C) "hello, world %d %d %d\n", B, C, A
#define add(a,b,c) (a+b+(c))
#define hoge fuga

int printf(char *, int);
//#include <stdio.h>
int main() {
  printf(if(1,2,3));
  add((1+2),(3+(4)),5);
}
