/* common.h */
#ifndef COMMON_H
#define COMMON_H

#include <assert.h> /* for assert */
#include <stdio.h> /* for FILE, fprint, fread, stdin, stdout, stderr */
#include <stdlib.h> /* for malloc, realloc */
#ifdef _MSC_VER
	#include <io.h>
	typedef __int16 int16_t;
	typedef unsigned __int16 uint16_t;
	typedef __int32 int32_t;
	typedef unsigned __int32 uint32_t;
	typedef __int64 int64_t;
	typedef unsigned __int64 uint64_t;
#else
	#include <unistd.h> /* for EXIT_FAILURE, EXIT_SUCCESS */
	#include <stdint.h> /* for uint32_t, uint64_t */
#endif
#include <errno.h> /* for external errno variable */
#include <string.h> /* for memcpy, memset, strlen */
#include <time.h> /* for clock(), CLOCKS_PER_SEC */
#include <math.h> /* for sqrt() */

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

