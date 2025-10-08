// bvh.c
#include "bvh/bvh.h"
#include <stdlib.h>
#include <string.h>
#include "bvh/bvh_ops.h"
#include "wavefront.h"

// Registry (static array for simplicity)
static const struct bvh_ops* s_bvh_registry[8] = { 0 };
static size_t                s_registry_count  = 0;

void bvh_register_ops(const struct bvh_ops* ops) {
  if (s_registry_count < sizeof(s_bvh_registry) / sizeof(s_bvh_registry[0])) {
    s_bvh_registry[s_registry_count++] = ops;
  }
}

const struct bvh_ops* bvh_get_ops(const char* name) {
  if (!name)
    return s_bvh_registry[0]; // default to first
  for (size_t i = 0; i < s_registry_count; ++i) {
    if (strcmp(s_bvh_registry[i]->name, name) == 0) {
      return s_bvh_registry[i];
    }
  }
  return NULL;
}

// Public API
bvh_tree_t* bvh_create(const char* type, const wf_face* faces,
                       size_t face_count, const wf_scene_t* scene) {
  const struct bvh_ops* ops = bvh_get_ops(type);
  if (!ops)
    return NULL;
  return ops->build(faces, face_count, scene);
}

bool bvh_intersect(const bvh_tree_t* tree, const ray_t* ray, float* t_hit) {
  if (!tree || !tree->ops)
    return false;
  return tree->ops->intersect(tree, ray, t_hit);
}

void bvh_destroy(bvh_tree_t* tree) {
  if (tree && tree->ops) {
    tree->ops->destroy(tree);
  }
}
