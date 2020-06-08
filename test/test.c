// This is a line comment.

/*
 * This is a block comment.
 */

int printf();
int exit();

int assert(char *str, long value, long expected) {
  printf("%s => %d", str, expected);
  if (value != expected) {
    printf(" expected, but got %d\n", value);
    exit(1);
  }
  printf("\n");
  return 0;
}

int ret3(){return 3;}
int ret5(){return 5;}
int add(int a,int b){return a+b;}
int sub(int a,int b){return a-b;}
int sum(int a,int b,int c,int d,int e,int f){return a+b+c+d+e+f;}
int calc(int i, int);
int f(int n){if(n<=1)return n;return n+f(n-1);}

int ret101() { 2;3;0;return 101; }
int if_func() { int a;a = 3 ; if(a<0) return 1; return 0; }

int x;
int y[4];
int p; int q;

int gvar; int *gvarp; int g[2][3]; 

int sub_char(char a, char b, char c) { return a-b-c; }

int summing();

int ret10(){ int i = 0; for (;;) { if (i<10) i = i + 1; else return i; } }

int main() {
  printf("\nRun test\n\n");

  assert("0", 0, 0);
  assert("42", 42, 42);
  assert("12 + 34 = 5", 12 + 34 - 5 , 41);

  assert("5+20-4"     , 5+20-4    , 21);
  assert("5+6*7"      , 5+6*7     , 47);
  assert("5*(9-6)"    , 5*(9-6)   , 15);
  assert("(3+5)/2"    , (3+5)/2   , 4);
  assert("-10+20"     , -10+20    , 10);
  assert("0==1"       , 0==1      , 0);
  assert("42==42"     , 42==42    , 1);
  assert("0!=1"       , 0!=1      , 1);
  assert("42!=42"     , 42!=42    , 0);

  assert("0<1"        , 0<1       , 1);
  assert("1<1"        , 1<1       , 0);
  assert("2<1"        , 2<1       , 0);
  assert("0<=1"       , 0<=1      , 1);
  assert("2<=1"       , 2<=1      , 0);
  assert("1>0"        , 1>0       , 1);
  assert("1>1"        , 1>1       , 0);
  assert("1>2"        , 1>2       , 0);
  assert("1>=1"       , 1>=1      , 1);
  assert("1>=2"       , 1>=2      , 0);

  assert("({ 2;3;0;return 101; })", ret101(), 101);
  assert("({ i=2 })", ({int i; i=2;}), 2);
  assert("({ int p;p=100/(10+10); })", ({int p; p=100/(10 +10);}), 5);
  assert("({ int p;1;p=100/(10+10);p-3; })", ({int p;1;p=100/(10+10);p-3;}) ,2);
  assert("({ int poyopoyo;1;poyopoyo=100/(10+10); poyopoyo-3; })", ({int poyopoyo;1;poyopoyo=100/(10+10); poyopoyo-3;}), 2);
  assert("({ int foo123; int bar;foo123=3; bar=5; foo123+bar; })", ({int foo123; int bar;foo123=3; bar=5; foo123+bar;}), 8);
  assert("({ int foo_123; int bar;foo_123=3; bar=5; foo_123+bar; })", ({int foo_123; int bar;foo_123=3; bar=5; foo_123+bar;}), 8);

  assert("({ int returnp; returnp=3; })", ({int returnp; returnp=3;}), 3);
  assert("({ int retur; retur=3; })", ({int retur; retur=3;}), 3);
  assert("int a;a = 3 ; if(a<0) return 1; return 0;", if_func(), 0);
  assert("({ int a;if (0) a=2; else a=3; a; })", ({int a;if (0) a=2; else a=3;a;}), 3); 
  assert("({ int a;if (10) a=2; else a=3; a;})", ({int a;if (10) a=2; else a=3;a;}), 2); 

  assert("({ int i;i=0;while(i<10)i=i+1; i; })", ({int i;i=0;while(i<10)i=i+1; i;}), 10);

  assert("({ int p;int i;1;p=100/(10+10);i=p+2;i; })", ({ int p;int i;1;p=100/(10+10);i=p+2;i; }) , 7 );
  assert("({ int i; int j=0;for(i=0;i<50;i=i+1){j= j+2;} j; })", ({int i; int j=0;for(i=0;i<50;i=i+1){j= j+2;} j;}), 100);
  assert("({ ret3(); })", ({ ret3(); }), 3 );
  assert("({ ret5(); })", ({ ret5(); }), 5 );
  assert("int add(int a,int b); int main(){return add(1,2);}", add(1,2), 3);
  assert("int sum(int, int, int, int, int, int); int main(){return sum(1,2,3,4,5,6);}", sum(1,2,3,4,5,6), 21);
  assert("int sub(int i,int j){return i-j;} int main(){return sub(10,2);}", sub(10,2), 8);
  assert("int calc(int i, int); int main(){return calc(10,2);}int calc(int i,int j){int k; k =10;return i-j+k;}'", calc(10,2), 18);
  assert("({ int i;i=0; while(i<4){ i = i +2; } i;})", ({ int i;i=0; while(i<4){ i = i +2; } i;}), 4);

  assert("int f(int n){if(n<=1)return n;return n+f(n-1);} int main(){return f(10);}", f(10), 55);
  assert("({ int x; x=3; *&x; })",({ int x; x=3; *&x; }), 3);
  assert("({ int x; int *y; int **z; x=3; y=&x; z=&y; **z; })", ({ int x; int *y; int **z ; x=3; y=&x; z=&y; **z; }), 3);
  assert("({ int x; int *y; x=3; y=&x; *y=5; x; })", ({ int x; int *y; x=3; y=&x; *y=5; x; }), 5);


  // It depends a compiler that the order of local variables' area in the stack 
  assert("({ int x; int y; x=3; y=5; *(&x-1); })",({ int x; int y; x=3; y=5; *(&x-1); }) ,5);
  assert("({ int x; int y; x=3; y=5; *(&y+1); })",({ int x; int y; x=3; y=5; *(&y+1); }) ,3);
  assert("({ int x; int y; x=3; y=5; *(&x-1)=7; y; })",({ int x; int y; x=3; y=5; *(&x-1)=7; y; }) , 7);
  assert("({ int x; int y; x=3; y=5; *(&y+1)=7; x; })", ({ int x; int y; x=3; y=5; *(&y+1)=7; x; }), 7);


  assert("({ long x; sizeof(x); })", ({ long x; sizeof(x); }), 8);
  assert("({ long *x; sizeof(x); })", ({ long *x; sizeof(x); }), 8);
  assert("({ long x; sizeof(x=2); })", ({ long x; sizeof(x=2); }), 8);
  assert("sizeof(long)", sizeof(long), 8);
  assert("({ long x=3; *&x; })", ({ long x=3; *&x; }), 3);
  assert("({ int x; sizeof(x); })", ({ int x; sizeof(x); }), 4);
  assert("({ int x; sizeof x ; })", ({ int x; sizeof x ; }), 4);
  assert("({ int *x; sizeof(x); })", ({ int *x; sizeof(x); }), 8);
  assert("({ int x; sizeof(x=2); })", ({ int x; sizeof(x=2); }), 4);
  assert("sizeof(int)", sizeof(int), 4);
  assert("sizeof(int*)", sizeof(int*), 8);
  assert("sizeof(int **)", sizeof(int**), 8);

  assert("({ int *x; int y; x = &y; *x = 3 ; *x;})", ({ int *x; int y; x = &y; *x = 3 ; *x;}), 3);
  assert("({ int x[3]; sizeof(x); })", ({ int x[3]; sizeof(x); }), 12);
  assert("({ int x[1]; *x = 4; *x; })", ({ int x[1]; *x = 4; *x; }), 4);
  assert("({ int x[2]; *x = 4; *x; })", ({ int x[2]; *x = 4; *x; }), 4);
  assert("({ int x[2]; *(x+1) = 4; *(x+1); })", ({ int x[2]; *(x+1) = 4; *(x+1); }), 4);
  assert("({ int x[3]; *x = 4; *(x+1) = 9; *(x+2) = 16; *(x+1) * *(x+2); })",
    ({ int x[3]; *x = 4; *(x+1) = 9; *(x+2) = 16; *(x+1) * *(x+2); }), 144);
  assert("({ int x[2][3]; int *y; y=x; *y=0; **x; })", ({ int x[2][3]; int *y; y=x; *y=0; **x; }),   0);
  assert("({ int x[2][3]; int *y; y=x; *(y+1)=1; *(*x+1); })",
    ({ int x[2][3]; int *y; y=x; *(y+1)=1; *(*x+1); }), 1);
  assert("({ int x[2][3]; int *y; y=x; *(y+3)=3; **(x+1); })",
    ({ int x[2][3]; int *y; y=x; *(y+3)=3; **(x+1); }), 3);
  assert("({ int x[2][3]; int *y; y=x; *(y+2)=2; *(*x+2); })",
    ({ int x[2][3]; int *y; y=x; *(y+2)=2; *(*x+2); }), 2);
  assert("({ int x[2][3]; int *y; y=x; *(y+4)=4; *(*(x+1)+1); })",
    ({ int x[2][3]; int *y; y=x; *(y+4)=4; *(*(x+1)+1); }),4);
  assert("({ int x[2][3]; int *y; y=x; *(y+5)=5; *(*(x+1)+2); })",

    ({ int x[2][3]; int *y; y=x; *(y+5)=5; *(*(x+1)+2); }), 5);
  assert("({ int x[3]; *x=3; x[1]=4; x[2]=5; *x; })", ({ int x[3]; *x=3; x[1]=4; x[2]=5; *x; }), 3);
  assert("({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+1); })",
    ({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+1); }), 4); 
  assert("({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2); })",
    ({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2); }), 5);
  assert("({ int x[3]; *x=3; x[1]=4; 2[x]=5; *(x+2); })",
    ({ int x[3]; *x=3; x[1]=4; 2[x]=5; *(x+2); }), 5);
  assert("({ int x[2][3]; int *y;y=x; y[0]=0; x[0][0]; })",
    ({ int x[2][3]; int *y;y=x; y[0]=0; x[0][0]; }), 0); 
  assert("({ int x[2][3]; int *y;y=x; y[1]=1; x[0][1]; })",
    ({ int x[2][3]; int *y;y=x; y[1]=1; x[0][1]; }), 1);
  assert("({ int x[2][3]; int *y;y=x; y[2]=2; x[0][2]; })",
    ({ int x[2][3]; int *y;y=x; y[2]=2; x[0][2]; }),2); 
  assert("({ int x[2][3]; int *y;y=x; y[3]=3; x[1][0]; })",
    ({ int x[2][3]; int *y;y=x; y[3]=3; x[1][0]; }), 3);
  assert("({ int x[2][3]; int *y;y=x; y[4]=4; x[1][1]; })",
    ({ int x[2][3]; int *y;y=x; y[4]=4; x[1][1]; }), 4);
  assert("({ int x[2][3]; int *y;y=x; y[5]=5; x[1][2]; })",
    ({ int x[2][3]; int *y;y=x; y[5]=5; x[1][2]; }), 5);

  assert("x // global variable", x, 0);
  assert("int gvar; int *gvarp; int g[2][3]; int main() { int x[2][3]; int *y;y=x; y[5]=5; return x[1][2]; } int gva;",
    ({ int x[2][3]; int *y;y=x; y[5]=5; x[1][2]; }), 5);
  assert("int x; int main() { x=3; return x; }",({ x=3; x; }), 3);
  assert("int p; int q; int main() { p=3; q=4; return p+q; }", ({ p=3; q=4; p+q; }), 7);
  assert("int y[4]; int main() { y[0]=0; y[1]=1; y[2]=2; y[3]=3; return y[0]; }",
    ({ y[0]=0; y[1]=1; y[2]=2; y[3]=3; y[0]; }) ,0);
  assert("int y[4]; int main() { y[0]=0; y[1]=1; y[2]=2; y[3]=3; return y[1]; }",
    ({ y[0]=0; y[1]=1; y[2]=2; y[3]=3; y[1]; }) ,1);
  assert("int y[4]; int main() { y[0]=0; y[1]=1; y[2]=2; y[3]=3; return y[2]; }",
    ({ y[0]=0; y[1]=1; y[2]=2; y[3]=3; y[2]; }) ,2);
  assert("int y[4]; int main() { y[0]=0; y[1]=1; y[2]=2; y[3]=3; return y[3]; }",
    ({ y[0]=0; y[1]=1; y[2]=2; y[3]=3; y[3]; }) ,3);
  assert("int x; int main() { return sizeof(x); }", sizeof(x), 4);
  assert("int y[4]; int main() { return sizeof(y); }", sizeof(y), 16);

  assert("({ char x;x=1; char y=2; x; })", ({ char x;x=1; char y=2; x; }), 1);
  assert("({ char x;x=1; char y=2; y; })", ({ char x;x=1; char y=2; y; }), 2);
  assert("({ char x; sizeof(x); })", ({ char x; sizeof(x); }), 1);

  assert("({ char x[10]; sizeof(x); })", ({ char x[10]; sizeof(x); }), 10);
  assert("int sub_char(char a, char b, char c) { return a-b-c; } int main() { return sub_char(7, 3, 3); }",
    sub_char(7, 3, 3), 1);
  assert("({ int k = 8; k; })", ({ int k = 8; k; }), 8);

  assert("({ int k[3] = {}; k[2]; })", ({ int k[3] = {}; k[2]; }), 0);
  assert("({int k[3] = {10,20}; k[1]; })", ({int k[3] = {10,20}; k[1]; }), 20);
  assert("({int k[3] = {10,20}; k[2]; })", ({int k[3] = {10,20}; k[2]; }), 0);
  assert("({ int k[3][2] = {{0,1},{2,3},{4,5}}; k[2][1]; })",
    ({ int k[3][2] = {{0,1},{2,3},{4,5}}; k[2][1]; }), 5);
  assert("({ int k[3][2] = {}; k[2][1]; })", ({ int k[3][2] = {}; k[2][1]; }),0);
  assert("({ int k[3][2] = {{1}}; k[0][0]; })", ({ int k[3][2] = {{1}}; k[0][0]; }), 1);
  assert("({ int k[3][2] = {{1}}; k[0][1]; })", ({ int k[3][2] = {{1}}; k[0][1]; }), 0);

  assert("({ char *k = \"a  b\"; k[3] - k[0]; })", ({ char *k = "a  b"; k[3] - k[0]; }), 1);
  assert("({ char *k = \"\\a\"; *k; })", ({ char *k = "\a"; *k; }), 7);
  assert("({ char *k = \"\\b\"; *k; })", ({ char *k = "\b"; *k; }), 8);
  assert("({ char *k = \"\\f\"; *k; })", ({ char *k = "\f"; *k; }), 12);
  assert("({ char *k = \"\\n\"; *k; })", ({ char *k = "\n"; *k; }), 10);
  assert("({ char *k = \"\\r\"; *k; })", ({ char *k = "\r"; *k; }), 13);
  assert("({ char *k = \"\\t\"; *k; })", ({ char *k = "\t"; *k; }), 9);
  assert("({ char *k = \"\\\\\"; *k; })", ({ char *k = "\\"; *k; }), 92);
  assert("({ char *k = \"\\'\"; *k; })", ({ char *k = "\'"; *k; }), 39);
  assert("({ char *k = \"'\"; *k; })", ({ char *k = "'"; *k; }), 39);
  assert("({ char *k = \"\"\"; *k; })", ({ char *k = "\""; *k; }), 34);

  assert("({ ({int a = 1; 10-a;}); })", ({ ({int a = 1; 10-a;}); }), 9);
  assert("int main() { int sum = 0; int a = 1; sum = sum + a; { int a = 2; sum = sum + a; } sum = sum + a; { int a = 3; sum = sum + a; } sum = sum + a; ({int a = 4; sum =sum+a;a;}); sum = sum + a; return sum; }",
  summing(), 13);

  assert("({ int a = 0; for (int i = 0; i<3; i = i + 1) { a = a + 1; } a; })",
    ({ int a = 0; for (int i = 0; i<3; i = i + 1) { a = a + 1; } a; }), 3);
  assert("int ret10(){ int i = 0; for (;;) { if (i<10) i = i + 1; else return i; } }", ret10() , 10);

//  assert(4, sizeof("abc"), "sizeof(\"abc\")");

  printf("\nOK\n");
  return 0;
}

int calc(int i,int j){int k; k =10;return i-j+k;}
int gva;

int summing() {
  int sum = 0;
  int a = 1;
  sum = sum + a;
  {
    int a = 2;
    sum = sum + a;
  }
  sum = sum + a;
  {
    int a = 3;
    sum = sum + a;
  }
  sum = sum + a;
  ({ int a = 4; sum = sum + a; a; });
  sum = sum + a;

  return sum;
}
