/* imageproc.c - functions for image processing */
#include <assert.h> /* for assert */
#include <errno.h> /* for errno */
#include <limits.h> /* for UINT_MAX */ 
#include <math.h> /* for sqrt */
#include <stdint.h> /* for uint16_t */
#include <stdlib.h> /* for malloc, exit, abs */
#include <stdio.h> /* for printf */
#include "../include/imageproc.h"
#include "../include/bmp.h"

extern int errno; /* these functions set errno on errors */

/* static function protoypes */
static void bayer_sqrmat(int32_t *mat, size_t dim);

/**
 * ordered dithering with Bayer matrices
 */
int ordered_dithering(struct image32_t *image)
{
    if (!image || !image->buf)
        return -1;

    int32_t thresholds[] = { 256/4, 256/4, 256/4 };
    int32_t pal[] = {
        0x000000, 0x008000, 0x00FF00,
        0x0000FF, 0x0080FF, 0x00FFFF,
        0x800000, 0x808000, 0x80FF00,
        0x8000FF, 0x8080FF, 0x80FFFF,
        0xFF0000, 0xFF8000, 0xFFFF00,
        0xFF00FF, 0xFF80FF, 0xFFFFFF
    };

    /* using an 8x8 Bayer matrix */
    const size_t dim = 8*8;
    int32_t mat[dim];
    bayer_sqrmat(mat, 8);

    /* iterate over */
    unsigned factor = 0, r = 0, g = 0, b = 0;
    int32_t color = 0;
    int32_t closest = 0;
    for (size_t y = 0; y < image->h; ++y)
        for (size_t x = 0; x < image->w / PXLSIZE; ++x) {
            factor = mat[(x & 7) + ((y & 7) << 3)];
            color = image->buf[index_at(image, x, y)];
            printf("color = 0x%08X\n", color);

            // TODO: Verify these calculations
            r = R_FROM_PXL(color) + factor * thresholds[0];
            g = G_FROM_PXL(color) + factor * thresholds[1];
            b = B_FROM_PXL(color) + factor * thresholds[2];

            color = INT32_MAX;
            color = color ^ ((color ^ r) & 0xFF0000);
            color = color ^ ((color ^ g) & 0x00FF00);
            color = color ^ ((color ^ b) & 0x0000FF);
    
            closest = closestfrompal(color, pal, sizeof(pal)/sizeof(pal[0]));
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
bayer_sqrmat(int32_t *mat, size_t dim)
{
    printf(" X=%lu, Y=%lu:\n", dim, dim);
    for (size_t y = 0; y < dim; ++y) {
        printf("    ");
        for (size_t x = 0; x < dim; ++x) {
            uint32_t xc = x ^ y, yc = y;
            uint32_t res = 0;
            uint32_t mask = 2;
            for(uint32_t bit = 0; bit < 6; --mask) {
                res |= ((yc >> mask) & 1) << bit++;
                res |= ((xc >> mask) & 1) << bit++;
            }
            // res = reverse(interleave(xc, yc));
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

int32_t
closestfrompal(int32_t color, int32_t *pal, size_t n)
{
    // Use euclidean RGB distance 
    int32_t d = INT32_MAX - 1, min = INT32_MAX, res = INT32_MAX;
    int32_t r = 0, g = 0, b = 0;
    for (size_t i = 0; i < n; ++i) {
        r = (pal[i] & 0xFF0000) >> 16;
        g = (pal[i] & 0x00FF00) >> 8;
        b = pal[i] & 0x0000FF;
        d = sqrt( pow(abs(r - ((color & 0xFF0000) >> 16)), 2) +
                  pow(abs(g - ((color & 0x00FF00) >> 8)), 2) +
                  pow(abs(b - (color & 0x0000FF)), 2)
            );

        if (d == 0)
            return (r << 16) | (g << 8) | b;

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
    return x + y * image->w/PXLSIZE;
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
int32_t
swapbytes(uint32_t a, unsigned i, unsigned j)
{
    if (i > 3 || j > 3)
        return -1;
    
    assert(a >= 0 && "Must not be a negative number.");

    unsigned temp = ((a >> CHAR_BIT*i) ^ (a >> CHAR_BIT*j)) & 
        ((1U << CHAR_BIT) - 1);
    return a ^ ((temp << CHAR_BIT*i) | (temp << CHAR_BIT*j));
}

