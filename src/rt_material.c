// rt_material.c
#include "rt_material.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "log4c.h"

rt_material_t* rt_material_from_wf(const wf_material_t* wf_mat) {
  rt_material_t* mat = calloc(1, sizeof(rt_material_t));
  if (!mat)
    return NULL;

  mat->name    = wf_mat->name ? strdup(wf_mat->name) : NULL;
  mat->opacity = wf_mat->d;
  mat->ior     = wf_mat->Ni;

  // choose brdf model based on illum
  const char* brdf_name   = "lambert";
  void*       brdf_params = NULL;

  log_debug("illum is %d", wf_mat->illum);
  if (wf_mat->illum == 2 || wf_mat->illum == 3) {
    // Phong 或 Blinn-Phong → 映射到 Cook-Torrance（更物理）
    struct params_t {
      wf_vec3 albedo;
      float   roughness;
      float   metallic;
    };
    struct params_t* params = malloc(sizeof(struct params_t));
    // smaller roughness for bigger Ns
    params->metallic  = 0.0f;
    params->roughness = sqrtf(2.0f / (wf_mat->Ns + 2.0f));
    params->albedo    = wf_mat->Kd;
    brdf_name         = "cook_torrance";
    brdf_params       = params;
  } else {
    // defalut Lambert
    brdf_params = (void*)&wf_mat->Kd;
  }

  mat->brdf = brdf_create(brdf_name, brdf_params);
  if (!mat->brdf) {
    log_warn("fallback lambert brdf");
    mat->brdf = brdf_create("lambert", &wf_mat->Kd);
  }

  return mat;
}

void rt_material_destroy(rt_material_t* mat) {
  if (!mat)
    return;
  free(mat->name);
  brdf_destroy(mat->brdf);
  free(mat);
}
