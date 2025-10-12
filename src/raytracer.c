// raytracer.c
#include "raytracer.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "algo.h"
#include "camera/camera.h"
#include "fileio.h"
#include "log4c.h"
#include "wavefront.h"

static void get_face_color(const wf_scene_t* scene, size_t face_idx, uint8_t* r,
                           uint8_t* g, uint8_t* b) {
  if (face_idx >= 2 && face_idx <= 3) {
    *r = 100;
    *g = 100;
    *b = 100;
  } else if (face_idx >= 4 && face_idx <= 5) {
    *r = 0;
    *g = 255;
    *b = 100;
  } else if (face_idx >= 6 && face_idx <= 7) {
    *r = 100;
    *g = 255;
    *b = 100;
  } else if (face_idx >= 8 && face_idx <= 9) {
    *r = 255;
    *g = 100;
    *b = 100;
  } else if (face_idx >= 10 && face_idx <= 21) {
    *r = 255;
    *g = 100;
    *b = 0;
  } else if (face_idx >= 22 && face_idx <= 33) {
    *r = 255;
    *g = 255;
    *b = 200;
  } else {
    *r = 200;
    *g = 200;
    *b = 200;
  }
}

// Test ray against all triangles
static bool hit_scene(const ray_t* ray, const wf_face* triangles,
                      size_t tri_count, const wf_scene_t* scene, float* t_min,
                      size_t* hit_face_idx) {
  bool   hit         = false;
  float  closest_t   = INFINITY;
  size_t closest_idx = 0;

  for (size_t i = 0; i < tri_count; ++i) {
    const wf_face* face = &triangles[i];
    const wf_vec3* v0   = &scene->vertices[face->vertices[0].v_idx];
    const wf_vec3* v1   = &scene->vertices[face->vertices[1].v_idx];
    const wf_vec3* v2   = &scene->vertices[face->vertices[2].v_idx];

    float t;
    if (ray_intersects_triangle(ray, v0, v1, v2, &t)) {
      if (t < closest_t) {
        closest_t   = t;
        closest_idx = i;
        hit         = true;
      }
    }
  }

  if (hit) {
    if (t_min)
      *t_min = closest_t;
    if (hit_face_idx)
      *hit_face_idx = closest_idx;
  }
  return hit;
}

static ray_t get_camera_ray(const camera_t* cam, int x, int y, int width,
                            int height) {
  float u = (width > 1) ? (float)x / (float)(width - 1) : 0.5f;
  float v = (height > 1) ? 1.0f - (float)y / (float)(height - 1) : 0.5f;

  wf_vec3 origin    = camera_get_position(cam);
  wf_vec3 direction = camera_get_ray_direction(cam, u, v);
  return (ray_t){ .origin = origin, .direction = direction };
}

int render_scene(const rtCfg* cfg) {
  log_info("Rendering scene: %dx%d", cfg->width, cfg->height);

  // Initialize scene
  wf_scene_t         scene   = { 0 };
  wf_parse_options_t options = { 0 };
  wf_parse_options_init(&options);

  // Load OBJ (MTL is loaded automatically if referenced)
  wf_error_t err = wf_load_obj(cfg->obj_file, &scene, &options);
  if (err != WF_SUCCESS) {
    const char* msg = wf_get_error(&scene);
    log_error("Failed to load OBJ: %s", msg ? msg : "Unknown error");
    wf_free_scene(&scene);
    return 1;
  }

  wf_print_options_t opt;
  wf_print_scene(&scene, &opt);

  // Convert to flat triangle array for efficient ray tracing
  wf_face* triangles      = NULL;
  size_t   triangle_count = 0;
  err = wf_scene_to_triangles(&scene, &triangles, &triangle_count);
  if (err != WF_SUCCESS) {
    log_error("Failed to triangulate scene");
    wf_free_scene(&scene);
    return 1;
  }

  // Create camera
  wf_vec3 position = { 0.0f, 1.0f, 2.8f };
  wf_vec3 target   = { 0.0f, 1.0f, -1.0f };
  wf_vec3 up       = { 0.0f, 1.0f, 0.0f };
  float   fov_y    = 60.0f * M_PI / 180.0f;
  float   aspect   = (float)cfg->width / (float)cfg->height;

  camera_t* cam =
      camera_create(PROJ_PERSPECTIVE, position, target, up, &(struct {
                      float fov_y_rad;
                      float aspect_ratio;
                    }){ fov_y, aspect });
  if (!cam) {
    log_error("create camera failed");
    free(triangles);
    wf_free_scene(&scene);
    return 1;
  }

  // Render
  size_t   size  = (size_t)cfg->width * cfg->height * 4;
  uint8_t* image = calloc(size, 1);
  if (!image) {
    camera_destroy(cam);
    free(triangles);
    wf_free_scene(&scene);
    return 1;
  }

  for (int y = 0; y < cfg->height; ++y) {
    for (int x = 0; x < cfg->width; ++x) {
      ray_t ray = get_camera_ray(cam, x, y, cfg->width, cfg->height);

      size_t  hit_face_idx;
      bool    hit = hit_scene(&ray, triangles, triangle_count, &scene, NULL,
                              &hit_face_idx);
      uint8_t r, g, b;
      if (hit) {
        get_face_color(&scene, hit_face_idx, &r, &g, &b);
      } else {
        r = g = b = 0; // 背景黑色
      }

      size_t idx     = (y * cfg->width + x) * 4;
      image[idx]     = r;
      image[idx + 1] = g;
      image[idx + 2] = b;
      image[idx + 3] = 255;
    }
  }

  int result = save_png(cfg->output, cfg->width, cfg->height, image);
  free(image);
  camera_destroy(cam);
  free(triangles);
  wf_free_scene(&scene);

  if (result != 0) {
    log_error("Failed to save PNG (code: %d)", result);
    return result;
  }

  log_info("Saved to %s", cfg->output);
  return 0;
}
