// sampler_random.c
#include <stdlib.h>
#include "sample/sampler.h"

typedef struct {
  int spp;
} random_data_t;

static void random_generate(const sampler_t* self, size_t idx, float* u,
                            float* v) {
  (void)self;
  (void)idx;
  *u = (float)rand() / RAND_MAX;
  *v = (float)rand() / RAND_MAX;
}

sampler_t* sampler_create_random(int spp) {
  sampler_t*     s = malloc(sizeof(sampler_t));
  random_data_t* d = malloc(sizeof(random_data_t));
  d->spp           = spp;
  s->data          = d;
  s->generate      = random_generate;
  return s;
}
