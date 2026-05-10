
int absol(int a, int b){
  int c = a - b;
  if (c < 0) return c * -1;
  return c;
}

int rondo(float x) { 
  if ((x - (int)x) < .5) {
    return (int)x; 
  } else {
    return (int)x + 1;
  }
}
