#include "camera/camera.h"
#include <stdlib.h>
#include <string.h>
#include "camera/camera_ops.h"
#include "log4c.h"

void camera_register_projection(const struct camera_projection_ops* ops) {
  log_info("register proj [%s]", ops->name);
  s_projections[ops->type] = ops;
}

struct camera {
  wf_vec3                             position;
  void*                               priv;
  const struct camera_projection_ops* ops;
};

void camera_init_builtin_projections(void) {
  log_info("register all");
  register_ortho_dof_projection();
  register_spherical_projection();
  register_perspective_projection();
  register_fisheye_projection();
  register_ortho_projection();
  register_dof_projection();
}

camera_t* camera_create(projection_type_t type, wf_vec3 position,
                        wf_vec3 target, wf_vec3 up, const void* params) {
  // camera_init_builtin_projections();
  if (type >= sizeof(s_projections) / sizeof(s_projections[0])
      || !s_projections[type]) {
    log_error("parameter error");
    return NULL;
  }

  camera_t* cam = (camera_t*)calloc(1, sizeof(camera_t));
  if (!cam) {
    log_error("oom when alloc for camera_t");
    return NULL;
  }

  cam->position = position;
  cam->ops      = s_projections[type];
  cam->priv     = cam->ops->init(position, target, up, params);
  if (!cam->priv) {
    free(cam);
    return NULL;
  }
  return cam;
}

wf_vec3 camera_get_position(const camera_t* cam) {
  return cam->position;
}

wf_vec3 camera_get_ray_direction(const camera_t* cam, float u, float v) {
  return cam->ops->get_ray_direction(cam->priv, u, v);
}

void camera_destroy(camera_t* cam) {
  if (cam) {
    if (cam->ops && cam->ops->exit)
      cam->ops->exit(cam->priv);
    free(cam);
  }
}
