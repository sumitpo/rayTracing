#include "fileio.h"
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int save_png(const char* filename, uint32_t width, uint32_t height,
             const uint8_t* image) {
  if (!filename || !image || width == 0 || height == 0) {
    return -1;
  }

  FILE* fp = fopen(filename, "wb");
  if (!fp) {
    return -2;
  }

  png_structp png_ptr =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    fclose(fp);
    return -3;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_write_struct(&png_ptr, NULL);
    fclose(fp);
    return -4;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
    return -5;
  }

  png_init_io(png_ptr, fp);
  png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGBA,
               PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png_ptr, info_ptr);

  // 写入每一行
  png_bytep row = (png_bytep)malloc(width * 4);
  if (!row) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
    return -6;
  }

  for (uint32_t y = 0; y < height; y++) {
    memcpy(row, image + y * width * 4, width * 4);
    png_write_row(png_ptr, row);
  }

  free(row);
  png_write_end(png_ptr, NULL);
  png_destroy_write_struct(&png_ptr, &info_ptr);
  fclose(fp);
  return 0;
}
