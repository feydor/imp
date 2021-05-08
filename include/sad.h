/* sad.h */
#ifndef SAD_H
#define SAD_H

#include <limits.h>

typedef struct {
  unsigned char *buffer;
  unsigned int   width;
  unsigned int   height;
  unsigned int   col;
  unsigned int   row;
  unsigned int   size;
} UCharBuffer;

/* interface */
int c_sad(UCharBuffer *template, UCharBuffer *frame);
UCharBuffer *create_UCharBuffer(unsigned int width, unsigned int height);
UCharBuffer *create_UCharBuffer_from_uchar(unsigned char *buff, unsigned int width, unsigned int height);
void destroy_UCharBuffer(UCharBuffer *b);
void print_UCharBuffer(UCharBuffer *b);

#endif
