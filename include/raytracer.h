#ifndef RAYTRACER_H
#define RAYTRACER_H

#include "config.h"
typedef struct {
  int         width, height;
  const char* output;
  int         verbose;
} Config;

int render_scene(const rtCfg* cfg);

#endif
