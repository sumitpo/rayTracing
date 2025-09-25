#ifndef FILEIO_H
#define FILEIO_H

#include <stdint.h>

/**
 * 保存 RGBA 图像为 PNG 文件
 * @param filename 输出文件路径
 * @param width 图像宽度
 * @param height 图像高度
 * @param image RGBA 像素数据（size = width * height * 4）
 * @return 0 成功，非 0 失败
 */
int save_png(const char* filename, uint32_t width, uint32_t height, const uint8_t* image);

#endif // FILEIO_H
