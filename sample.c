int printf();

int main() {
  struct {
    int b;
    char c;
  } x;
  x.b=2;
  x.c=3;

  int p = x.b;
  printf("%d\n", p);
  return 0;
}
