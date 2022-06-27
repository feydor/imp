/* image.c - image/buffer processing algorithms */
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "image.h"
#define rgb_red(rgb) ((rgb & 0xFF0000) >> 16)
#define rgb_green(rgb) ((rgb & 0x00FF00) >> 8)
#define rgb_blue(rgb) (rgb & 0x0000FF)

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

static double distance_rgb(int32_t tone1, int32_t tone2) {
   return sqrt( pow(abs(rgb_red(tone1) - rgb_red(tone2)), 2) +
                pow(abs(rgb_green(tone1) - rgb_green(tone2)), 2) +
                pow(abs(rgb_blue(tone1) - rgb_blue(tone2)), 2) );
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

void two_tone(uchar *buf, size_t bytes, uint32_t tone1, uint32_t tone2) {
   size_t adjusted_end = bytes - (bytes % 3);
   for (size_t px = 0; px < adjusted_end; px += 3) {
      uint32_t pixel = 0;
      pixel = (buf[px + 2] << 16) | (buf[px + 1] << 8) | buf[px];
      double dist1 = distance_rgb(tone1, pixel);
      double dist2 = distance_rgb(tone2, pixel);

      if (dist1 > dist2) {
         buf[px + 2] = rgb_red(tone1);
         buf[px + 1] = rgb_green(tone1);
         buf[px] = rgb_blue(tone1);
      } else {
         buf[px + 2] = rgb_red(tone2);
         buf[px + 1] = rgb_green(tone2);
         buf[px] = rgb_blue(tone2);
      }
   }
}

// compare the RGb components of both colors
static void nearest_palette_color(uint32_t *palette, size_t palette_size, uchar *red, uchar *green, uchar* blue) {
    assert(red && green && blue && palette);

    // find the color in the palette that is 'closest' to the input
    // use euclidean RGB distances to determine closeness
    int32_t closest_color = 0;
    double distance = 0.0, min = DBL_MAX;
    for (size_t i = 0; i < palette_size; ++i) {
        distance = sqrt( pow(abs((int)rgb_blue(palette[i]) - *blue), 2) +
                         pow(abs((int)rgb_green(palette[i]) - *green), 2) +
                         pow(abs((int)rgb_red(palette[i]) - *red), 2) );

        if (distance < min) {
            min = distance;
            closest_color = palette[i];
        }
    }
    *red = rgb_red(closest_color);
    *green = rgb_green(closest_color);
    *blue = rgb_blue(closest_color);
}

void ordered_dithering_triple_channel(uchar *buf, size_t size_bytes, size_t width_pixels, uint32_t *palette, size_t palette_size) {
   int matrix_dim = 4;
   float bayer_matrix[4][4] = {
      { 0.0f, 8.0f, 2.0f, 10.0f },
      { 12.0f, 4.0f, 14.0f, 6.0f },
      { 3.0f, 11.0f, 1.0f, 9.0f },
      { 15.0f, 7.0f, 13.0f, 5.0f }
   };
   // BMP supports 2^16 colors
   int N = 4;
   float spread = 256.0f/N;
   // precompute threshold maps and normalize
   for (int r = 0; r < matrix_dim; ++r) {
      for (int c = 0; c < matrix_dim; ++c) {
         bayer_matrix[r][c] = (bayer_matrix[r][c] / 16.0f) - 0.5f;
      }
   }

   int pixel_count = 0;
   size_t adjusted_end = size_bytes - (size_bytes % 3);
   for (size_t px = 0; px < adjusted_end; px += 3, ++pixel_count) {
      int x = pixel_count % width_pixels;
      int y = pixel_count / width_pixels;
      
      uchar new_red = buf[px+2] + spread * bayer_matrix[y % matrix_dim][x % matrix_dim];
      uchar new_green = buf[px+1] + spread * bayer_matrix[y % matrix_dim][x % matrix_dim];
      uchar new_blue = buf[px] + spread * bayer_matrix[y % matrix_dim][x % matrix_dim];
      
      nearest_palette_color(palette, palette_size, &new_red, &new_green, &new_blue);

      buf[px + 2] = new_red;
      buf[px + 1] = new_green;
      buf[px] = new_blue;
   }
}

/**
 * ordered dithering with Bayer matrices
 * eq: color' = nearest_palette_color(color + r * (M(x % n, y % n) - 1/2))
 * color = 3 byte triple of red, green, blue
 * r = 256 / N (given an RGB palette with 2^3*N evenly distanced colors)
 * M = threshold map
 * (1/2 is the normalizing term)
 */
void ordered_dithering_single_channel(uchar *buf, size_t size_bytes, size_t width_pixels, uint32_t *palette, size_t palette_size) {
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
    float spread = 256/N;

    // precompute threshold maps and normalize
    for (int r = 0; r < matrix_dim; ++r) {
        for (int c = 0; c < matrix_dim; ++c) {
            bayer_matrix[r][c] = (bayer_matrix[r][c] / 16.0f) - 0.5f;
        }
    }

    int pixel_count = 0;
    size_t adjusted_end = size_bytes - (size_bytes % 3);
    for (size_t px = 0; px < adjusted_end; px += 3, ++pixel_count) {
        int x = pixel_count % width_pixels;
        int y = pixel_count / width_pixels;
        
        unsigned int color = 0, new_color = 0;
        color = (buf[px + 2] << 16) | (buf[px + 1] << 8) | buf[px]; // temp conversion to rgb
        new_color = color + spread * bayer_matrix[y % matrix_dim][x % matrix_dim];
        
        uchar new_red = rgb_red(new_color);
        uchar new_green = rgb_green(new_color);
        uchar new_blue = rgb_blue(new_color);
        nearest_palette_color(palette, palette_size, &new_red, &new_green, &new_blue);
        buf[px + 2] = new_red;
        buf[px + 1] = new_green;
        buf[px] = new_blue;
    }
}

void palette_quantization(uchar *buf, size_t size_bytes, uint32_t *palette_buf, size_t palette_buf_size) {
   size_t adjusted_end = size_bytes - (size_bytes % 3);
   for (size_t px = 0; px < adjusted_end; px += 3) {
      nearest_palette_color(palette_buf, palette_buf_size, &buf[px + 2], &buf[px + 1], &buf[px]);
   }
}
