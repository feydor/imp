#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

/* Function prototypes */
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
#define FRAME_HEIGHT 10
#define FRAME_WIDTH 20

// Link this program with an external C or x86-64 compression function
extern int sad(uint64_t* template, uint64_t starting_row,
	uint64_t starting_col, uint64_t* frame, uint64_t f_height, uint64_t f_width);
int self_test(void);

/* Main function */
int main(void) {
	if (!self_test()) {
		printf("Self test failed!\n");
		return EXIT_FAILURE;
	}
	
	printf("Self test passed!\n");
	
	// do benchmarking
	const long TRIALS = 300000;
	time_t start;
	
	uint64_t frame [FRAME_HEIGHT * FRAME_WIDTH];
	uint64_t template [3 * 3];
	
	// fill both frame and template with random bytes
	memcpy(frame, (void*)memcpy, FRAME_HEIGHT * FRAME_WIDTH * 8);
	memcpy(template, (void*)memcpy, 3 * 3 * 8);
	
	for (int i = 0; i < FRAME_HEIGHT * FRAME_WIDTH; ++i) {
		printf("%i, ", frame[i]);
	}
	
	start = clock();
	for (long i = 0; i < TRIALS; i++) {
		sad(frame, 0, 0, frame, FRAME_HEIGHT, FRAME_WIDTH);
	}
	printf("Speed (C)  : %.1f MB/s\n", (double)(FRAME_HEIGHT * FRAME_WIDTH) * TRIALS / (clock() - start) * CLOCKS_PER_SEC / 1000000);
	
	//~ start = clock();
	//~ for (long i = 0; i < TRIALS; i++)
		//~ rc4_encrypt_x86(&state, msg, MSG_LEN);
	//~ printf("Speed (x86): %.1f MB/s\n", (double)MSG_LEN * TRIALS / (clock() - start) * CLOCKS_PER_SEC / 1000000);
	
	return EXIT_SUCCESS;
}

int self_test(void) {
	// testcase 1: "wikipedia example"
	/*
	 * Template    Search image
		2 5 5       2 7 5 8 6
		4 0 7       1 7 4 2 7
		7 5 9       8 4 6 8 5	 
	 */
	 
	
	 uint64_t frame [FRAME_HEIGHT * FRAME_WIDTH] = {
		2, 7, 5, 8, 6,
		1, 7, 4, 2, 7,
		8, 4, 6, 8, 5
	 };
	 uint64_t template [3 * 3] = {
		2, 5, 5,
		4, 0, 7,
		7, 5, 9
	 };
	 
	 assert(20 == sad((uint64_t[3 * 3]){2, 5, 5, 4, 0, 7, 7, 5, 9}, 0, 0, \
	  (uint64_t[5 * 3]){2, 7, 5, 8, 6, 1, 7, 4, 2, 7, 8, 4, 6, 8, 5}, 3, 5));
		 
	 //~ uint64_t sad_result = 0;
	 //~ sad_result = sad(template, 0, 0,
		//~ frame, 5, 3);
	
	//~ printf("%lu\n", sad_result);
	
	//~ if (sad_result == 20)
		//~ return 1;
	//~ else 
		//~ return 0;

	return 1;
}
