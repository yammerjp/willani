int main() {
  int a = 1;
  switch(a) {
    case 0:
    break;
    case 1:
    a = 10;
    switch (a) {
      case 10:
        a = 100;
        break;
      case 1:
        a = 200;
        break;
    }
  }
  return a;
}
