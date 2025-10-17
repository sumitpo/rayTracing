// sampler_jittered.c
#include <math.h>
#include <stdlib.h>
#include "sample/sampler.h"

typedef struct {
  int spp;
  int grid_size;
} jittered_data_t;

static void jittered_generate(const sampler_t* self, size_t idx, float* u,
                              float* v) {
  jittered_data_t* d        = (jittered_data_t*)self->data;
  int              gx       = idx % d->grid_size;
  int              gy       = idx / d->grid_size;
  float            jitter_u = ((float)rand() / RAND_MAX) / d->grid_size;
  float            jitter_v = ((float)rand() / RAND_MAX) / d->grid_size;
  *u                        = (gx + jitter_u) / d->grid_size;
  *v                        = (gy + jitter_v) / d->grid_size;
}

sampler_t* sampler_create_jittered(int spp) {
  sampler_t*       s = malloc(sizeof(sampler_t));
  jittered_data_t* d = malloc(sizeof(jittered_data_t));
  d->spp             = spp;
  d->grid_size       = (int)sqrtf(spp);
  while (d->grid_size * d->grid_size < spp)
    d->grid_size++;
  s->data     = d;
  s->generate = jittered_generate;
  return s;
}
