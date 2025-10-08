#include <math.h>
#include <stdlib.h>
#include "camera/camera_ops.h"

typedef struct {
  wf_vec3 forward, up, right;
  float   tan_half_fov_y;
  float   aspect_ratio;
} perspective_priv_t;

typedef struct {
  float fov_y_rad;
  float aspect_ratio;
} perspective_params_t;

static void* perspective_init(wf_vec3 pos, wf_vec3 target, wf_vec3 up,
                              const void* params) {
  const perspective_params_t* p = (const perspective_params_t*)params;
  perspective_priv_t*         priv =
      (perspective_priv_t*)calloc(1, sizeof(perspective_priv_t));
  if (!priv)
    return NULL;

  wf_vec3 dir   = v3_sub(target, pos);
  priv->forward = v3_normalize(dir);
  priv->up      = v3_normalize(up);
  priv->right   = v3_normalize(v3_cross(priv->forward, priv->up));
  priv->up      = v3_cross(priv->right, priv->forward); // re-orthogonalize

  priv->aspect_ratio   = p->aspect_ratio;
  priv->tan_half_fov_y = tanf(p->fov_y_rad * 0.5f);
  return priv;
}

static wf_vec3 perspective_get_ray_direction(const void* priv, float u,
                                             float v) {
  const perspective_priv_t* p = (const perspective_priv_t*)priv;
  float x = (2.0f * u - 1.0f) * p->aspect_ratio * p->tan_half_fov_y;
  float y = (1.0f - 2.0f * v) * p->tan_half_fov_y;

  wf_vec3 scaled_right = v3_scale(x, p->right);
  wf_vec3 scaled_up    = v3_scale(-y, p->up);
  wf_vec3 dir          = v3_add(v3_add(scaled_right, scaled_up), p->forward);
  return v3_normalize(dir);
}

static void perspective_exit(void* priv) {
  free(priv);
}

static struct camera_projection_ops perspective_ops = {
  .type              = PROJ_PERSPECTIVE,
  .name              = "perspective",
  .init              = perspective_init,
  .get_ray_direction = perspective_get_ray_direction,
  .exit              = perspective_exit,
};

CAMERA_PROJECTION_OPS_REGISTER(perspective_ops)

void register_perspective_projection(void) {
  camera_register_projection(&perspective_ops);
}
