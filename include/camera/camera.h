#ifndef CAMERA_H
#define CAMERA_H

#include "algo.h" // wf_vec3 etc

typedef struct camera camera_t;

typedef enum {
  PROJ_PERSPECTIVE = 0,
  PROJ_ORTHOGRAPHIC,
  PROJ_FISHEYE,
  PROJ_SPHERICAL,
  PROJ_PERSPECTIVE_DOF,
  PROJ_ORTHOGRAPHIC_DOF,
  PROJ_END,
} projection_type_t;

// Parameters for depth-of-field camera
typedef struct {
  float fov_y_rad;      // Vertical field of view (radians)
  float aspect_ratio;   // Image aspect ratio (width / height)
  float aperture;       // Aperture size (0.0 = no DoF, pinhole camera)
  float focus_distance; // Distance to the focal plane (where objects are sharp)
} dof_params_t;

typedef struct {
  float width;          // Orthographic view width
  float height;         // Orthographic view height
  float aperture;       // Aperture size (0 = no DoF)
  float focus_distance; // Distance to focal plane (for DoF)
} ortho_dof_params_t;

// user create camera
camera_t* camera_create(projection_type_t type, wf_vec3 position,
                        wf_vec3 target, wf_vec3 up,
                        const void* params); // class related parameter

wf_vec3 camera_get_position(const camera_t* cam);
wf_vec3 camera_get_ray_direction(const camera_t* cam, float u, float v);
void    camera_destroy(camera_t* cam);

#endif
