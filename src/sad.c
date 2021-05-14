/* sad.c */
#include "../include/common.h"
#include "../include/sad.h"
#include "saru-bytebuf.h"

/* static function prototypes */
static int do_sad_calculation(struct saru_bytemat *frame, struct saru_bytemat *template);
static int sum(unsigned int height, unsigned int width, int arr[][width]);
static inline bool are_empty(unsigned char *buf1, unsigned char *buf2);
static inline int min_sad_index(Result *results, int len);

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
int 
c_sad(struct saru_bytemat *template, struct saru_bytemat *frame) 
{
  if (are_empty(template->buf, frame->buf) || 
      !sbm_injective(template, frame)) {
    return INT_MIN;
  }

  const int NUM_ITERATIONS = (frame->wid - template->wid + 1) *
    (frame->hgt - template->hgt + 1);

  Result results[NUM_ITERATIONS];
  memset(results, 0, NUM_ITERATIONS * sizeof(Result));

  // iterate through frame, doing SAD calculation where possible
  int res_count = 0;
  for (frame->row = 0; frame->row < frame->hgt; frame->row++) {
    for (frame->col = 0; frame->col < frame->wid; frame->col++) {
      if (sbm_subinjective(template, frame)) {
        Result res;
        res.frow = frame->row;
        res.fcol = frame->col;
        res.sad = do_sad_calculation(frame, template);
        results[res_count++] = res;
      }
    }
  }

  /* find the minimum sad and its corresponding row and col */
  int min_index = min_sad_index(results, res_count);
  Result min_sad_result = results[min_index]; 

  /* set row and col results into frame struct */
  frame->row = min_sad_result.frow;
  frame->col = min_sad_result.fcol;

  return min_sad_result.sad;
}

static int 
do_sad_calculation(struct saru_bytemat *frame, struct saru_bytemat *template) 
{
  // iterate template and the "overlapped" portion of the frame
  int temp[template->hgt][template->wid];
  for (unsigned int trow = 0, frow = frame->row; trow < template->hgt; trow++, frow++) {
    for (unsigned int tcol = 0, fcol = frame->col; tcol < template->wid; tcol++, fcol++) {
      temp[trow][tcol] = abs(
          frame->buf[frow * frame->wid + fcol] - template->buf[trow * template->wid + tcol]
          );
    }
  }
  
  return sum(template->hgt, template->wid, temp);
}

static bool 
are_empty(unsigned char *buf1, unsigned char *buf2) 
{
  return !buf1 || !buf2;
}

/**
 * returns the sum of the elements in arr
 */
static int 
sum(unsigned int height, unsigned int width, int arr[][width]) 
{
  int sum = 0;
  for (unsigned int row = 0; row < height; row++) {
    for (unsigned int col = 0; col < width; col++) {
      sum += arr[row][col];
    }
  }
  return sum;
}

/**
 * returns the index of results where the minimum sad value is held
 */
static int
min_sad_index(Result *results, int len) 
{
  int min = INT_MAX;
  int min_index = 0;
  Result *rp = results;
  for (int i = 0; i < len; i++) {
    if (min > rp->sad) {
      min = rp->sad;
      min_index = i;
    }
    rp++;
  }
  return min_index;
}

