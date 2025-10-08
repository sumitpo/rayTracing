// bvh_ops.h
#ifndef BVH_OPS_H
#define BVH_OPS_H

#include "bvh/bvh.h"
#include "raytracer.h"

// BVH strategy interface (like Linux tcp_congestion_ops)
struct bvh_ops {
  const char* name;
  bvh_tree_t* (*build)(const wf_face* faces, size_t face_count,
                       const wf_scene_t* scene);
  void (*destroy)(bvh_tree_t* tree);
  bool (*intersect)(const bvh_tree_t* tree, const ray_t* ray, float* t_hit);
};

// Auto-register macro (like Linux module_init)
#define BVH_OPS_REGISTER(ops)                                                  \
  static void __attribute__((constructor)) __register_##ops(void) {            \
    bvh_register_ops(&ops);                                                    \
  }

void                  bvh_register_ops(const struct bvh_ops* ops);
const struct bvh_ops* bvh_get_ops(const char* name);

#endif // BVH_OPS_H
