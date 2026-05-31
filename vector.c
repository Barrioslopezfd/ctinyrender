#include <stdint.h>

typedef struct { int32_t x; int32_t y;} vec2;
typedef struct { int32_t x; int32_t y; int32_t z; } vec3;
typedef struct { int32_t x; int32_t y; int32_t z; int32_t w; } vec4;


vec2 vec2_set(int32_t x, int32_t y, int32_t z){
  return (vec2){ .x = x, .y = y };
}

vec2 vec2_add(vec2 vecA, vec2 vecB) {
  return (vec2){ .x = vecA.x+vecB.x, .y = vecA.y+vecB.y };
}

vec2 vec2_rest(vec2 vecA, vec2 vecB) {
  return (vec2){ .x = vecA.x-vecB.x, .y = vecA.y-vecB.y };
}

vec2 vec2_mult(vec2 vecA, vec2 vecB) {
  int32_t x = vecA.x * vecB.x;
  int32_t y = vecA.y * vecB.y;
  return (vec2){ .x = x, .y = y };
}

vec3 vec3_set(int32_t x, int32_t y, int32_t z){
  return (vec3){ .x = x, .y = y, .z = z };
}

vec3 vec3_add(vec3 vecA, vec3 vecB) {
  return (vec3){ .x = vecA.x+vecB.x, .y = vecA.y+vecB.y, .z = vecA.z+vecB.z };
}

vec3 vec3_rest(vec3 vecA, vec3 vecB) {
  return (vec3){ .x = vecA.x-vecB.x, .y = vecA.y-vecB.y, .z = vecA.z-vecB.z };
}

vec3 vec3_mult(vec3 vecA, vec3 vecB) {
  int32_t x = vecA.y * vecB.z - vecA.z * vecB.y;
  int32_t y = vecA.z * vecB.x - vecA.x * vecB.z;
  int32_t z = vecA.x * vecB.y - vecA.y * vecB.x;
  return (vec3){ .x = x, .y = y, .z = z };
}

vec4 vec4_set(int32_t x, int32_t y, int32_t z, int32_t w){
  return (vec4){ .x = x, .y = y, .z = z, .w = w };
}

vec4 vec4_add(vec4 vecA, vec4 vecB) {
  return (vec4){ .x = vecA.x+vecB.x, .y = vecA.y+vecB.y, .z = vecA.z+vecB.z, .w = vecA.w+vecB.w };
}

vec4 vec4_rest(vec4 vecA, vec4 vecB) {
  return (vec4){ .x = vecA.x-vecB.x, .y = vecA.y-vecB.y, .z = vecA.z-vecB.z, .w = vecA.w-vecB.w};
}

vec4 vec4_mult(vec4 vecA, vec4 vecB) {
  int32_t x = vecA.y * vecB.z - vecA.z * vecB.y;
  int32_t y = vecA.z * vecB.x - vecA.x * vecB.z;
  int32_t z = vecA.x * vecB.y - vecA.y * vecB.x;
  return (vec4){ .x = x, .y = y, .z = z };
}
