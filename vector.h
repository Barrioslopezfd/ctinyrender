#include <stdint.h>

typedef struct { int32_t x; int32_t y;} vec2;
typedef struct { int32_t x; int32_t y; int32_t z; } vec3;
typedef struct { int32_t x; int32_t y; int32_t z; int32_t w; } vec4;

vec2 vec2_set(int32_t, int32_t, int32_t);
vec2 vec2_add(vec2, vec2);
vec2 vec2_rest(vec2, vec2);
vec2 vec2_mult(vec2, vec2);

vec3 vec3_set(int32_t, int32_t, int32_t);
vec3 vec3_add(vec3, vec3);
vec3 vec3_rest(vec3, vec3);
vec3 vec3_mult(vec3, vec3);

vec4 vec4_set(int32_t, int32_t, int32_t, int32_t);
vec4 vec4_add(vec4, vec4);
vec4 vec4_rest(vec4, vec4);
vec4 vec4_mult(vec4, vec4);
