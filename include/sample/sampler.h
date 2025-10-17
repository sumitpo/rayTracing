// sampler.h
#ifndef SAMPLER_H
#define SAMPLER_H

#include <stddef.h>

typedef struct sampler_s sampler_t;

// 生成一个 [0,1)x[0,1) 的样本点
typedef void (*sample_gen_fn)(const sampler_t* self, size_t sample_index,
                              float* u, float* v);

struct sampler_s {
  void*         data;
  sample_gen_fn generate;
};

sampler_t* sampler_create_regular_grid(int spp); // spp = samples per pixel
sampler_t* sampler_create_random(int spp);
sampler_t* sampler_create_jittered(int spp);
void       sampler_destroy(sampler_t* sampler);

#endif
