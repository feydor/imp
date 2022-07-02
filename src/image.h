/* image.c - image/buffer processing algorithms */
#ifndef IMAGE_H
#define IMAGE_H
#include <stdint.h>
#include <stddef.h>

typedef unsigned char uchar;

/** all functions are format agnostic, ie the inputs are treated as raw bytes */
void invert(uchar *buf, size_t size);

/** assumes seeded rand */
void add_uniform_bernoulli_noise(uchar *buf, size_t size_bytes);
void grayscale(uchar *buf, size_t size_bytes);

/** tones must be in RGB, MSB format */
void two_tone(uchar *buf, size_t bytes, uint32_t tone1, uint32_t tone2);
void ordered_dithering_triple_channel(uchar *buf, size_t size_bytes, size_t width_pixels, uint32_t *palette, size_t palette_size);
void ordered_dithering_single_channel(uchar *buf, size_t size_bytes, size_t width_pixels, uint32_t *palette, size_t palette_size);
void palette_quantization(uchar *buf, size_t size_bytes, uint32_t *palette_buf, size_t palette_buf_size);
#endif
