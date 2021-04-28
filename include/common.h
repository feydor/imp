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

typedef uint64_t LONG;    // LONG = unsigned 64 bit value
typedef uint32_t DWORD;   // DWORD = unsigned 32 bit value
typedef uint16_t WORD;    // WORD = unsigned 16 bit value
typedef uint8_t  BYTE;    // BYTE = unsigned 8 bit value

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

