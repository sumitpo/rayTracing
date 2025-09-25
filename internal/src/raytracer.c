#include "fileio.h"  // ← 替换 png.h
#include "logger.h"
#include "raytracer.h"
#include "config.h"
#include <stdlib.h>
#include <math.h>

int render_scene(const rtCfg* cfg) {
    log_info("Rendering scene: %dx%d", cfg->width, cfg->height);

    size_t size = (size_t)cfg->width * cfg->height * 4;
    uint8_t* image = calloc(size, 1);
    if (!image) return -1;

    // Simple red sphere
    for (int y = 0; y < cfg->height; y++) {
        for (int x = 0; x < cfg->width; x++) {
            double u = (double)x / (cfg->width - 1);
            double v = (double)y / (cfg->height - 1);
            double dx = 2*u - 1, dy = -(2*v - 1), dz = -1;
            double t = dx*dx + dy*dy + dz*dz - 0.25;
            uint8_t r = (t > 0) ? 255 : 0;
            size_t idx = (y * cfg->width + x) * 4;
            image[idx] = r; image[idx+1] = 0; image[idx+2] = 0; image[idx+3] = 255;
        }
    }

    int result = save_png(cfg->output, cfg->width, cfg->height, image);
    free(image);

    if (result != 0) {
        log_error("Failed to save PNG (code: %d)", result);
        return result;
    }

    log_info("Saved to %s", cfg->output);
    return 0;
}
