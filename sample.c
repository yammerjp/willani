#define DEFINED 1
#ifndef DEFINED
#ifndef DEFINING
int a = 2;
#endif
#ifndef DEFINING
#endif
#endif
int a = 1;

int main () {
  return a;
}
