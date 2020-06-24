int main() {
  return({int a=1;switch(a){case 0:break;case 1:a=2;switch(a){case 0:a=100;break;case 1:a=200;break;default:a=300;}break;default:a=400;}a;});
}
