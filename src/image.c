/* image.c - image/buffer processing algorithms */
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "image.h"

static int max(int a, int b) {
   return a > b ? a : b;
}

static int min(int a, int b) {
   return a < b ? a : b;
}

static bool bernoulli_trial() {
   return rand() % 2 ? true : false;
}

// clamp val into range [lower, upper]
static int clamp(int val, int lower, int upper) {
   return max(lower, min(upper, val));
}

static int rgb_to_gray(uchar r, uchar g, uchar b) {
   return (int) (0.2989 * r + 0.5870 * g + 0.1140 * b);
}

void invert(uchar *buf, size_t size) {
   for (size_t i = 0; i < size; ++i)
      buf[i] = 255 - buf[i];
}

void add_uniform_bernoulli_noise(uchar *buf, size_t size_bytes) {
   size_t adjusted_end = size_bytes - (size_bytes % 3);
   for (size_t i = 0; i < adjusted_end; i += 3) {
      int difference = 50;
      int result = bernoulli_trial() ? difference : -difference;
      buf[i + 2] = clamp(buf[i + 2] + result, 0, 255);
      buf[i + 1] = clamp(buf[i + 1] + result, 0, 255);
      buf[i] = clamp(buf[i] + result, 0, 255);
   }
}

void grayscale(uchar *buf, size_t size_bytes) {
   size_t adjusted_end = size_bytes - (size_bytes % 3);
   for (size_t px = 0; px < adjusted_end; px += 3) {
      uchar gray = rgb_to_gray(buf[px + 2], buf[px + 1], buf[px]);
      buf[px + 2] = gray;
      buf[px + 1] = gray;
      buf[px] = gray;
   }
}

void nearest_palette_color(int *red, int *green, int* blue) {
    assert(red && green && blue);
    
    int32_t palette[] = {
        0x000000, 0x008000, 0x00FF00,
        0x0000FF, 0x0080FF, 0x00FFFF,
        0x800000, 0x808000, 0x80FF00,
        0x8000FF, 0x8080FF, 0x80FFFF,
        0xFF0000, 0xFF8000, 0xFFFF00,
        0xFF00FF, 0xFF80FF, 0xFFFFFF
    };

    // find the color in the palette that is 'closest' to the input
    // use euclidean RGB distances to determine closeness
    int32_t r = 0, g = 0, b = 0;
    int32_t distance = 0, closest_color = 0, min = INT32_MAX;
    int palette_size = sizeof(palette) / sizeof(palette[0]);
    for (int i = 0; i < palette_size; ++i) {
        b = (palette[i] & 0xFF0000) >> 16;
        g = (palette[i] & 0x00FF00) >> 8;
        r = palette[i] & 0x0000FF;
        
        distance = sqrt( pow(abs(b - *blue), 2) +
                         pow(abs(g - *green), 2) +
                         pow(abs(r - *red), 2) );
        if (distance < min) {
            min = distance;
            closest_color = palette[i];
        }
    }

    *red = closest_color & 0x0000FF;
    *green = (closest_color & 0x00FF00) >> 8;
    *blue = (closest_color & 0xFF0000) >> 16;
}

/**
 * ordered dithering with Bayer matrices
 * eq: color' = nearest_palette_color(color + r * (M(x % n, y % n) - 1/2))
 * color = 3 byte triple of red, green, blue
 * r = 256 / N (given an RGB palette with 2^3*N evenly distanced colors)
 * M = threshold map
 * (1/2 is the normalizing term)
 */
void ordered_dithering(uchar *buf, size_t size_bytes, size_t width_pixels) {
    // From wikipedia article on ordered dithering: https://en.wikipedia.org/wiki/Ordered_dithering
    int matrix_dim = 4;
    float bayer_matrix[4][4] = {
        { 0, 8, 2, 10 },
        { 12, 4, 14, 6 },
        { 3, 11, 1, 9 },
        { 15, 7, 13, 5 }
    };
    // BMP supports 2^16 colors
    int N = 4;
    int spread = 256/N;

    // precompute threshold maps and normalize
    for (int r = 0; r < matrix_dim; ++r) {
        for (int c = 0; c < matrix_dim; ++c) {
            bayer_matrix[r][c] = (bayer_matrix[r][c] / 16.0f) - 0.5f;
        }
    }

    int pixel_count = 0;
    size_t adjusted_end = size_bytes - (size_bytes % 3);
    for (size_t px = 0; px < adjusted_end; px += 3) {
        int x = pixel_count % width_pixels;
        int y = pixel_count / width_pixels;

        // TODO: substitute nearest color in palette
        int new_red = buf[px + 2] + spread * (bayer_matrix[y % matrix_dim][x % matrix_dim]);
        int new_green = buf[px + 1] + spread * (bayer_matrix[y % matrix_dim][x % matrix_dim]);
        int new_blue = buf[px] + spread * (bayer_matrix[y % matrix_dim][x % matrix_dim]);

        nearest_palette_color(&new_red, &new_green, &new_blue);

        buf[px + 2] = new_red;
        buf[px + 1] = new_green;
        buf[px] = new_blue;
        pixel_count++;
    }
}
