/* sadx64.c */
#include <inttypes.h>
#include <sys/time.h>
#include <libgen.h> /* for basename() */
#include <time.h> /* for clock(), CLOCKS_PER_SEC */
#include <assert.h> /* for assert */
#include <stdio.h> /* for printf */
#include <stdlib.h> /* for free */
#include <errno.h> /* for errno */
#include <string.h> /* for memcpy */
#include "../include/sad-test.h"
#include "../include/bmp.h"
#include "../include/sad.h"

#include "saru-bytebuf.h"

// Link this program with an external C or x86-64 compression function
extern int sad(uint64_t* template, uint64_t starting_row,
	uint64_t starting_col, uint64_t* frame, uint64_t f_height, uint64_t f_width);

static void run_benchmarks();

static void
run_benchmarks() 
{
    const long TRIALS = 1000;
    struct timeval stop, start, stop1, start1;
	
	uint64_t frame [FRAME_HEIGHT * FRAME_WIDTH];
	uint64_t template [3 * 3];

    SBM_CREATE(fb, FRAME_WIDTH, FRAME_HEIGHT);
    SBM_CREATE(tb, 3, 3);
	
	// fill both frame and template with random bytes
	memcpy(frame, (void*) memcpy, FRAME_HEIGHT * FRAME_WIDTH * sizeof(uint64_t));
	memcpy(template, (void*) memcpy, 3 * 3 * sizeof(uint64_t));
    memcpy(fb->buf, (void*) memcpy, fb->len);
	memcpy(tb->buf, (void*) memcpy, tb->len);
	
    printf("Printing frame...\n");
    unsigned char *fbp = fb->buf;
	for (int i = 0; i < FRAME_HEIGHT * FRAME_WIDTH; ++i) {
		// printf("%li, ", frame[i]);
        printf("%u ", *(++fbp));
	}
	printf("\n");
	
	gettimeofday(&start, NULL);
	for (long i = 0; i < TRIALS; i++) {
		sad(template, 0, 0, frame, FRAME_HEIGHT, FRAME_WIDTH);
	}
	gettimeofday(&stop, NULL);
	
	gettimeofday(&start1, NULL);
    for (long i = 0; i < TRIALS; i++) {
        c_sad(tb, fb);
    }
   gettimeofday(&stop1, NULL);
	
	printf("Printing benchmarks...\n");
    printf("Naive Looping x86-64 Assembly SAD:\n");
    printf("Time (uS)  : %lu us\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec );
	printf("Speed (C)  : %.1f MB/s\n", (double)(FRAME_HEIGHT * FRAME_WIDTH) * TRIALS / (stop.tv_usec - start.tv_usec) * CLOCKS_PER_SEC / 1000000);
    
     printf("Naive Looping C SAD:\n");
     printf("Time (uS)  : %lu us\n", (stop1.tv_sec - start1.tv_sec) * 1000000 + stop1.tv_usec - start1.tv_usec );
     printf("Speed (C)  : %.1f MB/s\n", (double)(FRAME_HEIGHT * FRAME_WIDTH) * TRIALS / (stop1.tv_usec - start.tv_usec) * CLOCKS_PER_SEC / 1000000);
	
    sbm_destroy(fb);
    sbm_destroy(tb);
}

int sad_selftest(void) {
	// testcase 1: "wikipedia example"
	/*
	 * Template    Search image
		2 5 5       2 7 5 8 6
		4 0 7       1 7 4 2 7
		7 5 9       8 4 6 8 5	 
	 */
	
     unsigned char fb[] = {
         2, 7, 5, 8, 6,
         1, 7, 4, 2, 7,
         8, 4, 6, 8, 5
    };
     unsigned char tb[] = {
         2, 5, 5,
         4, 0, 7,
         7, 5, 9
     };

     // run c_sad
     SBM_WRAP(template, tb, 3, 3);
     SBM_WRAP(frame, fb, 5, 3);

     struct sad_result res = c_sad(template, frame);
     assert(17 == res.sad);
     assert(0 == res.frow);
     assert(2 == res.fcol);

     free(template);
     free(frame);

	 return 1;
}

