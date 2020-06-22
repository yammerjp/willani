int main() {
  typedef struct {int a;} t;
  {
    typedef int t;
    int x;
    int y;
  }
  t p;
  p.a=2;
  return p.a;
}
