// include/config.h
#ifndef CONFIG_H
#define CONFIG_H

#define DEFAULT_WIDTH  800
#define DEFAULT_HEIGHT 600
#define DEFAULT_OUTPUT "output.png"

typedef struct {
  int         width;
  int         height;
  const char* output;
  int         samples;
  int         verbose;
  const char* obj_file;
  const char* mtl_file;
} rtCfg;

#endif // CONFIG_H
