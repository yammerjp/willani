int sum;
int printf();
int main() {
  sum = 0;
  int a = 1;

  sum = sum + a;
  printf("%d\n", a);
  if (1) {
    int i = 2;
  }

  {
    int a = 2;
    sum = sum + a;
    printf("%d\n", a);
  }

  sum = sum + a;
  printf("%d\n", a);

  {
    int a = 3;
    sum = sum + a;
    printf("%d\n", a);
  }

  sum = sum + a;
  printf("%d\n", a);

  printf("%d\n", ({int a = 4; sum =sum+a;a;}));

  sum = sum + a;
  printf("%d\n", a);

  return sum;
}
