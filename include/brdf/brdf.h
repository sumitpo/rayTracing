// brdf.h
#ifndef BRDF_H
#define BRDF_H

#include <stdbool.h>
#include "wavefront.h"

typedef struct brdf_s brdf_t;

// BRDF ops
typedef struct brdf_ops {
  const char* name; // "lambert", "cook_torrance", etc.

  // eval function：given wi, wo, n → return f_r(wi, wo)
  wf_vec3 (*eval)(const brdf_t* self, const wf_vec3* wi, const wf_vec3* wo,
                  const wf_vec3* n);

  // sample function：given wo, n, random number → generate wi and return f_r /
  // pdf
  wf_vec3 (*sample)(const brdf_t* self, const wf_vec3* wo, const wf_vec3* n,
                    float u1, float u2, wf_vec3* wi_out, float* pdf_out);

  // create/destroy private data
  void* (*create)(const void* params);
  void (*destroy)(void* data);
} brdf_ops_t;

// BRDF instance
struct brdf_s {
  const brdf_ops_t* ops;
  void*             data; // point to specific model data（like lambert_data_t）
};

// register/find
int               brdf_register_model(const brdf_ops_t* ops);
const brdf_ops_t* brdf_find_model(const char* name);

brdf_t* brdf_create(const char* model_name, const void* params);
void    brdf_destroy(brdf_t* brdf);

#endif
