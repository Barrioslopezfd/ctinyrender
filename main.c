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
typedef struct { int x; int y; } vec2;

void linelow(vec2, vec2, BMImage*, BMColor);
void linehigh(vec2, vec2, BMImage*, BMColor);
void line(vec2, vec2, BMImage*, BMColor);
void triangle(vec2, vec2, vec2, BMImage*, BMColor);
void modelrender(OBJModel, BMImage*);
void sortbyY(vec2 *, vec2, vec2, vec2);

int main(int argc, char *argv[])
{
  BMImage BMI = BMSet(128, 128);
  FILE *f = BMCreate();
  
  OBJModel model = modelget();
  if (argc == 1) {
    // modelrender(model, &BMI);
    vec2 v0; 
    vec2 v1;
    vec2 v2;
    v0 = (vec2){ .x = 7,  .y = 45, };
    v1 = (vec2){ .x = 35, .y = 100, };
    v2 = (vec2){ .x = 45, .y = 60, };
    triangle(v0, v1, v2, &BMI, RED);
    v0 = (vec2){ .x = 120, .y = 35 };
    v1 = (vec2){ .x = 90,  .y = 5 };
    v2 = (vec2){ .x = 45,  .y = 110 };
    triangle(v0, v1, v2, &BMI, WHITE);
    v0 = (vec2){ .x = 115, .y = 83 };
    v1 = (vec2){ .x = 80,  .y = 90 };
    v2 = (vec2){ .x = 85,  .y = 120 };
    triangle(v0, v1, v2, &BMI, GREEN);
  } else {
    srand(time(NULL));
    for (int i = 0; i < (1<<24); i++) {
      vec2 v0 = { .x = rand()%64, .y = rand()%64 };
      vec2 v1 = { .x = rand()%64, .y = rand()%64 };
      uint32_t color = (rand()%255 << 16) | (rand()%255 << 8) | (rand()%255);
      line(v0, v1, &BMI, color);
    }
  }
  BMWrite(&BMI, f);
  fclose(f);
  free(BMI.pixels);
  return 0;
}

void linelow(vec2 v0, vec2 v1, BMImage *BMI, BMColor color) 
{
  int dx = v1.x - v0.x;
  int dy = v1.y - v0.y;
  int yi = 1; if (dy < 0) {
    yi = -1;
    dy = -dy;
  }
  int D = (2 * dy) - dx;
  int y = v0.y;
  for (int x = v0.x; x <= v1.x; x++) {
    BMSetPixel(BMI, x, y, color);
    if (D > 0) {
      y = y + yi;
      D = D + (2 * (dy - dx));
    } else {
      D = D + 2*dy;
    }
  }
}

void linehigh(vec2 v0, vec2 v1, BMImage *BMI, BMColor color) 
{
  int dx = v1.x - v0.x;
  int dy = v1.y - v0.y;
  int xi = 1;
  if (dx < 0) {
    xi = -1;
    dx = -dx;
  }
  int D = (2 * dx) - dy;
  int x = v0.x;
  for (int y = v0.y; y <= v1.y; y++) {
    BMSetPixel(BMI, x, y, color);
    if (D > 0) {
      x = x + xi;
      D = D + (2 * (dx - dy));
    } else {
      D = D + 2*dx;
    }
  }
}

void line(vec2 v0, vec2 v1, BMImage *BMI, BMColor color) 
{
  int ay = absol(v1.y - v0.y);
  int ax = absol(v1.x - v0.x);
  if (ay < ax) {
    if (v0.x > v1.x)
      linelow(v1, v0, BMI, color);
    else
      linelow(v0, v1, BMI, color);
  } else {
    if (v0.y > v1.y)
      linehigh(v1, v0, BMI, color);
    else
      linehigh(v0, v1, BMI, color);
  }
}

void triangle(vec2 v0, vec2 v1, vec2 v2, BMImage *BMI, BMColor color)
{
  vec2 vecs[3];
  sortbyY(vecs, v0, v1, v2);

  if (vecs[1].y != vecs[0].y) {
    for (int y = vecs[0].y; y <= vecs[1].y; y++) {
      int x1 = vecs[0].x + ((vecs[2].x - vecs[0].x)*(y - vecs[0].y)) / (vecs[2].y - vecs[0].y);
      int x2 = vecs[0].x + ((vecs[1].x - vecs[0].x)*(y - vecs[0].y)) / (vecs[1].y - vecs[0].y);
      if ( x1 < x2 ) {
        for ( int x = x1; x < x2; x++ ) {
          BMSetPixel(BMI, x, y, color);
        }
      } else {
        for ( int x = x2; x < x1; x++ ) {
          BMSetPixel(BMI, x, y, color);
        }
      }
    }
  }

  if (vecs[2].y != vecs[1].y) { 
    for (int y = vecs[1].y; y <= vecs[2].y; y++) {
      int x1 = vecs[0].x + ((vecs[2].x - vecs[0].x)*(y - vecs[0].y)) / (vecs[2].y - vecs[0].y);
      int x2 = vecs[1].x + ((vecs[2].x - vecs[1].x)*(y - vecs[1].y)) / (vecs[2].y - vecs[1].y);
      if ( x1 < x2 ) {
        for ( int x = x1; x < x2; x++ ) {
          BMSetPixel(BMI, x, y, color);
        }
      } else {
        for ( int x = x2; x < x1; x++ ) {
          BMSetPixel(BMI, x, y, color);
        }
      }
    }
  }
  // line(v0, v1, BMI, BLACK);
  // line(v1, v2, BMI, BLACK);
  // line(v2, v0, BMI, BLACK);
}

void modelrender(OBJModel model, BMImage *BMI)
{
  for (int i = 0; i < model.facec; i+=3) {
    vec2 v0 = {
      .x = rondo((float)BMI->BIH.BIWidth/2 * (model.vertices[(model.faces[i] - 1) * 3]+1)),
      .y = rondo((float)BMI->BIH.BIHeight/2 * (model.vertices[(model.faces[i] - 1) * 3 + 1]+1)),
    };
    vec2 v1 = {
      .x = rondo((float)BMI->BIH.BIWidth/2 * (model.vertices[(model.faces[i+1] - 1) * 3]+1)),
      .y = rondo((float)BMI->BIH.BIHeight/2 * (model.vertices[(model.faces[i+1] - 1) * 3 +1]+1)),
    };
    vec2 v2 = {
      .x = rondo((float)BMI->BIH.BIWidth/2 * (model.vertices[(model.faces[i+2] - 1) * 3]+1)),
      .y = rondo((float)BMI->BIH.BIHeight/2 * (model.vertices[(model.faces[i+2] - 1) * 3 +1]+1)),
    };
    triangle(v0, v1, v2, BMI, WHITE);
  }
}

void sortbyY(vec2 *vecs, vec2 v0, vec2 v1, vec2 v2)
{
  if (v0.y < v1.y && v0.y < v2.y) {
    vecs[0] = v0;
    if ( v1.y < v2.y ) {
      vecs[1] = v1;
      vecs[2] = v2;
    } else {
      vecs[1] = v2;
      vecs[2] = v1;
    }
  } else if ( v1.y < v0.y && v1.y < v2.y ) {
    vecs[0] = v1;
    if ( v0.y < v2.y ) {
      vecs[1] = v0;
      vecs[2] = v2;
    } else {
      vecs[1] = v2;
      vecs[2] = v0;
    }
  } else {
    vecs[0] = v2;
    if ( v0.y < v1.y ) {
      vecs[1] = v0;
      vecs[2] = v1;
    } else {
      vecs[1] = v1;
      vecs[2] = v0;
    }
  }
}

