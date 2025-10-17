// brdf.c
#include "brdf/brdf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log4c.h"

#define MAX_BRDF_MODELS 16

static const brdf_ops_t* registered_models[MAX_BRDF_MODELS];
static int               num_models = 0;

void brdf_register_model(const brdf_ops_t* ops) {
  log_info("register proj [%s]", ops->name);
  registered_models[ops->type] = ops;
  num_models += 1;
}

const brdf_ops_t* brdf_find_model(const char* name) {
  for (int i = 0; i < num_models; ++i) {
    if (strcmp(registered_models[i]->name, name) == 0) {
      return registered_models[i];
    }
  }
  return NULL;
}

void brdf_init_builtin_brdf(void) {
  log_info("register all brdf");
  register_lambert_brdf();
  register_cook_torrance_brdf();
}

brdf_t* brdf_create(const char* model_name, const void* params) {
  log_info("search for brdf %s", model_name);
  if (registered_models[0] == NULL) {
    brdf_init_builtin_brdf();
  }
  const brdf_ops_t* ops = brdf_find_model(model_name);
  if (!ops) {
    log_error("couldn't found %s", model_name);
    return NULL;
  }
  log_debug("choose %s", ops->name);
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
