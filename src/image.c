/* image.c - image/buffer processing algorithms */
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "image.h"

static int max(int a, int b) {
   return a > b ? a : b;
}

static int min(int a, int b) {
   return a < b ? a : b;
}

// random int in range [lower, upper]
static int random_int(int lower, int upper) {
   assert(lower < upper);
   return lower + (rand() % upper + 1);
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

// assumes seeded rand
void add_uniform_bernoulli_noise(uchar *buf, size_t size_bytes) {
   size_t adjusted_end = size_bytes - (size_bytes % 3);
   for (size_t i = 0; i < adjusted_end; i += 3) {
      int r = 77;
      int result = bernoulli_trial() ? r : -r;
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
