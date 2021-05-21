/* sad.c */
#include "../include/sad.h"
#include <limits.h> /* for INT_MIN, INT_MAX */
#include <stddef.h> /* for size_t */
#include <string.h> /* for memset */
#include <stdlib.h> /* for abs */
#include "saru-bytebuf.h"

/* static function prototypes */
static int do_sad_calculation(struct saru_bytemat *frame, struct saru_bytemat *template);
static int sum(size_t height, size_t width, int arr[][width]);
static int are_empty(unsigned char *buf1, unsigned char *buf2);
static struct sad_result min_sad(struct sad_result *results, int len); 

/**
 * function: c_sad, calculates the sum of absolute differences (SAD)
 *           between the frame (the larger buffer) and the template 
 *           (the smaller one)
 * returns: the minimum SAD value, 
 *          frame->col and frame->row contain the location of min SAD value
 * notes: 1. template must be a square matrix (width = height). 
 *        2. template must 'fit' frame, (template->width <= frame->height, etc)
 *           otherwise it returns INT_MIN.
 *        3. begins on upper left corner of frame.
 */
struct sad_result
c_sad(struct saru_bytemat *template, struct saru_bytemat *frame) 
{
  if (are_empty(template->buf, frame->buf) || 
      !sbm_injective(template, frame)) {
      struct sad_result err;
      err.sad = INT_MIN;
      err.frow = 0;
      err.fcol = 0;
      return err;
  }

  const int NUM_ITERATIONS = (frame->wid - template->wid + 1) *
    (frame->hgt - template->hgt + 1);

  struct sad_result results[NUM_ITERATIONS];
  memset(results, 0, sizeof(results));

  // iterate through frame, doing SAD calculation where possible
  int nresults = 0;
  for (frame->row = 0; frame->row < frame->hgt; frame->row++) {
    for (frame->col = 0; frame->col < frame->wid; frame->col++) {
      if (sbm_subinjective(template, frame)) {
        struct sad_result res;
        res.frow = frame->row;
        res.fcol = frame->col;
        res.sad = do_sad_calculation(frame, template);
        results[nresults++] = res;
      }
    }
  }

  /* return the smallest sad result and its coordinates */
  return min_sad(results, nresults);
}

static int 
do_sad_calculation(struct saru_bytemat *frame, struct saru_bytemat *template) 
{
  // iterate template and the "overlapped" portion of the frame
  int temp[template->hgt][template->wid];
  for (size_t trow = 0, frow = frame->row; trow < template->hgt; trow++, frow++) {
    for (size_t tcol = 0, fcol = frame->col; tcol < template->wid; tcol++, fcol++) {
      temp[trow][tcol] = abs(
          frame->buf[frow * frame->wid + fcol] - template->buf[trow * template->wid + tcol]
          );
    }
  }
  
  return sum(template->hgt, template->wid, temp);
}

static int
are_empty(unsigned char *buf1, unsigned char *buf2) 
{
  return !buf1 || !buf2;
}

/**
 * returns the sum of the elements in arr
 */
static int 
sum(size_t height, size_t width, int arr[][width]) 
{
  int sum = 0;
  for (size_t row = 0; row < height; row++) {
    for (size_t col = 0; col < width; col++) {
      sum += arr[row][col];
    }
  }
  return sum;
}

/**
 * returns the results where the minimum sad value is held
 */
static struct sad_result
min_sad(struct sad_result *results, int len) 
{
    int min = INT_MAX;
    int min_idx = 0;
    struct sad_result *rp = results;
    for (int i = 0; i < len; i++, rp++) {
        if (min > rp->sad) {
            min = rp->sad;
            min_idx = i;
        }
    }
    return results[min_idx];
}

