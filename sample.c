int main() {
  ({ int *x; sizeof(x); });
  ({ int *x;int y; x = &y; *x = 3 ; *x;});
}
