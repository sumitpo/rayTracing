#include <stdlib.h>
#include "camera/camera_ops.h"

typedef struct {
  wf_vec3 forward, up, right;
  float   half_width;
  float   half_height;
} ortho_priv_t;

typedef struct {
  float width;
  float height;
} ortho_params_t;

static void* ortho_init(wf_vec3 pos, wf_vec3 target, wf_vec3 up,
                        const void* params) {
  const ortho_params_t* p    = (const ortho_params_t*)params;
  ortho_priv_t*         priv = (ortho_priv_t*)calloc(1, sizeof(ortho_priv_t));
  if (!priv)
    return NULL;

  wf_vec3 dir   = v3_sub(target, pos);
  priv->forward = v3_normalize(dir);
  priv->up      = v3_normalize(up);
  priv->right   = v3_normalize(v3_cross(priv->forward, priv->up));
  priv->up      = v3_cross(priv->right, priv->forward);

  priv->half_width  = p->width * 0.5f;
  priv->half_height = p->height * 0.5f;
  return priv;
}

// Orthographic camera: ray direction is always forward,
// offset is handled at the origin (this function returns direction only)
static wf_vec3 ortho_get_ray_direction(const void* priv, float u, float v) {
  const ortho_priv_t* p = (const ortho_priv_t*)priv;
  return p->forward; // Direction remains constant
}

static void ortho_exit(void* priv) {
  free(priv);
}

static struct camera_projection_ops ortho_ops = {
  .type              = PROJ_ORTHOGRAPHIC,
  .name              = "orthographic",
  .init              = ortho_init,
  .get_ray_direction = ortho_get_ray_direction,
  .exit              = ortho_exit,
};

CAMERA_PROJECTION_OPS_REGISTER(ortho_ops)

void register_ortho_projection(void) {
  camera_register_projection(&ortho_ops);
}
