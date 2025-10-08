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

// Test ray against all triangles
static bool hit_scene(const ray_t* ray, const wf_face* triangles,
                      size_t tri_count, const wf_scene_t* scene, float* t_min) {
  bool  hit       = false;
  float closest_t = INFINITY;

  for (size_t i = 0; i < tri_count; ++i) {
    const wf_face* face = &triangles[i];
    // Get vertex positions from scene
    const wf_vec3* v0 = &scene->vertices[face->vertices[0].v_idx];
    const wf_vec3* v1 = &scene->vertices[face->vertices[1].v_idx];
    const wf_vec3* v2 = &scene->vertices[face->vertices[2].v_idx];

    float t;
    if (ray_intersects_triangle(ray, v0, v1, v2, &t)) {
      if (t < closest_t) {
        closest_t = t;
        hit       = true;
      }
    }
  }

  if (hit && t_min)
    *t_min = closest_t;
  return hit;
}

static ray_t get_camera_ray(const camera_t* cam, int x, int y, int width,
                            int height) {
  float   u         = (float)x / (float)(width - 1);
  float   v         = (float)y / (float)(height - 1);
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
  wf_vec3 position = { 0.0f, 0.0f, 3.0f };
  wf_vec3 target   = { 0.0f, 0.0f, 0.0f };
  wf_vec3 up       = { 0.0f, 1.0f, 0.0f };
  float   fov_y    = M_PI / 3.0f;
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
      bool  hit = hit_scene(&ray, triangles, triangle_count, &scene, NULL);

      uint8_t r = hit ? 255 : 0;
      uint8_t g = hit ? 100 : 0;
      uint8_t b = hit ? 50 : 0;

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
