// raytracer.c
#include "raytracer.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "algo.h"
#include "camera/camera.h"
#include "fileio.h"
#include "log4c.h"
#include "rt_material.h"
#include "rt_types.h"
#include "wavefront.h"

typedef struct {
  wf_vec3 position;
  wf_vec3 color;
  int     type; // 0: point light
} light_t;

static bool hit_scene(const ray_t* ray, const wf_face* tris, size_t tri_count,
                      const wf_scene_t* scene, hit_record_t* rec);

static bool in_shadow(const wf_vec3* p, const wf_vec3* light_pos,
                      const wf_face* tris, size_t tri_count,
                      const wf_scene_t* scene) {
  wf_vec3 dir  = { light_pos->x - p->x, light_pos->y - p->y,
                   light_pos->z - p->z };
  float   dist = sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
  if (dist < 1e-6f)
    return false;
  dir.x /= dist;
  dir.y /= dist;
  dir.z /= dist;

  ray_t        shadow_ray = { .origin = *p, .direction = dir };
  hit_record_t shadow_rec;
  if (hit_scene(&shadow_ray, tris, tri_count, scene, &shadow_rec)) {
    return shadow_rec.t < dist - 1e-4f;
  }
  return false;
}

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
// 在 raytracer.c 中
static bool hit_scene(const ray_t* ray, const wf_face* tris, size_t tri_count,
                      const wf_scene_t* scene, hit_record_t* rec) {
  bool  hit       = false;
  float closest_t = INFINITY;

  for (size_t i = 0; i < tri_count; ++i) {
    const wf_face* face = &tris[i];
    const wf_vec3* v0   = &scene->vertices[face->vertices[0].v_idx];
    const wf_vec3* v1   = &scene->vertices[face->vertices[1].v_idx];
    const wf_vec3* v2   = &scene->vertices[face->vertices[2].v_idx];

    float t, u, v;
    if (!ray_intersects_triangle(ray, v0, v1, v2, &t, &u, &v))
      continue;
    if (t <= 1e-4f || t >= closest_t)
      continue;
    closest_t = t;
    hit       = true;

    // 计算交点
    rec->point.x = ray->origin.x + t * ray->direction.x;
    rec->point.y = ray->origin.y + t * ray->direction.y;
    rec->point.z = ray->origin.z + t * ray->direction.z;

    // 插值法线
    if (face->vertices[0].vn_idx >= 0) {
      wf_vec3 n0    = scene->normals[face->vertices[0].vn_idx];
      wf_vec3 n1    = scene->normals[face->vertices[1].vn_idx];
      wf_vec3 n2    = scene->normals[face->vertices[2].vn_idx];
      rec->normal.x = (1 - u - v) * n0.x + u * n1.x + v * n2.x;
      rec->normal.y = (1 - u - v) * n0.y + u * n1.y + v * n2.y;
      rec->normal.z = (1 - u - v) * n0.z + u * n1.z + v * n2.z;
    } else {
      // 面法线
      wf_vec3 e1    = { v1->x - v0->x, v1->y - v0->y, v1->z - v0->z };
      wf_vec3 e2    = { v2->x - v0->x, v2->y - v0->y, v2->z - v0->z };
      rec->normal.x = e1.y * e2.z - e1.z * e2.y;
      rec->normal.y = e1.z * e2.x - e1.x * e2.z;
      rec->normal.z = e1.x * e2.y - e1.y * e2.x;
    }
    rec->normal = v3_normalize(rec->normal);

    rec->t            = t;
    rec->material_idx = face->material_idx;
  }

  if (hit)
    *rec = *rec; // ensure output
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
  rt_material_t** rt_materials =
      calloc(scene.material_count, sizeof(rt_material_t*));
  for (size_t i = 0; i < scene.material_count; ++i) {
    rt_materials[i] = rt_material_from_wf(&scene.materials[i]);
  }

  // 定义光源（可扩展为数组）
  light_t lights[] = {
    { .position = { 5.0f, 5.0f, 5.0f },
     .color    = { 1.0f, 1.0f, 1.0f },
     .type     = 0 },
    { .position = { -3.0f, 4.0f, -2.0f },
     .color    = { 0.8f, 0.8f, 1.0f },
     .type     = 0 }
  };
  size_t num_lights = sizeof(lights) / sizeof(lights[0]);

  for (int y = 0; y < cfg->height; ++y) {
    for (int x = 0; x < cfg->width; ++x) {
      ray_t ray = get_camera_ray(cam, x, y, cfg->width, cfg->height);

      wf_vec3      final_color = { 0, 0, 0 };
      hit_record_t rec;
      bool    hit = hit_scene(&ray, triangles, triangle_count, &scene, &rec);
      uint8_t r, g, b;
      if (hit) {
        wf_vec3 view_dir = { -ray.direction.x, -ray.direction.y,
                             -ray.direction.z };

        rt_material_t* mat = rt_materials[rec.material_idx];
        view_dir           = v3_normalize(view_dir);

        for (size_t li = 0; li < num_lights; ++li) {
          if (in_shadow(&rec.point, &lights[li].position, triangles,
                        triangle_count, &scene)) {
            continue;
          }

          wf_vec3 wi = { lights[li].position.x - rec.point.x,
                         lights[li].position.y - rec.point.y,
                         lights[li].position.z - rec.point.z };
          wi         = v3_normalize(wi);

          float ndotwi = v3_dot(rec.normal, wi);
          if (ndotwi <= 0)
            continue;

          // 调用 BRDF eval
          wf_vec3 fr =
              mat->brdf->ops->eval(mat->brdf, &wi, &view_dir, &rec.normal);

          final_color.x += fr.x * lights[li].color.x * ndotwi;
          final_color.y += fr.y * lights[li].color.y * ndotwi;
          final_color.z += fr.z * lights[li].color.z * ndotwi;
        }
        r = (uint8_t)(fminf(1.0f, fmaxf(0.0f, final_color.x)) * 255);
        g = (uint8_t)(fminf(1.0f, fmaxf(0.0f, final_color.y)) * 255);
        b = (uint8_t)(fminf(1.0f, fmaxf(0.0f, final_color.z)) * 255);
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
  for (size_t i = 0; i < scene.material_count; ++i) {
    rt_material_destroy(rt_materials[i]);
  }
  free(rt_materials);
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
