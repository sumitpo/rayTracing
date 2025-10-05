#include <stdlib.h>
#include "cli.h"
#include "log4c.h"
#include "raytracer.h"

int main(int argc, char** argv) {
  log_init(LOG_LEVEL_DEBUG);

  rtCfg cfg;
  if (parse_args(argc, argv, &cfg) != 0)
    return 1;

  if (render_scene(&cfg) != 0) {
    log_error("Render failed");
    return 1;
  }

  return 0;
}
