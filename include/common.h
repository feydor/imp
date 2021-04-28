#ifndef COMMON_H
#define COMMON_H

#include <libgen.h> /* for basename() */
#include <stdio.h> /* for FILE, fprint, stdin, stdout, stderr */
#include <stdlib.h> /* for malloc, realloc */
#include <unistd.h> /* for EXIT_FAILURE, EXIT_SUCCESS */
#include <errno.h> /* for external errno variable */
#include <string.h> /* for memcpy, memset, strlen */
#include <getopt.h> /* for external optarg, opterr, optind, getopt() */
#include <stdint.h> /* for uint32_t, uint64_t */

/* datatypes */
typedef struct {
  int           verbose;
  uint32_t      flags;
  FILE         *input;
  FILE         *output;
} options_t;

/* function prototypes */
int  run_sad(options_t *options);

#endif

