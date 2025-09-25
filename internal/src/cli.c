#include "cli.h"
#include "config.h"
#include <argtable3.h>
#include <stdio.h>
#include <stdlib.h>

int parse_args(int argc, char **argv, rtCfg* cfg) {
    struct arg_lit* help = arg_lit0("h", "help", "print this help and exit");
    struct arg_int *width = arg_int1("w", "width", "<int>", "Image width (default: 800)");
    struct arg_int *height = arg_int1("H", "height", "<int>", "Image height (default: 600)");
    struct arg_str *output = arg_str1("o", "output", "<file>", "Output PNG file");
    struct arg_lit *verbose = arg_lit0("v", "verbose", "Enable verbose logging");
    struct arg_end *end = arg_end(20);

    void* argtable[] = {help, width, height, output, verbose, end};
    const char* progname = "raytracer";
    int errors = arg_parse(argc, argv, argtable);

    if (help->count > 0 || 1 == argc) {
        printf("Ray Tracer - A simple path tracer\n");
        printf("Usage: %s", progname);
        arg_print_syntax(stdout, argtable, "\n");
        printf("\n");
        arg_print_glossary(stdout, argtable, "  %-30s %s\n");
        printf("\nExample:\n");
        printf("  %s -s scene.json -o output.png -w 1920 -t 1080 -n 500\n", progname);
        exit(0);
    }

    if (errors > 0) {
        arg_print_errors(stderr, end, "raytracer");
        arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
        return 1;
    }

    cfg->width = width->count ? *width->ival : 800;
    cfg->height = height->count ? *height->ival : 600;
    cfg->output = output->count ? output->sval[0] : "output.png";
    cfg->verbose = verbose->count;

    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}
