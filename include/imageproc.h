/* imageproc.h - functions for image processing */
#ifndef IMAGEPROC_H
#define IMAGEPROC_H

#include <stddef.h> /* for size_t */

#define uchar unsigned char

struct pixel_t {
    uchar r;
    uchar g;
    uchar b;
};

struct image_t {
    struct pixel_t *buf;
    size_t w;
    size_t h;
};

/* function prototypes */
int ordered_dithering(struct image_t *image);
size_t index_at(const struct image_t *image, size_t x, size_t y);
int setpixel(struct image_t *image, size_t x, size_t y, 
             struct pixel_t *pixel);

#define PIXEL_FROM_INDEX(image, i) image->buf[i];

#endif
