#ifndef FILEIO_H
#define FILEIO_H

#include <stdint.h>

/**
 * save rgba image as PNG file
 * @param filename output file path
 * @param width image width
 * @param height image heigth
 * @param image RGBA pixel data（size = width * height * 4）
 * @return 0 success，none 0 failure
 */
int save_png(const char* filename, uint32_t width, uint32_t height,
             const uint8_t* image);

#endif // FILEIO_H
