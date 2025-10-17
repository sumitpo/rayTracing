// accumulator_average.c
#include <stdlib.h>
#include <string.h>
#include "sample/accumulator.h"

typedef struct {
  wf_vec3 sum;
  int     count;
} avg_data_t;

static void avg_init(accumulator_t* self, int spp) {
  avg_data_t* d = (avg_data_t*)self->data;
  d->sum        = (wf_vec3){ 0, 0, 0 };
  d->count      = 0;
}

static void avg_add(accumulator_t* self, const wf_vec3* color, int index) {
  avg_data_t* d = (avg_data_t*)self->data;
  d->sum.x += color->x;
  d->sum.y += color->y;
  d->sum.z += color->z;
  d->count++;
}

static wf_vec3 avg_get(const accumulator_t* self) {
  avg_data_t* d = (avg_data_t*)self->data;
  if (d->count == 0)
    return (wf_vec3){ 0, 0, 0 };
  return (wf_vec3){ d->sum.x / d->count, d->sum.y / d->count,
                    d->sum.z / d->count };
}

accumulator_t* accumulator_create_average(void) {
  accumulator_t* a = malloc(sizeof(accumulator_t));
  avg_data_t*    d = malloc(sizeof(avg_data_t));
  a->data          = d;
  a->init          = avg_init;
  a->add           = avg_add;
  a->get           = avg_get;
  return a;
}
