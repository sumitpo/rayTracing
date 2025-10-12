// rt_types.h
#ifndef RT_TYPES_H
#define RT_TYPES_H

#include "wavefront.h"

// typedef struct {
//   wf_face face;
//   size_t  material_idx; // for scene->materials[material_idx]
// } rt_triangle_t;

typedef struct {
  bool    hit;
  float   t;
  wf_vec3 point;
  wf_vec3 normal;
  size_t  material_idx;
} hit_record_t;

#endif
