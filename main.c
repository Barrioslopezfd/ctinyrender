#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <omp.h>
#include <sys/mman.h>
#include <limits.h>
#include "util.h"
#include "math.h"
#include "obj.h"
#include "vector.h"

#define RED     0x00FF0000
#define GREEN   0x0000FF00
#define BLUE    0x000000FF
#define YELLOW  0x00FFDE21
#define WHITE   0x00FFFFFF
#define BLACK   0x00000000
#define M_PI 3.14159265358979323846

// I want 80% of the area to draw, I use 20% cz it lets me get 
// the upper left vertex with bresenham
#define INTERNAL_AREA_RATIO 0.2f

#define WIDTH         1000
#define HEIGHT        1000
#define SCREEN_WIDTH  1000
#define SCREEN_HEIGHT 1000
#define BORDER_SIZE   2
#define NBUFFER       2

typedef uint32_t BMColor;
typedef struct { vec3 v0; vec3 v1; vec3 v2; } tri;
typedef struct { vec3 v0; vec3 v1; vec3 v2; vec3 v3; } bbox;
typedef struct { float x; float y; float z; } vec3f;

typedef struct { int32_t *pixel_buffer; int32_t width; int32_t height; } MPixmap;
typedef struct { int32_t *Buffer; size_t size; } ZBuffer;

void linelow(vec3 v0, vec3 v1, MPixmap *pix, BMColor color);
void linehigh(vec3 v0, vec3 v1, MPixmap *pix, BMColor color); 
void line(vec3 v0, vec3 v1, MPixmap *pix, BMColor color); 
void triangle(tri t, bbox bbox, MPixmap* pix, ZBuffer ZBuff, BMColor color);
void modelrender(OBJModel model, MPixmap* pix);
void sortbyY(vec3 *vecs, vec3 v0, vec3 v1, vec3 v2);
void sortbyX(vec3 *vecs, vec3 v0, vec3 v1, vec3 v2);
bbox getbbox(tri t);
int32_t getarea(tri t);
void SetPixel(MPixmap *pix, int x, int y, uint32_t color);
ZBuffer ZBSet(int width, int height);
void FillSquare(MPixmap *pix, vec2 upper_left_vertex, vec2 lower_right_vertex, uint32_t color);
bbox model_getbbox(OBJModel *model, MPixmap *pix);
void vec3f_rot(vec3f *vec);
void vec3f_perspec(vec3f *vec);
void vec3f_proj(vec3 *vec, vec3f *pre_vec, MPixmap *pix);
void vec3f_print(vec3f *vec);
void vec3_print(vec3 *vec);

int main(int argc, char *argv[])
{

  Display *display = XOpenDisplay(NULL);
  if (display == NULL) {
    printf("Cannot open display\n");
    exit(1);
  }


  Window root = XDefaultRootWindow(display);
  int screen = DefaultScreen(display);
  XSetWindowAttributes attr;
  attr.background_pixmap  = 0L;
  attr.event_mask         = KeyPressMask | ExposureMask;

  // Display*		/* display */,
  // Window		/* parent */,
  // int			/* x */,
  // int			/* y */,
  // unsigned int	/* width */,
  // unsigned int	/* height */,
  // unsigned int	/* border_width */,
  // int			/* depth */,
  // unsigned int	/* class */,
  // Visual*		/* visual */,
  // unsigned long	/* valuemask */,
  // XSetWindowAttributes*	/* attributes */

  Window window = XCreateWindow(display, root, 0, 0,
                                SCREEN_WIDTH, SCREEN_HEIGHT, BORDER_SIZE,
                                DefaultDepth(display, screen), InputOutput, 
                                DefaultVisual(display, screen), CWEventMask, &attr);

  Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &wm_delete_window, 1);

  GC graphic_context = XCreateGC(display, window, 0, NULL);
  XSetForeground(display, graphic_context, WHITE);

  XMapWindow(display, window);

  MPixmap* pix = &(MPixmap){
    .pixel_buffer = mmap(NULL, SCREEN_WIDTH * SCREEN_HEIGHT * 4, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0),
    .width = SCREEN_WIDTH,
    .height = SCREEN_HEIGHT
  };

  if (pix->pixel_buffer == MAP_FAILED) {
    perror("mmap failed for zbuffer");
    exit(1);
  };

  XImage *image = XCreateImage(display, 
                               DefaultVisual(display, screen),
                               24,           
                               ZPixmap,      
                               0,            
                               (char *)pix->pixel_buffer, 
                               SCREEN_WIDTH, SCREEN_HEIGHT, 
                               32,           
                               0);          

  // This is for the internal area where i will draw the models
  vec2 v1 = (vec2) { 
    .x = SCREEN_WIDTH * INTERNAL_AREA_RATIO,
    .y = rondo((SCREEN_WIDTH / (float)SCREEN_HEIGHT) * (SCREEN_WIDTH * INTERNAL_AREA_RATIO)),
  };
  vec2 v2 = (vec2) {
    .x = SCREEN_WIDTH,
    .y = SCREEN_HEIGHT,
  };
  // FillSquare(pix, v1, v2, RED);

  int quit = 0;
  KeySym keysym;
  OBJModel model = modelget("obj/african_head/african_head.obj");
  // for testing, i just comment diablio when i want african.
  model = modelget("obj/diablo3_pose/diablo3_pose.obj");
  // model = modelget("obj/A.obj");
  // model_getbbox(&model, pix); // not using rn
  bbox bb;
  static char key_return[32];
  int a = 1;
  modelrender(model, pix);
  XPutImage(display, window, graphic_context, image, 0, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  while(!quit) {
    XQueryKeymap(display, key_return);
    while (XPending(display)) {
      XEvent event;
      XNextEvent(display, &event);
      switch (event.type) {
        case ClientMessage: {
           if ( (Atom)event.xclient.data.l[0] == wm_delete_window )
             quit = 1;
        } break;

        case KeyPress: {
          keysym = XLookupKeysym(&event.xkey , 0);
          if (keysym == XK_Escape){
            quit = 1;
          }
        } break;
        case Expose: {
          modelrender(model, pix);
          XPutImage(display, window, graphic_context, image, 0, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        }
      }
    }
    usleep(16666);
  }

  XCloseDisplay(display);
  return 0;
}

void linelow(vec3 v0, vec3 v1, MPixmap *pix, BMColor color) 
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
    SetPixel(pix, x, y, color);
    if (D > 0) {
      y = y + yi;
      D = D + (2 * (dy - dx));
    } else {
      D = D + 2*dy;
    }
  }
}

void linehigh(vec3 v0, vec3 v1, MPixmap *pix, BMColor color) 
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
    SetPixel(pix, x, y, color);
    if (D > 0) {
      x = x + xi;
      D = D + (2 * (dx - dy));
    } else {
      D = D + 2*dx;
    }
  }
}

void line(vec3 v0, vec3 v1, MPixmap *pix, BMColor color) 
{
  int ay = absol(v1.y - v0.y);
  int ax = absol(v1.x - v0.x);
  if (ay < ax) {
    if (v0.x > v1.x)
      linelow(v1, v0, pix, color);
    else
      linelow(v0, v1, pix, color);
  } else {
    if (v0.y > v1.y)
      linehigh(v1, v0, pix, color);
    else
      linehigh(v0, v1, pix, color);
  }
}

void triangle(tri t, bbox bbox, MPixmap* pix, ZBuffer ZBuff, BMColor color)
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

  // TODO: clear memory at the start of each call
  // currently it breaks the mouth triangles, not sure why.
  // memset(ZBuff.Buffer, 0, ZBuff.size);
  int32_t sarea = getarea(t);
  if (sarea < 1) return;
  float tarea = 1/(float)sarea;
  float cx = (t.v0.x + t.v1.x + t.v2.x) / 3.0f;
  float cy = (t.v0.y + t.v1.y + t.v2.y) / 3.0f;

  for (int32_t y = bbox.v0.y; y <= bbox.v2.y; y++)
  {
    for (int32_t x = bbox.v0.x; x <= bbox.v1.x; x++)
    {
      tri t0 = (tri){ .v0 = (vec3){ .x = x, .y = y }, .v1 = t.v1, .v2 = t.v2 };
      tri t1 = (tri){ .v0 = t.v0, .v1 = (vec3){ .x = x, .y = y }, .v2 = t.v2 };
      tri t2 = (tri){ .v0 = t.v0, .v1 = t.v1, .v2 = (vec3){ .x = x, .y = y } };
      float alpha = getarea(t0) * tarea;
      float beta  = getarea(t1) * tarea;
      float gamma = getarea(t2) * tarea;

      if (alpha < 0   || beta < 0   || gamma < 0) continue;
      int32_t z = color;
      if (color == BLACK) {
        uint8_t intensity = (uint8_t)(alpha*t.v0.z  + beta*t.v1.z   + gamma*t.v2.z);
        z = (intensity << 16) | (intensity << 8) | intensity;
      };
      int32_t depth = (uint32_t)(alpha*t.v0.z  + beta*t.v1.z   + gamma*t.v2.z);
      int32_t screen_index = y * pix->width + x;
      if (screen_index < pix->width * pix->height && screen_index > 0 ) {
      // if (screen_index > 0 ) {
        if ( depth > ZBuff.Buffer[screen_index]) {
          ZBuff.Buffer[screen_index] = depth;
          SetPixel(pix, x, y, z);
        };
      }
    };
  };
}

int32_t getarea(tri t){
  vec3 V01 = (vec3){ .x = t.v1.x - t.v0.x, .y = t.v1.y - t.v0.y };
  vec3 V02 = (vec3){ .x = t.v2.x - t.v0.x, .y = t.v2.y - t.v0.y };

  return V01.x * V02.y - V01.y * V02.x;
}

void modelrender(OBJModel model, MPixmap* pix)
{
  ZBuffer ZBuff = ZBSet(SCREEN_WIDTH, SCREEN_HEIGHT);
  for (int i = 0; i < model.nface; i+=3) {
    vec3f pre_v0 = (vec3f){ .x = model.vertices[(model.faces[i] - 1) * 3],
                            .y = model.vertices[(model.faces[i] - 1) * 3 + 1],
                            .z = model.vertices[(model.faces[i] - 1) * 3 + 2], };
    vec3f pre_v1 = (vec3f){ .x = model.vertices[(model.faces[i+1] - 1) * 3],
                            .y = model.vertices[(model.faces[i+1] - 1) * 3 + 1],
                            .z = model.vertices[(model.faces[i+1] - 1) * 3 + 2], };
    vec3f pre_v2 = (vec3f){ .x = model.vertices[(model.faces[i+2] - 1) * 3],
                            .y = model.vertices[(model.faces[i+2] - 1) * 3 + 1],
                            .z = model.vertices[(model.faces[i+2] - 1) * 3 + 2], };

    vec3f_rot(&pre_v0);
    vec3f_rot(&pre_v1);
    vec3f_rot(&pre_v2);

    vec3f_perspec(&pre_v0);
    vec3f_perspec(&pre_v1);
    vec3f_perspec(&pre_v2);

    vec3 v0;
    vec3 v1;
    vec3 v2;
    vec3f_proj(&v0, &pre_v0, pix);
    vec3f_proj(&v1, &pre_v1, pix);
    vec3f_proj(&v2, &pre_v2, pix);

    tri t   = { .v0 = v0, .v1 = v1, .v2 = v2, };
    bbox bb = getbbox(t);
    uint32_t color = (rand()%255 << 16) | (rand()%255 << 8) | (rand()%255);
    triangle(t, bb, pix, ZBuff, BLACK); // color or BLACK
  }
  munmap(ZBuff.Buffer, ZBuff.size);
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

void SetPixel(MPixmap *pix, int x, int y, uint32_t color)
{
  y += 1;
  if (x >= 0 && x < pix->width && y >= 0 && y < pix->height) {
    // pix->pixel_buffer[(pix->height-y) * pix->width + x] = color;
    pix->pixel_buffer[(pix->height - y - 1) * pix->width + x] = color;
  }
}

ZBuffer ZBSet(int width, int height)
{
    size_t size = width * height * 4;
    ZBuffer ZBuff = (ZBuffer) { 
      .Buffer = mmap(
        NULL, size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1, 0),
      .size = size,
    };

    if (ZBuff.Buffer == MAP_FAILED) {
        perror("mmap failed for zbuffer");
        exit(1);
    }

    int total_pixels = width * height;
    for (int i = 0; i < total_pixels; i++) {
        ZBuff.Buffer[i] = 0;
    }

    return ZBuff;
}

void FillSquare(MPixmap *pix, vec2 upper_left_vertex, vec2 lower_right_vertex, uint32_t color)
{
  for (int x = upper_left_vertex.x; x <= lower_right_vertex.x; x++)
    for (int y = upper_left_vertex.y; y <= lower_right_vertex.y; y++)
      SetPixel(pix, x, y, color);
}


bbox model_getbbox(OBJModel *model, MPixmap *pix)
{
  float smx = 3.f;
  float bgx = -1.f;
  float smy = 3.f;
  float bgy = -1.f;

  for (int i = 0; i < model->nvertex; i+=3) {
    float vertx = model->vertices[i];
    if (vertx < smx )
      smx = vertx;
    if (vertx > bgx)
      bgx = vertx;

    float verty = model->vertices[i+1];
    if (verty < smy )
      smy = verty;
    if (verty > bgy)
      bgy = verty;
  } 

  // printf("smx: %f\n", smx);
  // printf("bgx: %f\n", bgx);
  // printf("smy: %f\n", smy);
  // printf("bgy: %f\n", bgy);
  /*
   *    v2     v3
   *    +-------+
   *    #       #
   *    #       #
   *    #       #
   *    +-------+
   *    v0     v1
   */
  vec3 v0 = (vec3) { .x = (int32_t)(smx * pix->width), .y = (int32_t)(smy * pix->height) };
  vec3 v1 = (vec3) { .x = (int32_t)(bgx * pix->width), .y = (int32_t)(smy * pix->height) };
  vec3 v2 = (vec3) { .x = (int32_t)(smx * pix->width), .y = (int32_t)(bgy * pix->height) };
  vec3 v3 = (vec3) { .x = (int32_t)(bgx * pix->width), .y = (int32_t)(bgy * pix->height) };

  return (bbox){ v0, v1, v2, v3 };
}

// void vec3f_rot(vec3f *vec)
// {
//   // float angle = 30.f * (M_PI / 180.f);
//   // float x = vec->x;
//   // vec->x = x * cosf(angle) + vec->z * sinf(angle);
//   // vec->z = -x * sinf(angle) + vec->z * cosf(angle);
//
//   float y = vec->y;
//   float angleY = 30.f * (M_PI / 180.f);
//   vec->y = y * cosf(angleY) - vec->z * sinf(angleY);
//   vec->z = y * sinf(angleY) + vec->z * cosf(angleY);
// }

void vec3f_rot(vec3f *vec)
{
  float y = vec->y;
  vec->y = y * 1 + vec->z * 1;
  vec->z = y * 0 + vec->z * 1;
}

void vec3f_perspec(vec3f *vec)
{
  float w = 1.f - (vec->z / 3.f);

  vec->x = vec->x / w;
  vec->y = vec->y / w;
  // vec->z = vec->z / w;
}

void vec3f_proj(vec3 *vec, vec3f *pre_vec, MPixmap *pix)
{
  float scale = .7f;
  vec->x = rondo((float)pix->width/2  * (pre_vec->x * scale + 1.f));
  vec->y = rondo((float)pix->height/2 * (pre_vec->y * scale + 1.f));
  vec->z = rondo((float)255.f/2       * (pre_vec->z + 1.f));
}

void vec3f_print(vec3f *vec)  { static int n = 1; printf("x: %f - y: %f - z: %f - n: %d\n", vec->x, vec->y, vec->z, n); n++; }
void vec3_print(vec3 *vec)    { static int n = 1; printf("x: %d - y: %d - z: %d - n: %d\n", vec->x, vec->y, vec->z, n); n++; }
