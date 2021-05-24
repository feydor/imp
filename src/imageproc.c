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
static void bayer_sqrmat(int32_t *mat, size_t dim);
static int32_t reverse(uint32_t x);
static uint32_t interleave(uint16_t x, uint16_t y);
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

    int32_t thresholds[] = { 256/4, 256/4, 256/4 };

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

            // TODO: Verify these calculations
            /*
            color = color ^ ((color ^ g) & 0x00FF00);
            r = R_FROM_PXL(color) + factor * thresholds[0];
            g = G_FROM_PXL(color) + factor * thresholds[1];
            b = B_FROM_PXL(color) + factor * thresholds[2];

            color = 0;
            color = color ^ ((color ^ r) & 0xFF0000);
            color = color ^ ((color ^ g) & 0x00FF00);
            color = color ^ ((color ^ b) & 0x0000FF);
            */
    
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
                    diff(r, (color & 0xFF0000) >> 16),
                    diff(g, (color & 0x00FF00) >> 8),
                    diff(b, (color & 0x0000FF))
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
    return x + y * image->w / PXLSIZE;
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

static int32_t
reverse(uint32_t x) {
   uint32_t y = x; // y holds the result; 
   int32_t s = sizeof(y) * CHAR_BIT - 1; // extra shift needed at end
   for (x >>= 1; x; x >>= 1) {
       y <<=1;
       y |= x & 1;
       s--;
   }
   return y << s; 
}

/**
 * returns an unsigned number of size 2n bits
 * which is composed of the bits of x and y interleaved together
 * starting with the first bit of x
 */
static uint32_t
interleave(uint16_t x, uint16_t y)
{
    uint32_t z = 0;
    for (size_t i = 0; i < sizeof(x) * CHAR_BIT; ++i) { 
        z |= (x & 1U << i) << i | (y & 1U << i) << (i + 1);
    }
    return z;
}
