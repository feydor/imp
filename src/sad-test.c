/* sadx64.c */
#include "../include/common.h"
#include "../include/bmp.h"

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
#define FRAME_HEIGHT 10
#define FRAME_WIDTH 20

// Link this program with an external C or x86-64 compression function
extern int sad(uint64_t* template, uint64_t starting_row,
	uint64_t starting_col, uint64_t* frame, uint64_t f_height, uint64_t f_width);
static int self_test(void);
static void run_benchmarks();

int run_sad(options_t *options) {
    if (!options) {
      errno = EINVAL;
      return EXIT_FAILURE;
      /* NOTREACHED */
    }
    
    /*
    if (!options->input || !options->output) {
      errno = ENOENT;
      return EXIT_FAILURE;
    }
    */
   
    if (!self_test()) {
		printf("Self test failed!\n");
		return EXIT_FAILURE;
        /* NOTREACHED */
	}
	
	printf("Self test passed!\n");
    
    if (options->input) {
        if(!parse_24bit_bmp(options->input)) {
            errno = ENOENT;
            return EXIT_FAILURE;
            /* NOTREACHED */
        }
    }
	
	// do benchmarking
    run_benchmarks();
	
	return EXIT_SUCCESS;
}

void run_benchmarks() {
    const long TRIALS = 300000;
	time_t start;
	
	uint64_t frame [FRAME_HEIGHT * FRAME_WIDTH];
	uint64_t template [3 * 3];
	
	// fill both frame and template with random bytes
	memcpy(frame, (void*) memcpy, FRAME_HEIGHT * FRAME_WIDTH * 8);
	memcpy(template, (void*) memcpy, 3 * 3 * 8);
	
    printf("Printing frame...\n");
	for (int i = 0; i < FRAME_HEIGHT * FRAME_WIDTH; ++i) {
		printf("%li, ", frame[i]);
	}
	printf("\n");
	
	start = clock();
	for (long i = 0; i < TRIALS; i++) {
		sad(template, 0, 0, frame, FRAME_HEIGHT, FRAME_WIDTH);
	}
	printf("Printing benchmarks...\n");
	printf("Speed (C)  : %.1f MB/s\n", (double)(FRAME_HEIGHT * FRAME_WIDTH) * TRIALS / (clock() - start) * CLOCKS_PER_SEC / 1000000);
	
	//~ start = clock();
	//~ for (long i = 0; i < TRIALS; i++)
		//~ rc4_encrypt_x86(&state, msg, MSG_LEN);
	//~ printf("Speed (x86): %.1f MB/s\n", (double)MSG_LEN * TRIALS / (clock() - start) * CLOCKS_PER_SEC / 1000000);
}

int self_test(void) {
	// testcase 1: "wikipedia example"
	/*
	 * Template    Search image
		2 5 5       2 7 5 8 6
		4 0 7       1 7 4 2 7
		7 5 9       8 4 6 8 5	 
	 */
	
	 uint64_t frame[] = {
		2, 7, 5, 8, 6,
		1, 7, 4, 2, 7,
		8, 4, 6, 8, 5
	 };
	 uint64_t template[] = {
		2, 5, 5,
		4, 0, 7,
		7, 5, 9
	 };
     
     int res = 0;
     res = sad(template, 0, 0, frame, 3, 5);
     printf("%d\n", res);
     assert(20 == res);

	return 1;
}
