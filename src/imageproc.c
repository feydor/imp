/* imageproc.c - functions for image processing */
#include <assert.h> /* for assert */
#include <stdint.h> /* for uint16_t */
#include <stdlib.h> /* for malloc, exit */
#include <stdio.h> /* for printf */
#include <errno.h> /* for errno */
#include "../include/imageproc.h"
#include "../include/bmp.h"

extern int errno; /* these functions set errno on errors */

/* static function protoypes */
static void bayer_sqrmat(unsigned char *mat, size_t dim);
static unsigned reverse(unsigned x);
static unsigned interleave(unsigned x, unsigned y, size_t n);

/**
 * ordered dithering with Bayer matrices
 */
int ordered_dithering(unsigned char *src, size_t w, size_t h)
{
    unsigned pal[] = {
        0x000000, 0x008000, 0x00FF00,
        0x0000FF, 0x0080FF, 0x00FFFF,
        0x800000, 0x808000, 0x80FF00,
        0x8000FF, 0x8080FF, 0x80FFFF,
        0xFF0000, 0xFF8000, 0xFFFF00,
        0xFF00FF, 0xFF80FF, 0xFFFFFF
    };

    unsigned char thresholds[] = { 256/4, 256/4, 256/4 };

    /* using an 8x8 Bayer matrix */
    const size_t dim = 8*8;
    unsigned char mat[dim];
    bayer_sqrmat(mat, 8);

    /* TODO
    unsigned char *r = src;
    unsigned char *g = src + 1;
    unsigned char *b = src + 2;
    for (size_t y = 0; y < h; ++y)
        for (size_t x = 0; x < w; ++x) {
            unsigned char factor = bmat[(x & 7) + ((y & 7) << 3)];
            unsigned color = color_from_index(src, indexat(src, x, y));
            unsigned char r = color['r'] + factor * thresholds[0];
            unsigned char g = color['g'] + factor * thresholds[1];
            unsigned char b = color['b'] + factor * thresholds[2];

            setpx(src, x, y, closestcolor(src, r, g, b));
        }
    */

    return 1;
}

/**
 * returns a Bayer matrix of X width and Y height
 * Algorithm for assigning slot (x, y):
 * 1. Take two values: the y coordinate and the XOR of the x and y coordinates,
 * 2. Interleave their bits in reverse order,
 * 3. Floating point divide the result by N (x * y)
 */
static void
bayer_sqrmat(unsigned char *mat, size_t dim)
{
    printf(" X=%lu, Y=%lu:\n", dim, dim);
    for (size_t y = 0; y < dim; ++y) {
        printf("    ");
        for (size_t x = 0; x < dim; ++x) {
            unsigned char xc = x ^ y, yc = y;
            unsigned char res = 0, mask = dim - 1;
            res = reverse(interleave(xc, yc, dim));
            mat[x + y * dim] = res;
            printf("%4d", res);
        }
        printf(" |");
        if (y == 0)
            printf(" 1/%lu", dim * dim);
        printf("\n");
    }
}

/**
 * returns an array color[] which contains
 * 1. the unsigned char red at color[0],
 * 2. the unsigned char blue at color[1],
 * 3. the unsigned char green ar color[2]
 */
static unsigned char*
color_from_index(unsigned char* src, size_t i)
{
    unsigned char color[] = { 0, 0, 0 };

    // sanity check
    assert(i % 3 < 3);
    switch (i % 3) {
        case 0:
            /* index points to red */
            color[0] = src[i];
            color[1] = src[i+1];
            color[2] = src[i+2];
            break;
        case 1:
            /* index points to green */
            color[0] = src[i-1];
            color[1] = src[i];
            color[2] = src[i+1];
            break;
        case 2:
            /* index points to blue */
            color[0] = src[i-2];
            color[1] = src[i-1];
            color[2] = src[i];
            break;
    }
    return color;
}

static size_t
indexat(unsigned char *src, size_t x, size_t y)
{
    return (x & 7) + ((y & 7) << 3);
}

static unsigned
reverse(unsigned x) {
   unsigned y = 0;
   for (size_t i = 0; i < 7 ; ++i) {
       y |= x & 1; // copy the set bits into tmp
       x >>= 1; 
       y <<= 1;
   }
   return y;
}

/**
 * returns an unsigned number of size 2n bits
 * which is composed of the bits of x and y interleaved together
 * starting with the first bit of x
 */
static unsigned
interleave(unsigned x, unsigned y, size_t n)
{
    unsigned z = 0;
    for (size_t i = 0; i < n; ++i) { 
        z |= ((x & (1 << i)) << i);
        z |= ((y & (1 << i)) << (i + 1));
    }
    return z;
}
