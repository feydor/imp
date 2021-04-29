/* common.h */
#ifndef COMMON_H
#define COMMON_H

#include <assert.h> /* for assert */
#include <libgen.h> /* for basename() */
#include <stdio.h> /* for FILE, fprint, fread, stdin, stdout, stderr */
#include <stdlib.h> /* for malloc, realloc */
#include <unistd.h> /* for EXIT_FAILURE, EXIT_SUCCESS */
#include <errno.h> /* for external errno variable */
#include <string.h> /* for memcpy, memset, strlen */
#include <getopt.h> /* for external optarg, opterr, optind, getopt() */
#include <stdint.h> /* for uint32_t, uint64_t */
#include <time.h> /* for clock(), CLOCKS_PER_SEC */
#include <math.h> /* for sqrt() */
#include <sys/time.h>

#define ERR_FOPEN_INPUT  "fopen(input, r)"
#define ERR_FOPEN_OUTPUT "fopen(output, w)"
#define ERR_FREAD_INPUT "fread failed"
#define ERR_SAD_EXIT "run_sad blew up"
#define ERR_NOT_BMP "filetype is not a bmp"
#define ERR_MALLOC_NULL "malloc returned null"
#define DEFAULT_PROGNAME "sadx64"

/* datatypes */
typedef struct {
  int           verbose;
  uint32_t      flags;
  char         *fname;
  FILE         *input;
  FILE         *output;
} options_t;

/* function prototypes */
int  run_sad(options_t *options);

#endif

