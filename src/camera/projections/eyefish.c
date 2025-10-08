#include <math.h>
#include <stdlib.h>
#include "camera/camera_ops.h"

typedef struct {
  wf_vec3 forward, up, right;
  float   fov_radius; // 最大视角半径（弧度），通常 π/2 或 π
} fisheye_priv_t;

typedef struct {
  float fov_radius_rad; // e.g., M_PI for 180°, M_PI_2 for 90°
} fisheye_params_t;

static void* fisheye_init(wf_vec3 pos, wf_vec3 target, wf_vec3 up,
                          const void* params) {
  const fisheye_params_t* p    = (const fisheye_params_t*)params;
  fisheye_priv_t*         priv = calloc(1, sizeof(fisheye_priv_t));
  if (!priv)
    return NULL;

  wf_vec3 dir      = v3_sub(target, pos);
  priv->forward    = v3_normalize(dir);
  priv->up         = v3_normalize(up);
  priv->right      = v3_normalize(v3_cross(priv->forward, priv->up));
  priv->up         = v3_cross(priv->right, priv->forward);
  priv->fov_radius = p->fov_radius_rad;
  return priv;
}

static wf_vec3 fisheye_get_ray_direction(const void* priv, float u, float v) {
  const fisheye_priv_t* p = (const fisheye_priv_t*)priv;

  // 将 [0,1] 映射到 [-1,1]
  float x = 2.0f * u - 1.0f;
  float y = 1.0f - 2.0f * v; // y=0 在顶部

  float r = sqrtf(x * x + y * y);
  if (r > 1.0f)
    r = 1.0f; // 裁剪到单位圆

  // 等距模型：θ = r * fov_radius
  float theta = r * p->fov_radius;
  float phi   = atan2f(y, x); // 方位角

  // 转换为 3D 方向（在相机局部坐标系）
  float   sin_theta = sinf(theta);
  wf_vec3 local_dir = { .x = sin_theta * cosf(phi),
                        .y = sin_theta * sinf(phi),
                        .z = cosf(theta) };

  wf_vec3 x_scaled = v3_scale(local_dir.x, p->right);
  wf_vec3 y_scaled = v3_scale(local_dir.y, p->up);
  wf_vec3 z_scaled = v3_scale(local_dir.z, p->forward);
  wf_vec3 x_y      = v3_add(x_scaled, y_scaled);

  // 从局部坐标系转到世界坐标系
  wf_vec3 world_dir = v3_add(v3_scale(local_dir.x, p->right),
                             v3_add(v3_scale(local_dir.y, p->up),
                                    v3_scale(local_dir.z, p->forward)));
  return v3_normalize(world_dir);
}

static void fisheye_exit(void* priv) {
  free(priv);
}

static struct camera_projection_ops fisheye_ops = {
  .type              = PROJ_FISHEYE,
  .name              = "fisheye",
  .init              = fisheye_init,
  .get_ray_direction = fisheye_get_ray_direction,
  .exit              = fisheye_exit,
};

CAMERA_PROJECTION_OPS_REGISTER(fisheye_ops)

void register_fisheye_projection(void) {
  camera_register_projection(&fisheye_ops);
}
