// sampler_regular.c
#include <math.h>
#include <stdlib.h>
#include "sample/sampler.h"

typedef struct {
  int spp;
  int grid_size; // sqrt(spp)
} regular_data_t;

static void regular_generate(const sampler_t* self, size_t idx, float* u,
                             float* v) {
  regular_data_t* data = (regular_data_t*)self->data;
  int             gx   = idx % data->grid_size;
  int             gy   = idx / data->grid_size;
  *u                   = (gx + 0.5f) / data->grid_size;
  *v                   = (gy + 0.5f) / data->grid_size;
}

sampler_t* sampler_create_regular_grid(int spp) {
  sampler_t*      s = malloc(sizeof(sampler_t));
  regular_data_t* d = malloc(sizeof(regular_data_t));
  d->spp            = spp;
  d->grid_size      = (int)sqrtf(spp);
  while (d->grid_size * d->grid_size < spp)
    d->grid_size++; // ensure >= spp
  s->data     = d;
  s->generate = regular_generate;
  return s;
}
