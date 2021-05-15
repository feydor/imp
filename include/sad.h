/* sad.h */
#ifndef SAD_H
#define SAD_H

#include <stddef.h> /* for size_t */

/* forward declaration */
struct saru_bytemat;

/* internal struct to hold temporary results */
struct sad_result{
  int sad;
  size_t frow;
  size_t fcol;
};

/* interface */
struct sad_result c_sad(struct saru_bytemat *template, struct saru_bytemat *frame);

#endif
