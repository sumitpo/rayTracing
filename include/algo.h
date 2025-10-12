#ifndef ALGO_H
#define ALGO_H

#include <math.h> // for sqrtf, fabsf
#include "config.h"
#include "raytracer.h"
#include "wavefront.h"

#include <stdbool.h>

// vector calculation

static inline wf_vec3 v3_add(wf_vec3 a, wf_vec3 b) {
  return (wf_vec3){ .x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z };
}

static inline wf_vec3 v3_sub(wf_vec3 a, wf_vec3 b) {
  return (wf_vec3){ .x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z };
}

static inline wf_vec3 v3_scale(float k, wf_vec3 v) {
  return (wf_vec3){ .x = k * v.x, .y = k * v.y, .z = k * v.z };
}

static inline float v3_dot(wf_vec3 a, wf_vec3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

wf_vec3 v3_cross(wf_vec3 a, wf_vec3 b);

static inline float v3_length_sq(wf_vec3 v) {
  return v3_dot(v, v);
}

static inline float v3_length(wf_vec3 v) {
  return sqrtf(v3_length_sq(v));
}

wf_vec3 v3_normalize(wf_vec3 v);

static inline wf_vec3 v3_zero(void) {
  return (wf_vec3){ .x = 0.0f, .y = 0.0f, .z = 0.0f };
}

static inline wf_vec3 v3_add_scalar(wf_vec3 v, float k) {
  return (wf_vec3){ .x = v.x + k, .y = v.y + k, .z = v.z + k };
}

static inline wf_vec3 v3_sub_scalar(wf_vec3 v, float k) {
  return (wf_vec3){ .x = v.x - k, .y = v.y - k, .z = v.z - k };
}

static inline wf_vec3 v3_div_scalar(wf_vec3 v, float k) {
  if (k == 0.0f)
    return v3_zero();
  return (wf_vec3){ .x = v.x / k, .y = v.y / k, .z = v.z / k };
}

static inline int v3_is_zero(wf_vec3 v, float epsilon) {
  return (fabsf(v.x) < epsilon) && (fabsf(v.y) < epsilon)
         && (fabsf(v.z) < epsilon);
}

static inline wf_vec3 ray_at(ray_t r, float t) {
  return v3_add(r.origin, v3_scale(t, r.direction));
}

bool ray_intersects_triangle(const ray_t* ray, const wf_vec3* v0,
                             const wf_vec3* v1, const wf_vec3* v2,
                             float* t_out);
#endif // ALGO_H
