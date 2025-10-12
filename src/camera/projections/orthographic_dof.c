// projections/orthographic_dof.c
#include <math.h>
#include <stdlib.h>
#include "camera/camera_ops.h"

// Reuse RNG utilities (you can move these to a shared utils file)
static inline uint32_t xorshift32(uint32_t* state) {
  uint32_t x = *state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return *state = x;
}

static inline float rand_float(uint32_t* state) {
  return (float)(xorshift32(state) & 0xFFFFFF) / (float)0x1000000;
}

static wf_vec3 random_in_unit_disk(uint32_t* state) {
  wf_vec3 p;
  do {
    p.x = 2.0f * rand_float(state) - 1.0f;
    p.y = 2.0f * rand_float(state) - 1.0f;
  } while (p.x * p.x + p.y * p.y >= 1.0f);
  p.z = 0.0f;
  return p;
}

// Private data for orthographic DoF camera
typedef struct {
  wf_vec3  position;
  wf_vec3  forward;
  wf_vec3  up;
  wf_vec3  right;
  float    half_width;
  float    half_height;
  float    aperture;
  float    focus_distance;
  uint32_t rng_state;
} ortho_dof_priv_t;

// Initialize orthographic DoF camera
static void* ortho_dof_init(wf_vec3 pos, wf_vec3 target, wf_vec3 up,
                            const void* params) {
  const ortho_dof_params_t* p = (const ortho_dof_params_t*)params;
  ortho_dof_priv_t*         priv =
      (ortho_dof_priv_t*)calloc(1, sizeof(ortho_dof_priv_t));
  if (!priv)
    return NULL;

  priv->position = pos;
  wf_vec3 dir    = v3_sub(target, pos);
  priv->forward  = v3_normalize(dir);
  priv->up       = v3_normalize(up);
  priv->right    = v3_normalize(v3_cross(priv->forward, priv->up));
  priv->up       = v3_cross(priv->right, priv->forward);

  priv->half_width     = p->width * 0.5f;
  priv->half_height    = p->height * 0.5f;
  priv->aperture       = p->aperture;
  priv->focus_distance = p->focus_distance;
  priv->rng_state      = 54321; // Different seed from perspective DoF

  return priv;
}

// Generate ray with orthographic DoF
static wf_vec3 ortho_dof_get_ray_direction(const void* priv, float u, float v) {
  ortho_dof_priv_t* p = (ortho_dof_priv_t*)priv;
  p->rng_state ^= (uint32_t)((u * 10000.0f) + (v * 1000.0f));

  // Compute ray origin offset on image plane (orthographic)
  float x = (2.0f * u - 1.0f) * p->half_width;
  float y = (1.0f - 2.0f * v) * p->half_height;

  wf_vec3 image_point = v3_add(v3_scale(x, p->right), v3_scale(-y, p->up));

  wf_vec3 ray_origin = p->position;

  // Apply DoF: sample lens and shift origin
  if (p->aperture > 0.0f) {
    wf_vec3 rd = random_in_unit_disk(&p->rng_state);
    wf_vec3 lens_offset =
        v3_scale(0.5f * p->aperture,
                 v3_add(v3_scale(rd.x, p->right), v3_scale(rd.y, p->up)));
    ray_origin = v3_add(p->position, lens_offset);
  }

  // In orthographic DoF, ray direction is always forward
  // The "focus" is simulated by shifting the origin, not the direction
  return p->forward;
}

static void ortho_dof_exit(void* priv) {
  free(priv);
}

static struct camera_projection_ops ortho_dof_ops = {
  .type              = PROJ_ORTHOGRAPHIC_DOF,
  .name              = "orthographic_dof",
  .init              = ortho_dof_init,
  .get_ray_direction = ortho_dof_get_ray_direction,
  .exit              = ortho_dof_exit,
};

CAMERA_PROJECTION_OPS_REGISTER(ortho_dof_ops)

void register_ortho_dof_projection(void) {
  camera_register_projection(&ortho_dof_ops);
}
