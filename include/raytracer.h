#ifndef RAYTRACER_H
#define RAYTRACER_H

#include "config.h"
#include "wavefront.h"

typedef struct {
  wf_vec3 origin;
  wf_vec3 direction;
} ray_t;

int render_scene(const rtCfg* cfg);

#endif
