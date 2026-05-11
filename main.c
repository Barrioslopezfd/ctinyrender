#include <stdlib.h>
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

void linelow(int, int, int, int, BMImage*, BMColor);
void linehigh(int, int, int, int, BMImage*, BMColor);
void line(int, int, int, int, BMImage*, BMColor);
void triangle(int, int, int, int, int, int, BMImage*, BMColor);
void modelrender(OBJModel, BMImage);

int main(int argc, char *argv[])
{
  BMImage BMI = BMSet(128, 128);
  FILE *f = BMCreate();
  
  OBJModel model = modelget();

  // modelrender(model, BMI);
  if (argc == 1) {
    triangle(  7, 45, 35, 100, 45,  60, &BMI, RED);
    triangle(120, 35, 90,   5, 45, 110, &BMI, WHITE);
    triangle(115, 83, 80,  90, 85, 120, &BMI, GREEN);
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

void linelow(int x0, int y0, int x1, int y1, BMImage *BMI, BMColor color) 
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int yi = 1;
    if (dy < 0) {
        yi = -1;
        dy = -dy;
    }
    int D = (2 * dy) - dx;
    int y = y0;

    for (int x = x0; x <= x1; x++) {
        BMSetPixel(BMI, x, y, color);;
        if (D > 0) {
            y = y + yi;
            D = D + (2 * (dy - dx));
        } else {
            D = D + 2*dy;
        }
    }
}

void linehigh(int x0, int y0, int x1, int y1, BMImage *BMI, BMColor color) 
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int xi = 1;
    if (dx < 0) {
        xi = -1;
        dx = -dx;
    }
    int D = (2 * dx) - dy;
    int x = x0;

    for (int y = y0; y <= y1; y++) {
        BMSetPixel(BMI, x, y, color);;
        if (D > 0) {
            x = x + xi;
            D = D + (2 * (dx - dy));
        } else {
            D = D + 2*dx;
        }
    }
}

void line(int x0, int y0, int x1, int y1, BMImage *BMI, BMColor color) 
{
    if (absol(y1 - y0) < absol(x1 - x0)) {
        if (x0 > x1)
            linelow(x1, y1, x0, y0, BMI, color);
        else
            linelow(x0, y0, x1, y1, BMI, color);
    } else {
        if (y0 > y1)
            linehigh(x1, y1, x0, y0, BMI, color);
        else
            linehigh(x0, y0, x1, y1, BMI, color);
    }
}

void triangle(int ax, int ay, int bx, int by, int cx, int cy, BMImage *BMI, BMColor color)
{
  line(ax, ay, bx, by, BMI, color);
  line(bx, by, cx, cy, BMI, color);
  line(cx, cy, ax, ay, BMI, color);
}

void modelrender(OBJModel model, BMImage BMI)
{

  for (int i = 0; i < model.facec; i+=3) {
    int ax = rondo((float)BMI.BIH.BIWidth/2 * (model.vertices[(model.faces[i] - 1) * 3]+1));
    int ay = rondo((float)BMI.BIH.BIHeight/2 * (model.vertices[(model.faces[i] - 1) * 3 + 1]+1));
    int bx = rondo((float)BMI.BIH.BIWidth/2 * (model.vertices[(model.faces[i+1] - 1) * 3]+1));
    int by = rondo((float)BMI.BIH.BIHeight/2 * (model.vertices[(model.faces[i+1] - 1) * 3 +1]+1));
    int cx = rondo((float)BMI.BIH.BIWidth/2 * (model.vertices[(model.faces[i+2] - 1) * 3]+1));
    int cy = rondo((float)BMI.BIH.BIHeight/2 * (model.vertices[(model.faces[i+2] - 1) * 3 +1]+1));
    
    triangle(ax, ay, bx, by, cx, cy, &BMI, WHITE);
  }
}
