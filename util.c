
int absol(int a){
  if (a < 0) return a * -1;
  return a;
}

int rondo(float x) { 
  if ((x - (int)x) < .5) {
    return (int)x; 
  } else {
    return (int)x + 1;
  }
}
