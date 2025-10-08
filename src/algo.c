#include "algo.h"
#include <math.h>

// --- 以下函数改为值传递 ---

wf_vec3 v3_cross(wf_vec3 a, wf_vec3 b) {
  return (wf_vec3){ .x = a.y * b.z - a.z * b.y,
                    .y = a.z * b.x - a.x * b.z,
                    .z = a.x * b.y - a.y * b.x };
}

// 注意：v3_length_sq 和 v3_length 已在 algo.h 中 inline，此处无需定义

wf_vec3 v3_normalize(wf_vec3 v) {
  float len = v3_length(v);
  if (len == 0.0f) {
    return v3_zero();
  }
  return v3_scale(1.0f / len, v);
}

// Ray-triangle intersection (Möller–Trumbore)
bool ray_intersects_triangle(const ray_t* ray, const wf_vec3* v0,
                             const wf_vec3* v1, const wf_vec3* v2,
                             float* t_out) {
  const float EPSILON = 1e-8f;
  wf_vec3     edge1   = v3_sub(*v1, *v0);
  wf_vec3     edge2   = v3_sub(*v2, *v0);
  wf_vec3     h       = v3_cross(ray->direction, edge2);
  float       a       = v3_dot(edge1, h);
  if (a > -EPSILON && a < EPSILON)
    return false;

  float   f = 1.0f / a;
  wf_vec3 s = v3_sub(ray->origin, *v0);
  float   u = f * v3_dot(s, h);
  if (u < 0.0f || u > 1.0f)
    return false;

  wf_vec3 q = v3_cross(s, edge1);
  float   v = f * v3_dot(ray->direction, q);
  if (v < 0.0f || u + v > 1.0f)
    return false;

  float t = f * v3_dot(edge2, q);
  if (t > EPSILON) {
    *t_out = t;
    return true;
  }
  return false;
}
