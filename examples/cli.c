// cli.c
#include "cli.h"
#include <argtable3.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

// Define defaults as constants for reuse
#define DEFAULT_WIDTH  800
#define DEFAULT_HEIGHT 600
#define DEFAULT_OUTPUT "output.png"

int parse_args(int argc, char** argv, rtCfg* cfg) {
  // Use defaults in argtable help
  char width_help[64];
  char height_help[64];
  snprintf(width_help, sizeof(width_help), "Image width (default: %d)",
           DEFAULT_WIDTH);
  snprintf(height_help, sizeof(height_help), "Image height (default: %d)",
           DEFAULT_HEIGHT);

  // clang-format: off
  struct arg_lit* help   = arg_lit0("h", "help", "print this help and exit");
  struct arg_int* width  = arg_int1("w", "width", "<int>", width_help);
  struct arg_int* height = arg_int1("H", "height", "<int>", height_help);
  struct arg_str* output = arg_str1("o", "output", "<file>", "Output PNG file");
  struct arg_str* obj_file =
      arg_str1(NULL, "obj", "<file.obj>", "Input OBJ file (required)");
  struct arg_str* mtl_file =
      arg_str0(NULL, "mtl", "<file.mtl>", "Input MTL file (optional)");
  struct arg_lit* verbose = arg_lit0("v", "verbose", "Enable verbose logging");
  // clang-format: on

  struct arg_end* end = arg_end(20);

  void*       argtable[] = { help,     width,    height,  output,
                             obj_file, mtl_file, verbose, end };
  const char* progname   = "raytracer";
  int         errors     = arg_parse(argc, argv, argtable);

  if (help->count > 0 || argc == 1) {
    printf("Ray Tracer - A simple path tracer\n");
    printf("Usage: %s", progname);
    arg_print_syntax(stdout, argtable, "\n");
    printf("\n");
    arg_print_glossary(stdout, argtable, "  %-30s %s\n");
    printf("\nExample:\n");
    printf("  %s --obj scene.obj --mtl scene.mtl -o %s -w %d -H %d\n", progname,
           DEFAULT_OUTPUT, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    exit(0);
  }

  if (errors > 0) {
    arg_print_errors(stderr, end, "raytracer");
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 1;
  }

  // Assign values with defaults
  cfg->width    = width->count ? *width->ival : DEFAULT_WIDTH;
  cfg->height   = height->count ? *height->ival : DEFAULT_HEIGHT;
  cfg->output   = output->count ? output->sval[0] : DEFAULT_OUTPUT;
  cfg->obj_file = obj_file->count ? obj_file->sval[0] : NULL;
  cfg->mtl_file = mtl_file->count ? mtl_file->sval[0] : NULL;
  cfg->verbose  = verbose->count;

  if (!cfg->obj_file) {
    fprintf(stderr, "Error: --obj <file.obj> is required\n");
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 1;
  }

  arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
  return 0;
}
