#define DEFINED 1
#ifndef DEFINED
#ifndef DEFINING
int a = 2;
#endif
#ifndef DEFINING
#endif
#else
int a = 1;
#endif

int main () {
  return a;
}
