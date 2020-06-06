int printf(char *p);
int main() {
  char *s = "hello,world!";
  printf(s);
  return *s - *"h";
}
