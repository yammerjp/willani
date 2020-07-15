#define DEFINE_ADD(a,b,c)  a+(b+(c))
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

int omit_return() {
  1;
}
void omit_return_value() {
  int a = 2;
  a = 2+3;
  return;
}

void nop_func() {}

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

static int static_fn() { return 3; }

char ar_char[3] = {1,2,3};
int ar_int[3] = {1,2,3};
long ar_long[3] = {1,2,3};
char hello_world[20] = "hello,world!";

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
  assert("sizeof(int **)", sizeof(((int**))), 8);

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

  assert("sizeof(\"abc\")", sizeof("abc") , 4);

  assert("({ _Bool _Boolean = 1; _Boolean;})", ({ _Bool boolean = 1; boolean; }), 1==1);
  assert("({ _Bool _Boolean = 0; _Boolean;})", ({ _Bool boolean = 0; boolean; }), 1==0);

  // allow variable that start '_'
  assert("({ _Bool _Boolean = 0; _Boolean; })", ({ _Bool _Boolean = 0; _Boolean; }), 0);

  assert("'c'", 'c', 'c');
  assert("'&'", '&', '&');
  assert("'\\''", '\'', '\'');
  assert("'\\\\'", '\\', '\\');
  assert("'\\n'", '\n', '\n');

  assert("({ struct {char a; int b; char c;} x; x.a=2; x.b=2; x.c=3; x.b; })", ({ struct {char a; int b; char c;} x; x.a=2; x.b=2; x.c=3; x.b; }), 2);

  assert("struct {int a; int b;} x; x.a=1; x.b=2; x.a;", ({ struct {int a; int b;} x; x.a=1; x.b=2; x.a; }), 1);
  assert("struct {int a; int b;} x; x.a=1; x.b=2; x.b;", ({ struct {int a; int b;} x; x.a=1; x.b=2; x.b; }), 2);
  assert("struct {char a; int b; char c;} x; x.a=1; x.b=2; x.c=3; x.a;", ({ struct {char a; int b; char c;} x; x.a=1; x.b=2; x.c=3; x.a; }), 1);
  assert("struct {char a; int b; char c;} a; x.b=1; x.b=2; x.c=3; x.b;", ({ struct {char a; int b; char c;} x; x.b=1; x.b=2; x.c=3; x.b; }), 2);

  assert("struct {char a; int b; char c;} x; x.a=1; x.b=2; x.c=3; x.c;", ({ struct {char a; int b; char c;} x; x.a=1; x.b=2; x.c=3; x.c; }), 3);

  assert("struct {int a; int b;} x[3]; int *p=x; p[0]=0; x[0].a;", ({ struct {int a; int b;} x[3]; int *p=x; p[0]=0; x[0].a; }), 0);
  assert("struct {int a; int b;} x[3]; int *p=x; p[1]=1; x[0].b;", ({ struct {int a; int b;} x[3]; int *p=x; p[1]=1; x[0].b; }), 1);
  assert("struct {int a; int b;} x[3]; int *p=x; p[2]=2; x[1].a;", ({ struct {int a; int b;} x[3]; int *p=x; p[2]=2; x[1].a; }), 2);
  assert("struct {int a; int b;} x[3]; int *p=x; p[3]=3; x[1].b;", ({ struct {int a; int b;} x[3]; int *p=x; p[3]=3; x[1].b; }), 3);
  assert("struct {int a[3]; int b[5];} x; int *p=&x; x.a[0]=6; p[0];", ({ struct {int a[3]; int b[5];} x; int *p=&x; x.a[0]=6; p[0]; }), 6);
  assert("struct {int a[3]; int b[5];} x; int *p=&x; x.b[0]=7; p[3];", ({ struct {int a[3]; int b[5];} x; int *p=&x; x.b[0]=7; p[3]; }), 7);

  assert("struct { struct { int b; } a; } x; x.a.b=6; x.a.b;", ({ struct { struct { int b; } a; } x; x.a.b=6; x.a.b; }), 6);

  assert("struct {int a;} x; sizeof(x);", ({ struct {int a;} x; sizeof(x); }), 4);
  assert("struct {int a; int b;} x; sizeof(x);", ({ struct {int a; int b;} x; sizeof(x); }), 8);
  assert("struct {int a[3];} x; sizeof(x);", ({ struct {int a[3];} x; sizeof(x); }), 12);
  assert("struct {int a;} x[4]; sizeof(x);", ({ struct {int a;} x[4]; sizeof(x); }), 16);
  assert("struct {int a[3];} x[2]; sizeof(x)};", ({ struct {int a[3];} x[2]; sizeof(x); }), 24);
  assert("struct {char a; char b;} x; sizeof(x);", ({ struct {char a; char b;} x; sizeof(x); }), 2);
  assert("struct {char a; int b;} x; sizeof(x);", ({ struct {char a; int b;} x; sizeof(x); }), 8);

  assert("struct t {int a; int b;} x; struct t y; sizeof(y);", ({ struct t {int a; int b;} x; struct t y; sizeof(y); }), 8);
  assert("struct t {int a; int b;}; struct t y; sizeof(y);", ({ struct t {int a; int b;}; struct t y; sizeof(y); }), 8);
  assert("struct t {char a[2];}; { struct t {char a[4];}; } struct t y; sizeof(y);", ({ struct t {char a[2];}; { struct t {char a[4];}; } struct t y; sizeof(y); }), 2);
  assert("struct t {int x;}; int t=1; struct t y; y.x=2; t+y.x;", ({ struct t {int x;}; int t=1; struct t y; y.x=2; t+y.x; }), 3);

  assert("struct t {char a;} x; struct t *y = &x; x.a=3; y->a;", ({ struct t {char a;} x; struct t *y = &x; x.a=3; y->a; }), 3);
  assert("struct t {char a;} x; struct t *y = &x; y->a=3; x.a;", ({ struct t {char a;} x; struct t *y = &x; y->a=3; x.a; }), 3);

  assert("typedef int t; t x=1; x;", ({ typedef int t; t x=1; x; }), 1);
  assert("typedef int t; ({t t=1; t;)}", ({ typedef int t; ({t t=1; t; }); }), 1);
  assert("typedef struct {int a;} t; t x; x.a=1; x.a;", ({ typedef struct {int a;} t; t x; x.a=1; x.a; }), 1);
  assert("typedef struct {int a;} t; { typedef int t; } t x; x.a=2; x.a;", ({ typedef struct {int a;} t; { typedef int t; } t x; x.a=2; x.a; }), 2);

  // static function (file private function)
  assert("static_fn()", static_fn(), 3);

  // continue statement
  assert( 
    "({ int ct = 0; for( int i = 0; i <10; i=i+1) { if(i<5) continue; ct = ct + 1; } ct; })",
    ({ int ct = 0; for( int i = 0; i <10; i=i+1) { if(i<5) continue; ct = ct + 1; } ct; }),
    5
  );
  assert( 
    "({ int ct=0; for(int i=0;i<10;i=i+1)for(int j=0;j<10;j=j+1){ if(j<5)continue; ct=ct+1; } ct; })",
    ({ int ct=0; for(int i=0;i<10;i=i+1)for(int j=0;j<10;j=j+1){ if(j<5)continue; ct=ct+1; } ct; }),
    50
  );

  // break statement
  assert(
    "({ int ct=0; for(int i=0;i<10;i=i+1)for(int j=0;j<10;j=j+1){ if(j==5)break; ct=ct+1; } ct; })",
    ({ int ct=0; for(int i=0;i<10;i=i+1)for(int j=0;j<10;j=j+1){ if(j==5)break; ct=ct+1; } ct; }),
    50
  );

  // switch statement
  assert(
    "({ int a=1; switch(a) { case 0: break; case 1: a=10; switch (a) { case 10: a=100; break; case 1: a=200; break; } } a; })",
    ({ int a=1; switch(a) { case 0: break; case 1: a=10; switch (a) { case 10: a=100; break; case 1: a=200; break; } } a; }),
    100
  );
  assert(
    "({int a=1;switch(a){case 0:break;case 1:a=2;switch(a){case 0:a=100;break;case 1:a=200;break;default:a=300;}break;default:a=400;}a;})",
    ({int a=1;switch(a){case 0:break;case 1:a=2;switch(a){case 0:a=100;break;case 1:a=200;break;default:a=300;}break;default:a=400;}a;}),
    300
  );
    assert("({int a;switch(0){case 1:a = 0;default:a=1;}a;})", ({int a;switch(0){case 1:a = 0;default:a=1;}a;}), 1);

  assert("222%4", 222%4, 2);
  assert("10%3", 10%3, 1);
  assert("({int a; a=4,10,101,a=a+3, a-7;})", ({int a; a=4,10,101,a=a+3, a-7;}), 0);

  assert( "({int a;a=10;a+=3;a;})", ({int a;a=10;a+=3;a;}), 13);
  assert( "({ int a[2] = {100, 200}; int *p; p = a; p += 1; *p; })", ({ int a[2] = {100, 200}; int *p; p = a; p += 1; *p; }), 200);
  assert( "({int a;a=10;a-=3;a;})", ({int a;a=10;a-=3;a;}), 7);
  assert( "({ int a[2] = {100, 200}; int *p; p = a+1; p -= 1; *p; })", ({ int a[2] = {100, 200}; int *p; p = a+1; p -= 1; *p; }), 100);
  assert( "({int a;a=10;a*=3;a;})", ({int a;a=10;a*=3;a;}), 30);
  assert( "({int a;a=10;a/=3;a;})", ({int a;a=10;a/=3;a;}), 3);
  assert( "({int a;a=10;a%=3;a;})", ({int a;a=10;a%=3;a;}), 1);

  assert("!0", !0, 1);
  assert("!1", !1, 0);
  assert("!1231", !1231, 0);

  assert("0x00", 0x00, 0);
  assert("0b00", 0b00, 0);
  assert("0b111", 0b111, 7);
  assert("0B111", 0b111, 7);
  assert("0x111", 0x111, 273);
  assert("0X111", 0X111, 273);
  assert("0xffff", 0xffff, 65535);

  assert("~0", ~0, -1);
  assert("~(-1)", ~(-1), 0);
  assert("~-1", ~-1, 0);
  assert("~(~93)", ~(~93), 93);
  assert("~~93", ~~93, 93);
  assert("~0b11111111111111111111111111111111", ~0b11111111111111111111111111111111, 0);

  assert("({ int p[10]; int *a=p+9; int *b=p; a-b; })", ({ int p[10]; int *a=p+9; int *b=p; a-b; }), 9);

  assert("~~93", ~~93, 93);

  assert("({ int i = 0; ++i; })", ({ int i = 0; ++i; }), 1);
  assert("({ int i = 0; ++i; i;})", ({ int i = 0; ++i; i;}), 1);
  assert("({ int i = 1; --i; })", ({ int i = 1; --i; }), 0);
  assert("({ int i = 1; --i; i;})", ({ int i = 1; --i; i;}), 0);

  assert("({ int a = 1; a++; })", ({ int a = 1; a++; }), 1);
  assert("({ int a = 1; a++,a; })", ({ int a = 1; a++,a; }), 2);
  assert("({ int a = 1; a--; })", ({ int a = 1; a--; }), 1);
  assert("({ int a = 1; a--,a; })", ({ int a = 1; a--,a; }), 0);

  assert( "({ int a = 0; 0==0 || a++!=0; a;})", ({ int a = 0; 0==0 || a++!=0; a;}), 0);
  assert( "({ int a = 0; 0!=0 || a++!=0; a;})", ({ int a = 0; 0!=0 || a++!=0; a;}), 1);

  assert( "0!=0 || 0!=0", 0!=0 || 0!=0, 0);
  assert( "0!=0 || 0==0", 0!=0 || 0==0, 1);
  assert( "0==0 || 0!=0", 0==0 || 0!=0, 1);
  assert( "0==0 || 0==0", 0==0 || 0==0, 1);

  assert( "0!=0 && 0!=0", 0!=0 && 0!=0, 0);
  assert( "0!=0 && 0==0", 0!=0 && 0==0, 0);
  assert( "0==0 && 0!=0", 0==0 && 0!=0, 0);
  assert( "0==0 && 0==0", 0==0 && 0==0, 1);

  assert( "0==0 || 0!=0 && 0==0", 0==0 || 0!=0 && 0==0, 1);

  assert("0==0 ? 100 : 200", (0==0 ? 100 : 200), 100);
  assert("1==0 ? 100 : 200", 1==0 ? 100 : 200, 200);

  assert( "0b0101 & 0b0011", 0b0101 & 0b0011, 0b0001);
  assert( "0b0101 | 0b0011", 0b0101 | 0b0011, 0b0111);
  assert( "0b0101 ^ 0b0011", 0b0101 ^ 0b0011, 0b0110);
  assert( "0b0101 ^ 0b0011 | 0b1111", 0b0101 ^ 0b0011 | 0b1111, 0b1111);
  assert( "0b0101 ^ 0b0011 & 0b0000", 0b0101 ^ 0b0011 & 0b0000, 0b0101);
  assert( "0b0101 | 0b0011 & 0b0000", 0b0101 | 0b0011 & 0b0000, 0b0101);

  assert("({ int *x[3]; sizeof(x); })", ({ int *x[3]; sizeof(x); }), 24);
  assert("({ int (*x)[3]; sizeof(x); })", ({ int (*x)[3]; sizeof(x); }), 8);
  assert("({ int *x[3]; int y; x[0]=&y; y=3; x[0][0]; })", ({ int *x[3]; int y; x[0]=&y; y=3; x[0][0]; }), 3);
  assert("({ int x[3]; int (*y)[3]=x; y[0][0]=4; y[0][0]; })", ({ int x[3]; int (*y)[3]=x; y[0][0]=4; y[0][0]; }), 4);

  assert("({enum agreement { hello, goodbye, }; hello;})", ({enum agreement { hello, goodbye, }; hello;}), 0);
  assert("({ enum { Hello, GoodBye, } a; a = GoodBye; a; })", ({ enum { Hello, GoodBye, } a; a = GoodBye; a; }), 1);
  assert("enum { zero, one, two }; zero;", ({ enum { zero, one, two }; zero; }), 0);
  assert("enum { zero, one, two }; one;", ({ enum { zero, one, two }; one; }), 1);
  assert("enum { zero, one, two }; two;", ({ enum { zero, one, two }; two; }), 2);
  assert("enum { five=5, six, seven }; five;", ({ enum { five=5, six, seven }; five; }), 5);
  assert("enum { five=5, six, seven }; six;", ({ enum { five=5, six, seven }; six; }), 6);
  assert("enum { zero, five=5, three=3, four }; zero;", ({ enum { zero, five=5, three=3, four }; zero; }), 0);
  assert("enum { zero, five=5, three=3, four }; five;", ({ enum { zero, five=5, three=3, four }; five; }), 5);
  assert("enum { zero, five=5, three=3, four }; three;", ({ enum { zero, five=5, three=3, four }; three; }), 3);
  assert("enum { zero, five=5, three=3, four }; four;", ({ enum { zero, five=5, three=3, four }; four; }), 4);
  assert("enum { zero, one, two } x; sizeof(x);", ({ enum { zero, one, two } x; sizeof(x); }), 4);
  assert("enum t { zero, one, two }; enum t y; sizeof(y);", ({ enum t { zero, one, two }; enum t y; sizeof(y); }), 4);

  nop_func();
  omit_return_value();

  // preprocessor
  assert("DEFINE_ADD((1+2),(3+(4)),5)", DEFINE_ADD((1+2),(3+(4)),5), 15);
  #define DEFINE_3 3
  assert("DEFINE_3", DEFINE_3, 3);

  // allow omit return statement
  assert("omit_return()", omit_return(), 0);

  // allow typedef struct tag before define members
  assert(
    "({ typedef struct t t; struct t { int mem; }; t x; x.mem=1; })",
    ({ typedef struct t t; struct t { int mem; }; t x; x.mem=1; }),
    1
  );

  // const variable
  assert( "({const int a = 3;a;})", ({const int a = 3;a;}), 3);

  assert("({ int a[4] = \"abc\"; a[2]; })", ({ int a[4] = "abc"; a[2]; }), 'c');
  assert(
    "({ char a[4][10] = { \"abc\", \"def\", \"fghijklm\", \"nm\" }; a[3][1]; })",
    ({ char a[4][10] = { "abc", "def", "fghijklm", "nm" }; a[3][1]; }),
    'm'
  );

  // initialize global variable
  assert("char ar_char[3] = {1,2,3}; // 2nd item (global variable)", ar_char[1], 2);
  assert("int ar_int[3] = {1,2,3}; // 2nd item (global variable)", ar_int[1], 2);
  assert("long ar_long[3] = {1,2,3}; // 2nd item (global variable)", ar_long[1], 2);
  assert("char hello_world[20] = \"hello,world!\"; // 10th item (global variable)", hello_world[10], 'd');

  // omit array length with declaration
  assert(
    "({ char hello_world[] = \"hello,world!\"; sizeof(hello_world); })",
    ({ char hello_world[] = "hello,world!"; sizeof(hello_world); }), 13
  );
  assert(
    "({ int a[] = {10-1,2,3}; sizeof(a); })",
    ({ int a[] = {10-1,2,3}; sizeof(a); }), 3*4
  );
  assert(
    "({ int b[][2] = {{1},{2},{3}}; sizeof(b); })",
    ({ int b[][2] = {{1},{2},{3}}; sizeof(b); }), 3*2*4
  );
  assert(
    "({ int c[][2][2] = {{{1}},{{2}},{{3}}}; sizeof(c); })",
    ({ int c[][2][2] = {{{1}},{{2}},{{3}}}; sizeof(c); }), 3*2*2*4
  );
  assert(
    "({ int d[] = { (0,1), ({1+2;}), 3}; sizeof(d); })",
    ({ int d[] = { (0,1), ({1+2;}), 3}; sizeof(d); }), 3*4
  );
  assert(
    "({ int d[] = { (0,1), ({1+2;}), 3}; d[1]; })",
    ({ int d[] = { (0,1), ({1+2;}), 3}; d[1]; }), 3
  );
  assert(
    "({ struct { char m1; int m2; char m3; } s = { 10,20,30 }; s.m3; })",
    ({ struct { char m1; int m2; char m3; } s = { 10,20,30 }; s.m3; }),
    30
  );
  assert(
    "({ struct { char m1; int m2; char m3; } s = { 10,20,30 }; s.m3; })",
    ({ int p = 100;struct { char m1; int m2; char m3; } s = { 10,p,30 }; s.m2; }),
    100
  );

  // type cast
  assert("(int)8590066177", (int)8590066177, 131585);
  assert("(char)8590066177", (char)8590066177, 1);
  assert("(_Bool)1", (_Bool)1, 1);
  assert("(_Bool)2", (_Bool)2, 1);
  assert("(_Bool)(char)256", (_Bool)(char)256, 0);
  assert("(long)1", (long)1, 1);
  assert("(long)&*(int *)0", (long)&*(int *)0, 0);
  assert("int x=5; long y=(long)&x; *(int*)y", ({ int x=5; long y=(long)&x; *(int*)y; }), 5);

  // bit shift
  assert("1<<0", 1<<0, 1);
  assert("1<<3", 1<<3, 8);
  assert("5<<1", 5<<1, 10);
  assert("5>>1", 5>>1, 2);
  assert("-1>>1", -1>>1, -1);
  assert("int i=1; i<<0;", ({ int i=1; i<<=0; i; }), 1);
  assert("int i=1; i<<3;", ({ int i=1; i<<=3; i; }), 8);
  assert("int i=5; i<<1;", ({ int i=5; i<<=1; i; }), 10);
  assert("int i=5; i>>1;", ({ int i=5; i>>=1; i; }), 2);
  assert("-1", -1, -1);
  //assert("int i=-1; i;", ({ int i=-1; i; }), -1);
  //assert("int i=1; i>>1;", ({ int i=-1; i>>=1; i; }), -1);

  #define DEFINED_IFNDEF_DIRECTIVE 1
  #ifndef DEFINED_IFNDEF_DIRECTIVE
  assert("The assert will not be run because of ifndef directive.", 1, 0);
  #endif

  #ifndef UNDEFINED_IFNDEF_DIRECTIVE
  assert("The assert will be run because of ifndef directive.", 0, 0);
  #endif

  #define DEFINED 1
  #ifndef DEFINED
  #ifndef DEFINING
  assert("The assert will not be run because of nested ifndef directive.", 1, 0);
  #endif
  #ifndef DEFINING
  #endif
  #endif

  #ifndef DEFINED
  assert("The assert will not be run because of ifndef else directive.", 1, 0);
  #else
  assert("The assert will be run because of ifndef else directive.", 0, 0);
  #endif


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
