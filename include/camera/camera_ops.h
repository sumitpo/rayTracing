// camera_ops.h (internal)
#ifndef CAMERA_OPS_H
#define CAMERA_OPS_H

#include "camera.h"

// 投影策略接口（仿 tcp_congestion_ops）
struct camera_projection_ops {
  projection_type_t type;
  const char*       name;

  // 初始化私有数据
  void* (*init)(wf_vec3 pos, wf_vec3 target, wf_vec3 up, const void* params);
  // 生成射线方向
  wf_vec3 (*get_ray_direction)(const void* priv, float u, float v);
  // 清理
  void (*exit)(void* priv);
};

// 注册宏（仿 Linux 内核）
#define CAMERA_PROJECTION_OPS_REGISTER(ops)                                    \
  static void __attribute__((constructor)) __register_##ops(void) {            \
    camera_register_projection(&ops);                                          \
  }

void camera_register_projection(const struct camera_projection_ops* ops);

// 投影策略注册表（仿拥塞控制）
static const struct camera_projection_ops* s_projections[6] = { 0 };

void register_ortho_dof_projection(void);
void register_spherical_projection(void);
void register_perspective_projection(void);
void register_fisheye_projection(void);
void register_ortho_projection(void);
void register_dof_projection(void);

#endif
