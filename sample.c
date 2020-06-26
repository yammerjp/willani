int main() {
  return
  ({ int a[2] = {100, 200}; int *p; p = a+1; p -= 1; *p; });
}
