// accumulator.h
#ifndef ACCUMULATOR_H
#define ACCUMULATOR_H

#include "wavefront.h" // for wf_vec3

typedef struct accumulator_s accumulator_t;

// 初始化
typedef void (*acc_init_fn)(accumulator_t* self, int spp);
// 累加一个样本
typedef void (*acc_add_fn)(accumulator_t* self, const wf_vec3* color,
                           int index);
// 获取最终颜色
typedef wf_vec3 (*acc_get_fn)(const accumulator_t* self);

struct accumulator_s {
  void*       data;
  acc_init_fn init;
  acc_add_fn  add;
  acc_get_fn  get;
};

accumulator_t* accumulator_create_average(void);
// 可扩展：accumulator_create_median(), etc.
void accumulator_destroy(accumulator_t* acc);

#endif
