int main() {
  int i = 0;
  for (;;) {
    if (i<3)
      i = i + 1;
    else
      return i;
  }
}
