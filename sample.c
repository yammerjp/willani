#define STR "hello, world %d\n", 3
#define hoge fuga

int printf(char *, int);
int main() {
  printf(STR);
  return 0;
}
