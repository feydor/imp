/* image.c - image/buffer processing algorithms */
#ifndef IMAGE_H
#define IMAGE_H

typedef unsigned char uchar;

/** all functions are format agnostic, ie the inputs are treated as raw bytes */
void invert(uchar *buf, size_t size);

/** assumes seeded rand */
void add_uniform_bernoulli_noise(uchar *buf, size_t size_bytes);
void grayscale(uchar *buf, size_t size_bytes);
void ordered_dithering(uchar *buf, size_t size_bytes, size_t width_pixels);
void palette_quantization(uchar *buf, size_t size_bytes);
#endif