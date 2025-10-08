#include <math.h>
#include <stdlib.h>
#include "camera/camera_ops.h"

typedef struct {
  wf_vec3 forward, up, right;
} spherical_priv_t;

// spherical camera doesn't need extra parameter
static void* spherical_init(wf_vec3 pos, wf_vec3 target, wf_vec3 up,
                            const void* params) {
  spherical_priv_t* priv = calloc(1, sizeof(spherical_priv_t));
  if (!priv)
    return NULL;
  wf_vec3 dir   = v3_sub(target, pos);
  priv->forward = v3_normalize(dir);
  priv->up      = v3_normalize(up);
  priv->right   = v3_normalize(v3_cross(priv->forward, priv->up));
  priv->up      = v3_cross(priv->right, priv->forward);
  return priv;
}

static wf_vec3 spherical_get_ray_direction(const void* priv, float u, float v) {
  const spherical_priv_t* p = (const spherical_priv_t*)priv;

  // φ: azimuth (0 to 2π), θ: polar (0 to π)
  float phi   = 2.0f * M_PI * u;
  float theta = M_PI * v;

  float   sin_theta = sinf(theta);
  wf_vec3 local_dir = { .x = sin_theta * cosf(phi),
                        .y = sin_theta * sinf(phi),
                        .z = cosf(theta) };

  wf_vec3 world_dir = v3_add(v3_scale(local_dir.x, p->right),
                             v3_add(v3_scale(local_dir.y, p->up),
                                    v3_scale(local_dir.z, p->forward)));
  return v3_normalize(world_dir);
}

static void spherical_exit(void* priv) {
  free(priv);
}

static struct camera_projection_ops spherical_ops = {
  .type              = PROJ_SPHERICAL,
  .name              = "spherical",
  .init              = spherical_init,
  .get_ray_direction = spherical_get_ray_direction,
  .exit              = spherical_exit,
};

CAMERA_PROJECTION_OPS_REGISTER(spherical_ops)

void register_spherical_projection(void) {
  camera_register_projection(&spherical_ops);
}
