#define DEFINED 1
int p = 1;
#ifndef DEFINED
int a = 2;
#endif
int a = 1;

int main () {
  return a;
}
