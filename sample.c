int printf(char *p);
// this is comment
int main() {
  char *s = "hello,world!";
  /*
     this is comment
  */
  printf(s);
  return *s - *"h";
}
