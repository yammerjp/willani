int main() {
  assert(
    "({ struct { char m1; int m2; char m3; } s = { 10,20,30 }; s.m3; })",
    ({ int p = 100;struct { char m1; int m2; char m3; } s = { 10,p,30 }; s.m2; }),
    100
  );
}
