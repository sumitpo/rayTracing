#include "raytracer.h"
#include "cli.h"
#include "logger.h"
#include <stdlib.h>

int main(int argc, char **argv) {
  logger_init(NULL);

    rtCfg cfg;
    if (parse_args(argc, argv, &cfg) != 0) return 1;

    if (render_scene(&cfg) != 0) {
        log_error("Render failed");
        return 1;
    }

    logger_fini();
    return 0;
}
