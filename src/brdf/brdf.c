// brdf.c
#include "brdf/brdf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BRDF_MODELS 16

static const brdf_ops_t* registered_models[MAX_BRDF_MODELS];
static int               num_models = 0;

int brdf_register_model(const brdf_ops_t* ops) {
  if (num_models >= MAX_BRDF_MODELS)
    return -1;
  for (int i = 0; i < num_models; ++i) {
    if (strcmp(registered_models[i]->name, ops->name) == 0) {
      return 0; // already registered
    }
  }
  registered_models[num_models++] = ops;
  return 0;
}

const brdf_ops_t* brdf_find_model(const char* name) {
  for (int i = 0; i < num_models; ++i) {
    if (strcmp(registered_models[i]->name, name) == 0) {
      return registered_models[i];
    }
  }
  return NULL;
}

brdf_t* brdf_create(const char* model_name, const void* params) {
  const brdf_ops_t* ops = brdf_find_model(model_name);
  if (!ops)
    return NULL;
  brdf_t* brdf = malloc(sizeof(brdf_t));
  brdf->ops    = ops;
  brdf->data   = ops->create ? ops->create(params) : NULL;
  return brdf;
}

void brdf_destroy(brdf_t* brdf) {
  if (!brdf)
    return;
  if (brdf->ops && brdf->ops->destroy) {
    brdf->ops->destroy(brdf->data);
  }
  free(brdf);
}
