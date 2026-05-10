#include <time.h>
#include "bmpimage.h"
#include "util.h"
#include "obj.h"

#define RED     0x00FF0000
#define GREEN   0x0000FF00
#define BLUE    0x000000FF
#define YELLOW  0x00FFDE21
#define WHITE   0x00FFFFFF
#define BLACK   0x00000000

typedef uint32_t BMColor;

void line(int, int, int, int, BMImage*, BMColor);

int main(int argc, char *argv[]) {
  BMImage BMI = BMSet(64, 64);
  FILE *f = BMCreate();
  
  if (argc == 1){
    int ax =  7, ay =  3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53;
    int dx = 52, dy = 27;

    BMSetPixel(&BMI, ax, ay, WHITE);
    BMSetPixel(&BMI, bx, by, WHITE);
    BMSetPixel(&BMI, cx, cy, WHITE);
    BMSetPixel(&BMI, dx, dy, WHITE);

    line(ax, ay, bx, by, &BMI, RED);
    line(cx, cy, bx, by, &BMI, GREEN);
    line(ax, ay, cx, cy, &BMI, BLUE);
    line(bx, by, dx, dy, &BMI, YELLOW);

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

  int dx = bx - ax;
  int dy = by - ay;
  int D  = 2*dy - dx;
  int y  = ay;
  int yi = 1;

  if (dy < 0) {
   yi = -1;
   dy = -dy;
  }


  for (int x = ax; x < bx; x++) {
    if (steep)
      BMSetPixel(BMI, y, x, color);
    else
      BMSetPixel(BMI, x, y, color);
    if (D > 0) {
      y+=yi;
      D+=2 * (dy - dx);
    } else {
      D+= 2*dy;
    }
  }
}

