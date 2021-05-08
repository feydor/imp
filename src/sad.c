/* sad.c */
#include "../include/common.h"
#include "../include/sad.h"

/* internal struct to hold temporary results */
typedef struct {
  int sad;
  unsigned int frow;
  unsigned int fcol;
}Result;

/* static function prototypes */
static int do_sad_calculation(UCharBuffer *frame, UCharBuffer *template);
static bool are_empty(unsigned char *buf1, unsigned char *buf2);
static bool template_fits_frame(UCharBuffer *template, UCharBuffer *frame);
static bool bijection(UCharBuffer *template, UCharBuffer *frame);
static int min_sad_index(Result *results, int len);
static int sum(unsigned int height, unsigned int width, int arr[][width]);
/*
static int min(int *arr, int len);
static int min_vla(int height, int width, int arr[][width]);
static void print_arr(int *arr, int len);
static void print_arr_vla(int height, int width, int arr[][width]);
*/

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
c_sad(UCharBuffer *template, UCharBuffer *frame) {
  if (are_empty(template->buffer, frame->buffer) || 
      !template_fits_frame(template, frame)) {
    return INT_MIN;
  }

  const int NUM_ITERATIONS = (frame->width - template->width + 1) *
    (frame->height - template->height + 1);

  Result results[NUM_ITERATIONS];
  memset(results, 0, NUM_ITERATIONS * sizeof(Result));

  // printf("frame_h: %d\nframe_w: %d\n", frame_h, frame_w);
  // printf("template_h: %d\ntemplate_w: %d\n", template_h, template_w);

  // iterate through frame, doing SAD calculation where possible
  int res_count = 0;
  for (frame->row = 0; frame->row < frame->height; frame->row++) {
    for (frame->col = 0; frame->col < frame->width; frame->col++) {
      if (bijection(template, frame)) {
        Result res;
        res.frow = frame->row;
        res.fcol = frame->col;
        res.sad = do_sad_calculation(frame, template);
        results[res_count++] = res;
      }
    }
  }

  // print_arr(results, res_count);

  /* find the minimum sad and its corresponding row and col */
  int min_index = min_sad_index(results, res_count);
  Result min_sad_result = results[min_index]; 

  /* set row and col results into frame struct */
  frame->row = min_sad_result.frow;
  frame->col = min_sad_result.fcol;

  return min_sad_result.sad;
}

/**
 * function: create_UCharBuffer,
 * returns: a heap-allocated UCharBuffer pointer with its buffer initialized to all zeros
 * notes: must be destroyed by destroy_UCharBuffer
 */
UCharBuffer *create_UCharBuffer(unsigned int width, unsigned int height) {
  UCharBuffer *b = malloc( sizeof(UCharBuffer) );
  if (b) {
    b->buffer = malloc( sizeof(unsigned char) * width * height );
    if (b->buffer) {
      memset( b->buffer, 0, sizeof(unsigned char) * width * height );
    }
    b->width = width;
    b->height = height;
    b->col = b->row = 0;
    b->size = width * height;
  }
  return b;
}

/**
 * function: create_UCharBuffer_from_uchar,
 * returns: a heap-allocated UCharBuffer pointer with its buffer set to the passed-in buff param
 * notes: must be destroyed by destroy_UCharBuffer
 */
UCharBuffer *create_UCharBuffer_from_uchar(unsigned char *buff, unsigned int width, unsigned int height) {
  UCharBuffer *b = malloc( sizeof(UCharBuffer) );
  if (b) {
    b->buffer = buff;
    b->width = width;
    b->height = height;
    b->col = b->row = 0;
    b->size = width * height;
  }
  return b;
}

void destroy_UCharBuffer(UCharBuffer *b) {
  if (b->buffer) {
    free(b->buffer);
    if (b) 
      free(b);
  }
}

void 
print_UCharBuffer(UCharBuffer *b) 
{
  if (!b->buffer) return;
  unsigned char *bp = b->buffer;
  for (unsigned int i = 0; i < b->size; i++) {
    printf("%u ", *(bp++));
  }
}

static int 
do_sad_calculation(UCharBuffer *frame, UCharBuffer *template) {
  // iterate template and the "overlapped" portion of the frame
  int temp[template->height][template->width];
  for (unsigned int trow = 0, frow = frame->row; trow < template->height; trow++, frow++) {
    for (unsigned int tcol = 0, fcol = frame->col; tcol < template->width; tcol++, fcol++) {
      temp[trow][tcol] = abs(
          frame->buffer[frow * frame->width + fcol] - template->buffer[trow * template->width + tcol]
          );
    }
  }
  
  return sum(template->height, template->width, temp);
}

static bool 
are_empty(unsigned char *buf1, unsigned char *buf2) {
  return !buf1 || !buf2;
}

/**
 * checks if the dimensions of template are less than or equal to
 * the dimensions of frame
 */
static bool 
template_fits_frame(UCharBuffer *template, UCharBuffer *frame) {
  return template->width <= frame->width && template->height <= frame->height;
}

/**
 * checks if an overlap of the template's elements 
 * with the current frame's elements is a 1-to-1 relationship
 */
static bool 
bijection(UCharBuffer *template, UCharBuffer *frame) {
  return frame->col + template->width <= frame->width &&
         frame->row + template->height <= frame->height;
}

static int 
sum(unsigned int height, unsigned int width, int arr[][width]) {
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
min_sad_index(Result *results, int len) {
  int min = INT_MAX;
  int min_index = 0;
  for (int i = 0; i < len; i++) {
    if (min > results[i].sad) {
      min = results[i].sad;
      min_index = i;
    }
      min = results[i].sad;
  }
  return min_index;
}

/*
static int 
min(int *arr, int len) {
  int min = INT_MAX;
  for (int i = 0; i < len; i++) {
    if (min > arr[i]) 
      min = arr[i];
  }
  return min;
}

static int 
min_vla(int height, int width, int arr[][width]) {
  int min = INT_MAX;
  for (int row = 0; row < height; row++) {
    for (int col = 0; col < width; col++) {
      if (min > arr[row][col])
        min = arr[row][col];
    }
  }
  return min;
}

static void 
print_arr(int *arr, int len) {
  printf("Printing array...\n");
  for (int i = 0; i < len; i++) {
    printf("%d ", arr[i]);
  }
  printf("\n");
}

static void 
print_arr_vla(int height, int width, int arr[][width]) {
  for (int row = 0; row < height; row++) {
    for (int col = 0; col < width; col++) {
      printf("%d ", arr[row][col]);
      if (col == width - 1) printf("\n");
    }
  }
}
*/
