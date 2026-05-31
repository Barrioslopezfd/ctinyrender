#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include "bmpimage.h"
#include "util.h"
#include "obj.h"
#include "vector.h"

#define RED     0x00FF0000
#define GREEN   0x0000FF00
#define BLUE    0x000000FF
#define YELLOW  0x00FFDE21
#define WHITE   0x00FFFFFF
#define BLACK   0x00000000

#define WIDTH   1080
#define HEIGHT  1080
#define NBUFFER 2

typedef uint32_t BMColor;
typedef struct { vec3 v0; vec3 v1; vec3 v2; } tri;
typedef struct { vec3 v0; vec3 v1; vec3 v2; vec3 v3; } bbox;

void linelow(vec3, vec3, BMImage*, BMColor);
void linehigh(vec3, vec3, BMImage*, BMColor);
void line(vec3, vec3, BMImage*, BMColor);
void triangle(tri, bbox, BMImage*, ZBuffer, BMColor);
void modelrender(OBJModel, BMImage*[]);
void sortbyY(vec3 *, vec3, vec3, vec3);
void sortbyX(vec3 *, vec3, vec3, vec3);
bbox getbbox(tri);
int32_t getarea(tri);

int main(int argc, char *argv[])
{
  BMImage BMI = BMSet(WIDTH, HEIGHT);
  BMImage ZBF = BMSet(WIDTH, HEIGHT);
  BMImage *buffers[] = { &BMI, &ZBF };
  FILE *zbuffer = BMCreate("zbuffer.bmp");
  FILE *bmpimage = BMCreate("bmpimage.bmp");
  OBJModel model = modelget("obj/diablo3_pose/diablo3_pose.obj");
  bbox bb;

  if (argc == 1) {
    modelrender(model, buffers);
  } else if (argc == 2 && (int)argv[1][0] == 't') {
    tri t0 = {  .v0 = (vec3){ .x = 17, .y =  4, .z =   1 },
      .v1 = (vec3){ .x = 55, .y = 39, .z =   1 },
      .v2 = (vec3){ .x = 23, .y = 59, .z = 255 },
    };
    bb = getbbox(t0);
    ZBuffer ZBuff = ZBSet(WIDTH, HEIGHT);
    triangle(t0, bb, &BMI, ZBuff, RED);
  } else if (argc == 2 && argv[1][0] == 's') {
    
    #pragma omp parallel
    {
        uint32_t seed = (uint32_t)time(NULL) ^ omp_get_thread_num();

        #pragma omp for
        for (int i = 0; i < (1<<24); i++) {
            vec3 v0 = { .x = rand_r(&seed) % BMI.BIH.BIWidth, 
                        .y = rand_r(&seed) % BMI.BIH.BIHeight };
            vec3 v1 = { .x = rand_r(&seed) % BMI.BIH.BIWidth, 
                        .y = rand_r(&seed) % BMI.BIH.BIHeight };
            uint32_t color = (rand_r(&seed) % 255 << 16) | 
                             (rand_r(&seed) % 255 << 8) | 
                             (rand_r(&seed) % 255);
            line(v0, v1, &BMI, color);
        }
    }
} else {
    printf("Use either t or m for stress test or model render, no argument for triangles\n");
  }
  BMWrite(&BMI, bmpimage);
  BMWrite(&ZBF, zbuffer);
  fclose(bmpimage);
  free(BMI.pixels);
  return 0;
}

void linelow(vec3 v0, vec3 v1, BMImage *BMI, BMColor color) 
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

void linehigh(vec3 v0, vec3 v1, BMImage *BMI, BMColor color) 
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

void line(vec3 v0, vec3 v1, BMImage *BMI, BMColor color) 
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

void triangle(tri t, bbox bbox, BMImage *BMI, ZBuffer ZBuff, BMColor color)
{
  /*
   * SAREA = Sub-area
   * PV = Perpendicular VECTOR
   * V  = Vector
   * P  = A point that may or may not be
   *      be part of the triangle.
   *
   * A, B and C are the points that create the triangle.
   * 
   * I can create triangles with any 2 point that is
   * part of the triangle: (PAB), (PBC) and (PAC)
   * The PV of V(A, B) is PV(B, -A)
   * AREA(ABC) = SAREA(PAB) + SAREA(PBC) + SAREA(PAC)
   * AREA(ABC) = PV(AB) * V(AC) / 2
   *
   * P is part of the Triangle (ABC) if both rules are met:
   * - Each subarea is smaller or equal to the total area
   * - The summation of each sub-area equal to the total area
   *
   * the Division by 2 can be ignored because we only care about proportions.
   *
   * We first get Perpendicular Vector AB
   * ill have 3 sub triangles here
   * P12, 0P2 and 01P
   * P is the point i want to check
   * 0, 1 and 2 are from v0, v1 and v2
   *
   */

  int32_t sarea = getarea(t);
  if (sarea < 1) return;
  float tarea = 1/(float)sarea;
  float cx = (t.v0.x + t.v1.x + t.v2.x) / 3.0f;
  float cy = (t.v0.y + t.v1.y + t.v2.y) / 3.0f;

  #pragma omp parallel for collapse(2)
  for (uint32_t y = bbox.v0.y; y <= bbox.v2.y; y++)
  {
    for (uint32_t x = bbox.v0.x; x <= bbox.v1.x; x++)
    {
      tri t0 = (tri){ .v0 = (vec3){ .x = x, .y = y }, .v1 = t.v1, .v2 = t.v2 };
      tri t1 = (tri){ .v0 = t.v0, .v1 = (vec3){ .x = x, .y = y }, .v2 = t.v2 };
      tri t2 = (tri){ .v0 = t.v0, .v1 = t.v1, .v2 = (vec3){ .x = x, .y = y } };
      float alpha = getarea(t0) * tarea;
      float beta  = getarea(t1) * tarea;
      float gamma = getarea(t2) * tarea;

      if (alpha < 0   || beta < 0   || gamma < 0) continue;
      uint32_t z = color;
      if (color == BLACK) {
        uint8_t a = (uint8_t)(alpha*t.v0.z  + beta*t.v1.z   + gamma*t.v2.z);
        uint8_t b = (uint8_t)(beta*t.v0.z   + gamma*t.v1.z  + alpha*t.v2.z);
        uint8_t c = (uint8_t)(gamma*t.v0.z  + alpha*t.v1.z  + beta*t.v2.z);
        z = (a << 16) | (c << 8) | b;
      };
      uint32_t depth = (uint32_t)(alpha*t.v0.z  + beta*t.v1.z   + gamma*t.v2.z);
      uint32_t screen_index = y * BMI->BIH.BIWidth + x;
      if (depth > ZBuff[screen_index]) {
        ZBuff[screen_index] = depth;
        BMSetPixel(BMI, x, y, z);
      };
    };
  };
}

int32_t getarea(tri t){
  vec3 V01 = (vec3){ .x = t.v1.x - t.v0.x, .y = t.v1.y - t.v0.y };
  vec3 V02 = (vec3){ .x = t.v2.x - t.v0.x, .y = t.v2.y - t.v0.y };

  return V01.x * V02.y - V01.y * V02.x;
}

void modelrender(OBJModel model, BMImage *buffers[])
{
  BMImage *BMI = buffers[0];
  ZBuffer ZBuffColor = ZBSet(WIDTH, HEIGHT);
  ZBuffer ZBuffBlack = ZBSet(WIDTH, HEIGHT);
  for (int i = 0; i < model.nface; i+=3) {
    tri t = {
      .v0 = {
        .x = rondo((float)BMI->BIH.BIWidth/2  * (model.vertices[(model.faces[i] - 1) * 3    ]+1)),
        .y = rondo((float)BMI->BIH.BIHeight/2 * (model.vertices[(model.faces[i] - 1) * 3 + 1]+1)),
        .z = rondo((float)127.5               * (model.vertices[(model.faces[i] - 1) * 3 + 2]+1)),
      },
      .v1 = {
        .x = rondo((float)BMI->BIH.BIWidth/2  * (model.vertices[(model.faces[i+1] - 1) * 3    ]+1)),
        .y = rondo((float)BMI->BIH.BIHeight/2 * (model.vertices[(model.faces[i+1] - 1) * 3 + 1]+1)),
        .z = rondo((float)127.5               * (model.vertices[(model.faces[i+1] - 1) * 3 + 2]+1)),
      },
      .v2 = {
        .x = rondo((float)BMI->BIH.BIWidth/2  * (model.vertices[(model.faces[i+2] - 1) * 3    ]+1)),
        .y = rondo((float)BMI->BIH.BIHeight/2 * (model.vertices[(model.faces[i+2] - 1) * 3 + 1]+1)),
        .z = rondo((float)127.5               * (model.vertices[(model.faces[i+2] - 1) * 3 + 2]+1)),
      },
    };
    bbox bb = getbbox(t);
    uint32_t color = (rand()%255 << 16) | (rand()%255 << 8) | (rand()%255);
    triangle(t, bb, buffers[0], ZBuffColor, color);
    triangle(t, bb, buffers[1], ZBuffBlack, BLACK);
  }
}

void sortbyY(vec3 *vecs, vec3 v0, vec3 v1, vec3 v2)
{
vecs[0] = v0;
    vecs[1] = v1;
    vecs[2] = v2;

    if (vecs[0].y > vecs[1].y) { vec3 t = vecs[0]; vecs[0] = vecs[1]; vecs[1] = t; }
    if (vecs[1].y > vecs[2].y) { vec3 t = vecs[1]; vecs[1] = vecs[2]; vecs[2] = t; }
    if (vecs[0].y > vecs[1].y) { vec3 t = vecs[0]; vecs[0] = vecs[1]; vecs[1] = t; }
}

void sortbyX(vec3 *vecs, vec3 v0, vec3 v1, vec3 v2)
{
vecs[0] = v0;
    vecs[1] = v1;
    vecs[2] = v2;

    if (vecs[0].x > vecs[1].x) { vec3 t = vecs[0]; vecs[0] = vecs[1]; vecs[1] = t; }
    if (vecs[1].x > vecs[2].x) { vec3 t = vecs[1]; vecs[1] = vecs[2]; vecs[2] = t; }
    if (vecs[0].x > vecs[1].x) { vec3 t = vecs[0]; vecs[0] = vecs[1]; vecs[1] = t; }
}

bbox getbbox(tri t)
{
  vec3 va = t.v0;
  vec3 vb = t.v1;
  vec3 vc = t.v2;

  vec3 yvecs[3];
  vec3 xvecs[3];
  sortbyY(yvecs, t.v0, t.v1, t.v2);
  sortbyX(xvecs, t.v0, t.v1, t.v2);

  /*
   *    v2     v3
   *    +-------+
   *    #       #
   *    #       #
   *    #       #
   *    +-------+
   *    v0     v1
   */
  vec3 v0 = (vec3) { .x = xvecs[0].x, .y = yvecs[0].y };
  vec3 v1 = (vec3) { .x = xvecs[2].x, .y = yvecs[0].y };
  vec3 v2 = (vec3) { .x = xvecs[0].x, .y = yvecs[2].y };
  vec3 v3 = (vec3) { .x = xvecs[2].x, .y = yvecs[2].y };

  return (bbox){ v0, v1, v2, v3 };
}

