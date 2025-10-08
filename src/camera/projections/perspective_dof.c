// projections/perspective_dof.c
#include <math.h>
#include <stdlib.h>
#include "camera/camera_ops.h"

// Lightweight pseudo-random number generator (Xorshift32)
static inline uint32_t xorshift32(uint32_t* state) {
  uint32_t x = *state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return *state = x;
}

// Generate random float in [0, 1)
static inline float rand_float(uint32_t* state) {
  return (float)(xorshift32(state) & 0xFFFFFF) / (float)0x1000000;
}

// Uniform random point inside unit disk (rejection sampling)
static wf_vec3 random_in_unit_disk(uint32_t* state) {
  wf_vec3 p;
  do {
    p.x = 2.0f * rand_float(state) - 1.0f;
    p.y = 2.0f * rand_float(state) - 1.0f;
  } while (p.x * p.x + p.y * p.y >= 1.0f);
  p.z = 0.0f;
  return p;
}

// Private data for DoF camera
typedef struct {
  wf_vec3  position;
  wf_vec3  forward;
  wf_vec3  up;
  wf_vec3  right;
  float    tan_half_fov_y;
  float    aspect_ratio;
  float    aperture;
  float    focus_distance;
  uint32_t rng_state; // Per-camera RNG state
} dof_priv_t;

// Initialize DoF camera from parameters
static void* dof_init(wf_vec3 pos, wf_vec3 target, wf_vec3 up,
                      const void* params) {
  const dof_params_t* p    = (const dof_params_t*)params;
  dof_priv_t*         priv = (dof_priv_t*)calloc(1, sizeof(dof_priv_t));
  if (!priv)
    return NULL;

  priv->position = pos;
  wf_vec3 dir    = v3_sub(target, pos);
  priv->forward  = v3_normalize(dir);
  priv->up       = v3_normalize(up);
  priv->right    = v3_normalize(v3_cross(priv->forward, priv->up));
  priv->up       = v3_cross(priv->right, priv->forward); // Re-orthogonalize

  priv->aspect_ratio   = p->aspect_ratio;
  priv->tan_half_fov_y = tanf(p->fov_y_rad * 0.5f);
  priv->aperture       = p->aperture;
  priv->focus_distance = p->focus_distance;
  priv->rng_state      = 12345; // Initial seed

  return priv;
}

// Generate ray direction with depth of field effect
static wf_vec3 dof_get_ray_direction(const void* priv, float u, float v) {
  dof_priv_t* p = (dof_priv_t*)priv;

  // Perturb RNG state based on pixel coordinates for variance
  p->rng_state ^= (uint32_t)((u * 10000.0f) + (v * 1000.0f));

  // Compute ray direction through image plane at focal distance
  float x = (2.0f * u - 1.0f) * p->aspect_ratio * p->tan_half_fov_y;
  float y = (1.0f - 2.0f * v) * p->tan_half_fov_y;

  wf_vec3 ray_dir_at_focus =
      v3_add(v3_scale(x, p->right),
             v3_add(v3_scale(-y, p->up),
                    v3_scale(p->focus_distance, p->forward)));

  wf_vec3 ray_origin = p->position;

  // If aperture > 0, sample a random point on the lens
  if (p->aperture > 0.0f) {
    wf_vec3 rd = random_in_unit_disk(&p->rng_state);
    wf_vec3 offset =
        v3_scale(0.5f * p->aperture,
                 v3_add(v3_scale(rd.x, p->right), v3_scale(rd.y, p->up)));
    ray_origin = v3_add(p->position, offset);
  }

  // Final ray direction: from sampled origin to point on focal plane
  wf_vec3 direction = v3_sub(ray_dir_at_focus, v3_sub(ray_origin, p->position));
  return v3_normalize(direction);
}

// Cleanup private data
static void dof_exit(void* priv) {
  free(priv);
}

// Projection operations for DoF camera
static struct camera_projection_ops dof_ops = {
  .type              = PROJ_PERSPECTIVE_DOF,
  .name              = "perspective_dof",
  .init              = dof_init,
  .get_ray_direction = dof_get_ray_direction,
  .exit              = dof_exit,
};

// Auto-register this projection at load time
CAMERA_PROJECTION_OPS_REGISTER(dof_ops)

void register_dof_projection(void) {
  camera_register_projection(&dof_ops);
}
