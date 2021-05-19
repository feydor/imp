/* imageproc.h - functions for image processing */
#ifndef IMAGEPROC_H
#define IMAGEPROC_H

#include <stddef.h> /* for size_t */

#define uchar unsigned char

struct pixel {
    uchar r;
    uchar g;
    uchar b;
};

struct image {
    struct pixel *buf;
    size_t w;
    size_t h;
};

/* function prototypes */
int ordered_dithering(unsigned char *src, size_t w, size_t h);

#endif
