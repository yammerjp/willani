int main() {
  return
  ({ int a[2] = {100, 200}; int *p; p = a; p += 1; *p; });
}
