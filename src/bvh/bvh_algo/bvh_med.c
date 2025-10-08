// bvh_median.c
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include "algo.h"
#include "bvh/bvh_ops.h"

static struct bvh_ops median_ops;

// Compute bounding box of a single face
static void compute_face_bbox(const wf_face* face, const wf_scene_t* scene,
                              wf_vec3* bbox_min, wf_vec3* bbox_max) {
  wf_vec3 min = { FLT_MAX, FLT_MAX, FLT_MAX };
  wf_vec3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

  for (int i = 0; i < 3; ++i) {
    const wf_vec3* v = &scene->vertices[face->vertices[i].v_idx];
    min.x            = fminf(min.x, v->x);
    min.y            = fminf(min.y, v->y);
    min.z            = fminf(min.z, v->z);
    max.x            = fmaxf(max.x, v->x);
    max.y            = fmaxf(max.y, v->y);
    max.z            = fmaxf(max.z, v->z);
  }
  *bbox_min = min;
  *bbox_max = max;
}

// Compute centroid of a face
static wf_vec3 compute_face_centroid(const wf_face*    face,
                                     const wf_scene_t* scene) {
  wf_vec3 c = v3_zero();
  for (int i = 0; i < 3; ++i) {
    c = v3_add(c, scene->vertices[face->vertices[i].v_idx]);
  }
  return v3_scale(1.0f / 3.0f, c);
}

// Expand a bounding box to include another
static void expand_bbox(wf_vec3* min, wf_vec3* max, const wf_vec3* bmin,
                        const wf_vec3* bmax) {
  min->x = fminf(min->x, bmin->x);
  min->y = fminf(min->y, bmin->y);
  min->z = fminf(min->z, bmin->z);
  max->x = fmaxf(max->x, bmax->x);
  max->y = fmaxf(max->y, bmax->y);
  max->z = fmaxf(max->z, bmax->z);
}

// Simple in-place partition around median (selection sort for small N)
static size_t partition_median(uint32_t* indices, wf_vec3* centroids,
                               size_t start, size_t end, int axis) {
  size_t n = end - start;
  if (n <= 1)
    return start;

  // Find median by partial sort (good enough for BVH)
  for (size_t i = start; i < start + n / 2 + 1; ++i) {
    size_t min_idx = i;
    for (size_t j = i + 1; j < end; ++j) {
      float val_j   = (&centroids[indices[j]].x)[axis];
      float val_min = (&centroids[indices[min_idx]].x)[axis];
      if (val_j < val_min) {
        min_idx = j;
      }
    }
    if (min_idx != i) {
      uint32_t tmp     = indices[i];
      indices[i]       = indices[min_idx];
      indices[min_idx] = tmp;
    }
  }
  return start + n / 2;
}

// Recursive BVH builder (emulated with manual stack to avoid deep recursion)
typedef struct {
  uint32_t*         face_indices;
  wf_vec3*          centroids;
  wf_vec3*          face_min;
  wf_vec3*          face_max;
  size_t            face_count;
  const wf_scene_t* scene;
  bvh_node_t*       nodes;
  size_t            node_count;
  size_t            node_cap;
} build_context_t;

static uint32_t alloc_node(build_context_t* ctx) {
  if (ctx->node_count >= ctx->node_cap) {
    ctx->node_cap = ctx->node_cap ? ctx->node_cap * 2 : 64;
    ctx->nodes    = realloc(ctx->nodes, ctx->node_cap * sizeof(bvh_node_t));
  }
  return (uint32_t)(ctx->node_count++);
}

static uint32_t build_node(build_context_t* ctx, size_t face_start,
                           size_t face_end) {
  if (face_end - face_start <= 4) {
    // Create leaf
    uint32_t    idx  = alloc_node(ctx);
    bvh_node_t* node = &ctx->nodes[idx];
    node->is_leaf    = true;
    node->leaf.start = (uint32_t)face_start;
    node->leaf.count = (uint32_t)(face_end - face_start);

    node->bbox_min = (wf_vec3){ FLT_MAX, FLT_MAX, FLT_MAX };
    node->bbox_max = (wf_vec3){ -FLT_MAX, -FLT_MAX, -FLT_MAX };
    for (size_t i = face_start; i < face_end; ++i) {
      expand_bbox(&node->bbox_min, &node->bbox_max,
                  &ctx->face_min[ctx->face_indices[i]],
                  &ctx->face_max[ctx->face_indices[i]]);
    }
    return idx;
  }

  // Find axis with largest centroid extent
  wf_vec3 centroid_min = ctx->centroids[ctx->face_indices[face_start]];
  wf_vec3 centroid_max = centroid_min;
  for (size_t i = face_start + 1; i < face_end; ++i) {
    wf_vec3 c      = ctx->centroids[ctx->face_indices[i]];
    centroid_min.x = fminf(centroid_min.x, c.x);
    centroid_min.y = fminf(centroid_min.y, c.y);
    centroid_min.z = fminf(centroid_min.z, c.z);
    centroid_max.x = fmaxf(centroid_max.x, c.x);
    centroid_max.y = fmaxf(centroid_max.y, c.y);
    centroid_max.z = fmaxf(centroid_max.z, c.z);
  }
  wf_vec3 extent = v3_sub(centroid_max, centroid_min);
  int     axis   = 0;
  if (extent.y > extent.x && extent.y > extent.z)
    axis = 1;
  else if (extent.z > extent.x)
    axis = 2;

  // Partition around median
  size_t mid = partition_median(ctx->face_indices, ctx->centroids, face_start,
                                face_end, axis);

  // Build children
  uint32_t left_idx  = build_node(ctx, face_start, mid);
  uint32_t right_idx = build_node(ctx, mid, face_end);

  // Create internal node
  uint32_t    idx      = alloc_node(ctx);
  bvh_node_t* node     = &ctx->nodes[idx];
  node->is_leaf        = false;
  node->internal.left  = left_idx;
  node->internal.right = right_idx;

  // Merge child bboxes
  node->bbox_min = ctx->nodes[left_idx].bbox_min;
  node->bbox_max = ctx->nodes[left_idx].bbox_max;
  expand_bbox(&node->bbox_min, &node->bbox_max, &ctx->nodes[right_idx].bbox_min,
              &ctx->nodes[right_idx].bbox_max);
  return idx;
}

// Ray-box intersection (slab method)
static bool ray_intersects_bbox(const ray_t* ray, const wf_vec3* bbox_min,
                                const wf_vec3* bbox_max) {
  float t_min = 0.0f, t_max = INFINITY;
  for (int i = 0; i < 3; ++i) {
    float origin  = (&ray->origin.x)[i];
    float dir     = (&ray->direction.x)[i];
    float min_val = (&bbox_min->x)[i];
    float max_val = (&bbox_max->x)[i];

    if (fabsf(dir) < 1e-8f) {
      if (origin < min_val || origin > max_val)
        return false;
    } else {
      float t1 = (min_val - origin) / dir;
      float t2 = (max_val - origin) / dir;
      if (t1 > t2) {
        float tmp = t1;
        t1        = t2;
        t2        = tmp;
      }
      t_min = fmaxf(t_min, t1);
      t_max = fminf(t_max, t2);
      if (t_min > t_max)
        return false;
    }
  }
  return true;
}

// Build function (strategy interface)
static bvh_tree_t* median_build(const wf_face* faces, size_t face_count,
                                const wf_scene_t* scene) {
  if (face_count == 0)
    return NULL;

  build_context_t ctx = { 0 };
  ctx.scene           = scene;
  ctx.face_count      = face_count;

  // Allocate working arrays
  ctx.face_indices = malloc(face_count * sizeof(uint32_t));
  ctx.centroids    = malloc(face_count * sizeof(wf_vec3));
  ctx.face_min     = malloc(face_count * sizeof(wf_vec3));
  ctx.face_max     = malloc(face_count * sizeof(wf_vec3));

  for (size_t i = 0; i < face_count; ++i) {
    ctx.face_indices[i] = (uint32_t)i;
    ctx.centroids[i]    = compute_face_centroid(&faces[i], scene);
    compute_face_bbox(&faces[i], scene, &ctx.face_min[i], &ctx.face_max[i]);
  }

  // Build tree
  ctx.nodes         = NULL;
  ctx.node_count    = 0;
  ctx.node_cap      = 0;
  uint32_t root_idx = build_node(&ctx, 0, face_count);

  // Create final tree
  bvh_tree_t* tree = malloc(sizeof(bvh_tree_t));
  tree->nodes      = ctx.nodes;
  tree->node_count = ctx.node_count;
  tree->faces      = faces;
  tree->scene      = scene;
  tree->ops        = &median_ops; // critical: link to ops

  free(ctx.face_indices);
  free(ctx.centroids);
  free(ctx.face_min);
  free(ctx.face_max);
  return tree;
}

// Intersect function (strategy interface)
static bool median_intersect(const bvh_tree_t* tree, const ray_t* ray,
                             float* t_hit) {
  if (!tree || tree->node_count == 0)
    return false;

  uint32_t stack[64];
  uint32_t stack_ptr = 0;
  stack[stack_ptr++] = 0; // root index

  float closest_t = INFINITY;
  bool  hit       = false;

  while (stack_ptr > 0) {
    uint32_t          node_idx = stack[--stack_ptr];
    const bvh_node_t* node     = &tree->nodes[node_idx];

    if (!ray_intersects_bbox(ray, &node->bbox_min, &node->bbox_max))
      continue;

    if (node->is_leaf) {
      // Test all triangles in leaf
      for (uint32_t i = 0; i < node->leaf.count; ++i) {
        const wf_face* face = &tree->faces[node->leaf.start + i];
        const wf_vec3* v0   = &tree->scene->vertices[face->vertices[0].v_idx];
        const wf_vec3* v1   = &tree->scene->vertices[face->vertices[1].v_idx];
        const wf_vec3* v2   = &tree->scene->vertices[face->vertices[2].v_idx];

        float t;
        if (ray_intersects_triangle(ray, v0, v1, v2, &t) && t < closest_t) {
          closest_t = t;
          hit       = true;
        }
      }
    } else {
      // Push children (order doesn't affect correctness)
      if (stack_ptr < 63)
        stack[stack_ptr++] = node->internal.right;
      if (stack_ptr < 63)
        stack[stack_ptr++] = node->internal.left;
    }
  }

  if (hit && t_hit)
    *t_hit = closest_t;
  return hit;
}

// Destroy function (strategy interface)
static void median_destroy(bvh_tree_t* tree) {
  if (tree) {
    free(tree->nodes);
    free(tree);
  }
}

// Strategy registration
static struct bvh_ops median_ops = {
  .name      = "median",
  .build     = median_build,
  .destroy   = median_destroy,
  .intersect = median_intersect,
};

BVH_OPS_REGISTER(median_ops)
