/* sad.h */
#ifndef SAD_H
#define SAD_H

#include <limits.h>

/* forward declaration */
struct saru_bytemat;

/* internal struct to hold temporary results */
typedef struct {
  int sad;
  unsigned int frow;
  unsigned int fcol;
}Result;

/* interface */
int c_sad(struct saru_bytemat *template, struct saru_bytemat *frame);

#endif
