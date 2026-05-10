#include <stdlib.h>
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
void triangle(int, int, int, int, int, int, BMImage*, BMColor);

int main(int argc, char *argv[])
{
  BMImage BMI = BMSet(1024, 1024);
  FILE *f = BMCreate();
  
  OBJModel model = modelget();

  for (int i = 0; i < model.facec; i+=3) {
    int ax = rondo((float)BMI.BIH.BIWidth/2 * (model.vertices[(model.faces[i] - 1) * 3]+1));
    int ay = rondo((float)BMI.BIH.BIHeight/2 * (model.vertices[(model.faces[i] - 1) * 3 + 1]+1));
    int bx = rondo((float)BMI.BIH.BIWidth/2 * (model.vertices[(model.faces[i+1] - 1) * 3]+1));
    int by = rondo((float)BMI.BIH.BIHeight/2 * (model.vertices[(model.faces[i+1] - 1) * 3 +1]+1));
    int cx = rondo((float)BMI.BIH.BIWidth/2 * (model.vertices[(model.faces[i+2] - 1) * 3]+1));
    int cy = rondo((float)BMI.BIH.BIHeight/2 * (model.vertices[(model.faces[i+2] - 1) * 3 +1]+1));
    
    triangle(ax, ay, bx, by, cx, cy, &BMI, WHITE);
  }

  BMWrite(&BMI, f);
  fclose(f);
  free(BMI.pixels);
  return 0;
}

void line(int ax, int ay, int bx, int by, BMImage *BMI, BMColor color) 
{
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

void triangle(int ax, int ay, int bx, int by, int cx, int cy, BMImage *BMI, BMColor color)
{
  line(ax, ay, bx, by, BMI, color);
  line(bx, by, cx, cy, BMI, color);
  line(cx, cy, ax, ay, BMI, color);
}
