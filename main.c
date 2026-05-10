#include "bmpimage.h"
#include <time.h>

#define TRUE 1
#define FALSE 0

#define RED     0x00FF0000
#define GREEN   0x0000FF00
#define BLUE    0x000000FF
#define YELLOW  0x00FFDE21
#define WHITE   0x00FFFFFF
#define BLACK   0x00000000

typedef uint32_t BMColor;

void line(int, int, int, int, BMImage*, BMColor);
int absol(int, int);
int rondo(float);

int main(void) {
  BMImage BMI = BMSet(64, 64);
  FILE *f = BMCreate();
  
  if (0){
    int ax =  7, ay =  3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53;

    line(ax, ay, bx, by, &BMI, RED);
    line(cx, cy, bx, by, &BMI, GREEN);
    line(cx, cy, ax, ay, &BMI, YELLOW);
    line(ax, ay, cx, cy, &BMI, BLUE);

    BMSetPixel(&BMI, ax, ay, WHITE);
    BMSetPixel(&BMI, bx, by, WHITE);
    BMSetPixel(&BMI, cx, cy, WHITE);
  } else {
    srand(time(NULL));
    for (int i = 0; i < (1<<24); i++) {
        int ax = rand()%64, ay = rand()%64;
        int bx = rand()%64, by = rand()%64;
        uint32_t color = (rand()%255 << 16) | (rand()%255 << 8) | (rand()%255);
        line(ax, ay, bx, by, &BMI, color);
    }
  }
  
  BMWrite(&BMI, f);
  fclose(f);
  free(BMI.pixels);
  return 0;
}

void line(int ax, int ay, int bx, int by, BMImage *BMI, BMColor color) {

  int steep = FALSE;
  if (absol(by, ay) > absol(bx, ax)) {
    steep = TRUE;
    int aux = by;
    by = bx;
    bx = aux;

    aux = ay;
    ay = ax;
    ax = aux;
  }

  if (ax > bx) {
    int aux = bx;
    bx = ax;
    ax = aux;

    aux = by;
    by = ay;
    ay = aux;
  }

  if ((bx - ax) == 0 && (by - ay) == 0) return;

  int bigy = by - ay;
  int bigx = bx - ax;

  for (int i = 0; i <= bigx; i++) {
    int x = ax + i;
    int y = ay + rondo(bigy * (i / (float)bigx));

    if (steep)
      BMSetPixel(BMI, y, x, color);
    else
      BMSetPixel(BMI, x, y, color);
  }
}

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
