/* imageproc.c - functions for image processing */
#include <assert.h> /* for assert */
#include <math.h> /* for sqrt */
#include <stdint.h> /* for uint16_t */
#include <stdlib.h> /* for malloc, exit */
#include <stdio.h> /* for printf */
#include <errno.h> /* for errno */
#include <limits.h> /* for UINT_MAX */ 
#include "../include/imageproc.h"
#include "../include/bmp.h"

extern int errno; /* these functions set errno on errors */

/* static function protoypes */
static void bayer_sqrmat(uchar *mat, size_t dim);
static unsigned reverse(unsigned x);
static unsigned interleave(unsigned x, unsigned y, size_t n);
static void closestcolor(struct pixel_t *color,
                                    struct pixel_t *res);
static uchar diff(uchar a, uchar b);
static unsigned sumofsquares(uchar a, uchar b, uchar c);

/**
 * ordered dithering with Bayer matrices
 */
int ordered_dithering(struct image_t *image)
{
    if (!image || !image->buf)
        return -1;

    uchar thresholds[] = { 256/4, 256/4, 256/4 };

    /* using an 8x8 Bayer matrix */
    const size_t dim = 8*8;
    uchar mat[dim];
    bayer_sqrmat(mat, 8);

    uchar factor = 0, r = 0, g = 0, b = 0;
    struct pixel_t color = { 0, 0, 0 };
    struct pixel_t closest = { 0, 0, 0 };
    for (size_t y = 0; y < image->h; ++y)
        for (size_t x = 0; x < image->w; ++x) {
            factor = mat[(x & 7) + ((y & 7) << 3)];
            color = PIXEL_FROM_INDEX(image, index_at(image, x, y));
            r = color.r + factor * thresholds[0];
            g = color.g + factor * thresholds[1];
            b = color.b + factor * thresholds[2];
    
            closestcolor(&color, &closest);
            setpixel(image, x, y, &closest);
        }

    return 1;
}

int
wrap_ucharbuf(struct image_t *image, uchar *buf, size_t size)
{
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
bayer_sqrmat(uchar *mat, size_t dim)
{
    printf(" X=%lu, Y=%lu:\n", dim, dim);
    for (size_t y = 0; y < dim; ++y) {
        printf("    ");
        for (size_t x = 0; x < dim; ++x) {
            uchar xc = x ^ y, yc = y;
            uchar res = 0; //, mask = dim - 1;
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

int 
setpixel(struct image_t *image, size_t x, size_t y, struct pixel_t *pixel)
{
    image->buf[index_at(image, x, y)] = *pixel;
    return 1;
}

static void
closestcolor(struct pixel_t *color, struct pixel_t *res)
{
    // Use euclidean RGB distance 
    uint32_t pal[] = {
        0x000000, 0x008000, 0x00FF00,
        0x0000FF, 0x0080FF, 0x00FFFF,
        0x800000, 0x808000, 0x80FF00,
        0x8000FF, 0x8080FF, 0x80FFFF,
        0xFF0000, 0xFF8000, 0xFFFF00,
        0xFF00FF, 0xFF80FF, 0xFFFFFF
    };

    unsigned d = 0, min = UINT_MAX;
    uchar r = 0, g = 0, b = 0;
    for (size_t i = 0; i < sizeof(pal); ++i) {
        r = (pal[i] & 0xFF0000) >> 4;
        g = (pal[i] & 0x00FF00) >> 2;
        b = pal[i] & 0x0000FF;
        d = sqrt(sumofsquares(
                    diff(r, color->r),
                    diff(g, color->g),
                    diff(b, color->b)
            ));
        if (d < min) {
           min = d;
           res->r = r;
           res->g = g;
           res->b = b;
        }
    }
}

size_t
index_at(const struct image_t *image, size_t x, size_t y)
{
    return x + y * image->w;
}

static uchar
diff(uchar a, uchar b)
{
    return (a > b) ? a - b : b - a;
}

static unsigned
sumofsquares(uchar a, uchar b, uchar c)
{
    return a*a + b*b + c*c;
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
