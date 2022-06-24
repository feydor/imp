/* image.c - image/buffer processing algorithms */
#ifndef IMAGE_H
#define IMAGE_H

typedef unsigned char uchar;

void invert(uchar *buf, size_t size);
void add_uniform_bernoulli_noise(uchar *buf, size_t size_bytes);
void grayscale(uchar *buf, size_t size_bytes);

#endif