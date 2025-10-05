// include/config.h
#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
  int         width;
  int         height;
  const char* output;
  int         samples;
  int         verbose;
} rtCfg;

#endif // CONFIG_H
