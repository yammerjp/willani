int main() {
  return ({ int ct=0; for(int i=0;i<10;i=i+1)for(int j=0;j<10;j=j+1){ if(j==5)break; ct=ct+1; } ct; });
}
