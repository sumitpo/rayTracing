#ifndef BVH_H
#define BVH_H
#include <stdbool.h>
#include <stdint.h>
#include "wavefront.h"

// bvh.h
typedef struct bvh_node_s {
  wf_vec3 bbox_min;
  wf_vec3 bbox_max;
  union {
    struct {
      uint32_t left;  // child index
      uint32_t right; // child index
    } internal;
    struct {
      uint32_t start; // face index start
      uint32_t count; // face count (leaf)
    } leaf;
  };
  bool is_leaf;
} bvh_node_t;

typedef struct bvh_tree_s {
  bvh_node_t*           nodes;
  size_t                node_count;
  size_t                node_cap;
  const wf_face*        faces; // pointer to original faces
  const wf_scene_t*     scene; // pointer to scene (for vertex lookup)
  const struct bvh_ops* ops;   // back-pointer to ops
} bvh_tree_t;
#endif
