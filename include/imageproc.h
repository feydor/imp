/* imageproc.h - functions for image processing */
#ifndef IMAGEPROC_H
#define IMAGEPROC_H

#include <stddef.h> /* for size_t */
#include <inttypes.h> /* for int32_t */

#define uchar unsigned char
#define PXLSIZE 4

struct image32_t {
    int32_t *buf; /* a buffer of 32b size pixels (2's compliment)
                   * always treated as positive
                   * format: 0xXXRRGGBB (XX - unused, RR = red, 
                   *                     GG = green, BB = blue)
                   */ 
    size_t w; /* in bytes 
               * guaranteed to be a multiple of 4, at least on bmp...
               */
    size_t h; /* in bytes/pixels */
};

/* function prototypes */
int ordered_dithering(struct image32_t *image);
void bayer_sqrmat(int32_t *mat, size_t dim);
int pixel_at(const struct image32_t *image, size_t x, size_t y);
int setpixel(struct image32_t *image, int32_t pixel, size_t x, size_t y);
int32_t closestfrompal(int32_t color, int32_t *pal, size_t size);
int32_t swapbytes(uint32_t a, unsigned i, unsigned j);

#define WPXLS_FROM_WBYTES(wbytes) ( wbytes / 4 )
#define PXL_FROM_IDX(image, i) ( image->buf[i] )
#define R_FROM_PXL(pixel) ( (pixel & 0xFF0000) >> 16 )
#define G_FROM_PXL(pixel) ( (pixel & 0x00FF00) >> 8 )
#define B_FROM_PXL(pixel) ( pixel & 0x0000FF )
#define IMGSIZE(imagep) { (imagep->w * imagep->h) / 4 }

#endif
