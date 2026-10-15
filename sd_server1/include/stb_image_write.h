/* stb_image_write - v1.16 - public domain
   writes out PNG/BMP/TGA/JPEG/TGA/PPM in one call
   see http://nothings.org/stb_image_write.h

   this is a single header file that implements a tiny image writing library.
   It supports PNG, BMP, TGA, JPEG, and PPM formats.
*/
#ifndef STB_IMAGE_WRITE_H
#define STB_IMAGE_WRITE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* user must define STB_IMAGE_WRITE_IMPLEMENTATION in one source file */

/* PNG write functions */
int stbi_write_png(const char *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
int stbi_write_png_to_func(void (*func)(void *context, void *data, int size), void *context, int w, int h, int comp, const void *data, int stride_in_bytes);

/* BMP write functions */
int stbi_write_bmp(const char *filename, int w, int h, int comp, const void *data);
int stbi_write_bmp_to_func(void (*func)(void *context, void *data, int size), void *context, int w, int h, int comp, const void *data);

/* TGA write functions */
int stbi_write_tga(const char *filename, int w, int h, int comp, const void *data);
int stbi_write_tga_to_func(void (*func)(void *context, void *data, int size), void *context, int w, int h, int comp, const void *data);

/* JPEG write functions */
int stbi_write_jpg(const char *filename, int w, int h, int comp, const void *data, int quality);
int stbi_write_jpg_to_func(void (*func)(void *context, void *data, int size), void *context, int w, int h, int comp, const void *data, int quality);

/* PPM write functions */
int stbi_write_ppm(const char *filename, int w, int h, int comp, const void *data);
int stbi_write_ppm_to_func(void (*func)(void *context, void *data, int size), void *context, int w, int h, int comp, const void *data);

#endif /* STB_IMAGE_WRITE_H */
