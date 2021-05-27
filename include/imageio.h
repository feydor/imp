/* imageio.h - functions for loading and writing images */
#ifndef IMAGEIO_H
#define IMAGEIO_H

/* function prototypes */
int get_image_size(const char *src, size_t *width, size_t *height);
int32_t * allocate_image_buf(size_t size);
int read_image(const char *src, int32_t *dest, size_t size);
int write_image(int32_t *img, char *src, char *dest, size_t size);
void free_image_buf(int32_t *image);

int widen(int32_t *dest, int8_t *src, size_t size);
int narrow(int8_t *dest, int32_t *src, size_t size);

#endif
