// brdf_lambert.c
#include <math.h>
#include <stdlib.h>
#include "algo.h"
#include "brdf/brdf.h"

typedef struct {
  wf_vec3 albedo;
} lambert_data_t;

// === Eval ===
static wf_vec3 lambert_eval(const brdf_t* self, const wf_vec3* wi,
                            const wf_vec3* wo, const wf_vec3* n) {
  (void)wo;
  lambert_data_t* data   = (lambert_data_t*)self->data;
  float           ndotwi = v3_dot(*n, *wi);
  return (ndotwi > 0) ? (wf_vec3){ data->albedo.x / M_PI, data->albedo.y / M_PI,
                                   data->albedo.z / M_PI }
                      : (wf_vec3){ 0, 0, 0 };
}

// === Sample: Cosine-weighted hemisphere sampling ===
static wf_vec3 lambert_sample(const brdf_t* self, const wf_vec3* wo,
                              const wf_vec3* n, float u1, float u2,
                              wf_vec3* wi_out, float* pdf_out) {
  (void)wo;
  lambert_data_t* data = (lambert_data_t*)self->data;

  wf_vec3 up =
      fabsf(n->z) < 0.999f ? (wf_vec3){ 0, 0, 1 } : (wf_vec3){ 1, 0, 0 };
  wf_vec3 tangent   = v3_normalize((wf_vec3){ n->y * up.z - n->z * up.y,
                                              n->z * up.x - n->x * up.z,
                                              n->x * up.y - n->y * up.x });
  wf_vec3 bitangent = v3_normalize((wf_vec3){
      n->y * tangent.z - n->z * tangent.y, n->z * tangent.x - n->x * tangent.z,
      n->x * tangent.y - n->y * tangent.x });

  // Cosine-weighted sampling on hemisphere
  float r     = sqrtf(u1);
  float theta = 2.0f * M_PI * u2;
  float x     = r * cosf(theta);
  float y     = r * sinf(theta);
  float z     = sqrtf(fmaxf(0.0f, 1.0f - u1));

  wi_out->x = x * tangent.x + y * bitangent.x + z * n->x;
  wi_out->y = x * tangent.y + y * bitangent.y + z * n->y;
  wi_out->z = x * tangent.z + y * bitangent.z + z * n->z;

  // PDF = cosθ / π
  *pdf_out = z / M_PI;

  // return f_r = albedo / π
  return (wf_vec3){ data->albedo.x / M_PI, data->albedo.y / M_PI,
                    data->albedo.z / M_PI };
}

// === Lifecycle ===
static void* lambert_create(const void* params) {
  const wf_vec3*  albedo = (const wf_vec3*)params;
  lambert_data_t* data   = malloc(sizeof(lambert_data_t));
  data->albedo           = *albedo;
  return data;
}

static void lambert_destroy(void* data) {
  free(data);
}

// === Ops Definition ===
static brdf_ops_t lambert_ops = { .name    = "lambert",
                                  .eval    = lambert_eval,
                                  .sample  = lambert_sample,
                                  .create  = lambert_create,
                                  .destroy = lambert_destroy };

// === Auto-register (constructor) ===
__attribute__((constructor)) static void register_lambert(void) {
  brdf_register_model(&lambert_ops);
}
