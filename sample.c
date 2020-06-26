int printf(char *, int);

int main() {
  return ({ int p[10]; int *a=p+9; int *b=p; a-b; });
}
