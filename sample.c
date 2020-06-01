int main() {
  int x[2][3];
  int *y;
  y=x;
  *(y+2)=3;
  return **(x+1);
}
