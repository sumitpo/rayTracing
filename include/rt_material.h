#ifndef RT_MATERIAL_H
#define RT_MATERIAL_H

#include "brdf/brdf.h"
#include "wavefront.h"

typedef struct {
  char*   name;
  brdf_t* brdf;
  float   opacity;
  float   ior;
} rt_material_t;

rt_material_t* rt_material_from_wf(const wf_material_t* wf_mat);
void           rt_material_destroy(rt_material_t* mat);

#endif
