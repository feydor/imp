/* imageproc.c - functions for image processing */
#include <assert.h> /* for assert */
#include <errno.h> /* for errno */
#include <limits.h> /* for UINT_MAX */ 
#include <math.h> /* for sqrt */
#include <stdint.h> /* for uint16_t */
#include <stdlib.h> /* for malloc, exit */
#include <stdio.h> /* for printf */
#include "../include/imageproc.h"
#include "../include/bmp.h"

extern int errno; /* these functions set errno on errors */

/* static function protoypes */
static void bayer_sqrmat(unsigned *mat, size_t dim);
static unsigned reverse(unsigned x);
static unsigned interleave(unsigned x, unsigned y, size_t n);
static int32_t closestcolor(int32_t color);
static int32_t diff(int32_t a, int32_t b);
static int32_t sumofsquares(int32_t a, int32_t b, int32_t c);

/**
 * ordered dithering with Bayer matrices
 */
int ordered_dithering(struct image32_t *image)
{
    if (!image || !image->buf)
        return -1;

    unsigned thresholds[] = { 256/4, 256/4, 256/4 };

    /* using an 8x8 Bayer matrix */
    const size_t dim = 8*8;
    unsigned mat[dim];
    bayer_sqrmat(mat, 8);

    unsigned factor = 0, r = 0, g = 0, b = 0;
    int32_t color = 0;
    int32_t closest = 0;
    for (size_t y = 0; y < image->h; ++y)
        for (size_t x = 0; x < image->w; x += 4) {
            factor = mat[(x & 7) + ((y & 7) << 3)];
            color = PXL_FROM_IDX(image, index_at(image, x, y));

            // TODO: Verify these calculations
            r = R_FROM_PXL(color) + factor * thresholds[0];
            g = G_FROM_PXL(color) + factor * thresholds[1];
            b = B_FROM_PXL(color) + factor * thresholds[2];
    
            closest = closestcolor(color);
            setpixel(image, closest, x, y);
        }

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
bayer_sqrmat(unsigned *mat, size_t dim)
{
    printf(" X=%lu, Y=%lu:\n", dim, dim);
    for (size_t y = 0; y < dim; ++y) {
        printf("    ");
        for (size_t x = 0; x < dim; ++x) {
            unsigned xc = x ^ y, yc = y;
            unsigned res = 0; //, mask = dim - 1;
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
setpixel(struct image32_t *image, int32_t pixel, size_t x, size_t y)
{
    image->buf[index_at(image, x, y)] = pixel;
    return 1;
}

static int32_t
closestcolor(int32_t color)
{
    // Use euclidean RGB distance 
    int32_t pal[] = {
        0x000000, 0x008000, 0x00FF00,
        0x0000FF, 0x0080FF, 0x00FFFF,
        0x800000, 0x808000, 0x80FF00,
        0x8000FF, 0x8080FF, 0x80FFFF,
        0xFF0000, 0xFF8000, 0xFFFF00,
        0xFF00FF, 0xFF80FF, 0xFFFFFF
    };

    int32_t d = 0, min = INT32_MAX, res;
    int32_t r = 0, g = 0, b = 0;
    for (size_t i = 0; i < sizeof(pal); ++i) {
        r = (pal[i] & 0xFF0000) >> 16;
        g = (pal[i] & 0x00FF00) >> 8;
        b = pal[i] & 0x0000FF;
        d = sqrt(sumofsquares(
                    diff(r, R_FROM_PXL(color)),
                    diff(g, G_FROM_PXL(color)),
                    diff(b, B_FROM_PXL(color))
            ));
        if (d < min) {
           min = d;
           res = (r << 16 ) | (g << 8) | b;
        }
    }
    return res;
}

size_t
index_at(const struct image32_t *image, size_t x, size_t y)
{
    return x + y * WPXLS_FROM_WBYTES(image->w);
}

/*
 * starting from the left hand side, 
 * swaps the ith byte with the jth byte in a
 * example: swap the 0th byte with the 2nd byte
 * swap(a, 0, 2) => 0x00112233 => 0x22110033
 * returns: a byteswapped, -1 otherwise
 * NOTE:i and j must be less than 4
 *      a must be non-negative
 */
uint32_t
swapbytes(uint32_t a, unsigned i, unsigned j)
{
    if (i > 3 || j > 3)
        return -1;
    
    assert(a >= 0 && "Must not be a negative number.");

    unsigned temp = ((a >> CHAR_BIT*i) ^ (a >> CHAR_BIT*j)) & 
        ((1U << CHAR_BIT) - 1);
    return a ^ ((temp << CHAR_BIT*i) | (temp << CHAR_BIT*j));
}

static int32_t
diff(int32_t a, int32_t b)
{
    return (a > b) ? a - b : b - a;
}

static int32_t
sumofsquares(int32_t a, int32_t b, int32_t c)
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
