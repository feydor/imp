/* sadx64.c */
#include "../include/common.h"
#include "../include/bmp.h"
#include "../include/sad.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
#define FRAME_HEIGHT 10
#define FRAME_WIDTH 20

// Link this program with an external C or x86-64 compression function
extern int sad(uint64_t* template, uint64_t starting_row,
	uint64_t starting_col, uint64_t* frame, uint64_t f_height, uint64_t f_width);
int handle_bmp(options_t *options);
void template_dim_from_frame_dim(int frame_w, int frame_h, int *x, int *y);
static int self_test(void);
static void run_benchmarks();
void strtoll_arr(unsigned char* arr, int width, int height);

int run_sad(options_t *options) {
    if (!options) {
      errno = EINVAL;
      return EXIT_FAILURE;
      /* NOTREACHED */
    }
    
    if (!options->input) {
      errno = ENOENT;
      return EXIT_FAILURE;
      /* NOTREACHED */
    }
   
    if (!self_test()) {
		printf("Self test failed!\n");
		return EXIT_FAILURE;
        /* NOTREACHED */
	}
	
	printf("Self test passed!\n");
    
    /* handle user-provided file */
    if (options->fname) {
        if (!handle_bmp(options)) {
            errno = ENOENT;
            return EXIT_FAILURE;
            /* NOTREACHED */
        }
    }
    
    if (options->verbose) {
        run_benchmarks();
    }
	return EXIT_SUCCESS;
    /* NOTREACHED */
}

int handle_bmp(options_t *options) {
    BITMAPINFOHEADER biHeader;
    unsigned char *bmpData; // must be free'd
    if( !(bmpData = parse_24bit_bmp(options->input, &biHeader)) ) {
        errno = ENOENT;
        return 0;
        /* NOTREACHED */
    }
    
    printf("Filename: %s\n", basename(options->fname));
    print_biHeader(&biHeader);
    
    // calculate frame height and width
    int w_bytes = biHeader.biWidth * biHeader.biBitPerPxl / 8;
    int frame_width = w_bytes + bmp_row_padding(w_bytes);
    int frame_height = biHeader.biHeight;
    printf("frame_width: %d\nframe_height: %d\n", frame_width, frame_height);
    
    // find a square template that fits the frame 
    // TODO: Let the user choose the template
    int template_w, template_h;
    template_dim_from_frame_dim(frame_width, frame_height, &template_w, &template_h);
    
    unsigned char template[template_w * template_h];
    memset(template, 0, template_w * template_h * sizeof(unsigned char));
    
    // start algorithms
    int res = 0;
    // res = sad((uint64_t *) template, 0, 0, (uint64_t *) bmpData, 4, 4);
    
    res = c_sad( template, template_w, template_h, bmpData, frame_width, frame_height );
    printf("Result of C_SAD: %d\n", res);
    
    free(bmpData);
    return 1;
}

/* Finds a square template that fits the given frame dimensions */
void template_dim_from_frame_dim(int frame_w, int frame_h, int *x, int *y) {
    *x = frame_w / 2;
    *y = frame_h / 2;
}

void run_benchmarks() {
    const long TRIALS = 1000;
    struct timeval stop, start, stop1, start1;
	
	uint64_t frame [FRAME_HEIGHT * FRAME_WIDTH];
	uint64_t template [3 * 3];
    unsigned char frame_uchar [FRAME_HEIGHT * FRAME_WIDTH];
	unsigned char template_uchar [3 * 3];
	
	// fill both frame and template with random bytes
	memcpy(frame, (void*) memcpy, FRAME_HEIGHT * FRAME_WIDTH * sizeof(uint64_t));
	memcpy(template, (void*) memcpy, 3 * 3 * sizeof(uint64_t));
    memcpy(frame_uchar, (void*) memcpy, FRAME_HEIGHT * FRAME_WIDTH * sizeof(unsigned char));
	memcpy(template_uchar, (void*) memcpy, 3 * 3 * sizeof(unsigned char));
	
    printf("Printing frame...\n");
	for (int i = 0; i < FRAME_HEIGHT * FRAME_WIDTH; ++i) {
		// printf("%li, ", frame[i]);
        printf("%u ", frame_uchar[i]);
	}
	printf("\n");
	
	gettimeofday(&start, NULL);
	for (long i = 0; i < TRIALS; i++) {
		sad(template, 0, 0, frame, FRAME_HEIGHT, FRAME_WIDTH);
	}
	gettimeofday(&stop, NULL);
	
	gettimeofday(&start1, NULL);
    for (long i = 0; i < TRIALS; i++) {
        c_sad(template_uchar, 3, 3, frame_uchar, FRAME_WIDTH, FRAME_HEIGHT);
    }
   gettimeofday(&stop1, NULL);
	
	printf("Printing benchmarks...\n");
    printf("Naive Looping x86-64 Assembly SAD:\n");
    printf("Time (uS)  : %lu us\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec );
	printf("Speed (C)  : %.1f MB/s\n", (double)(FRAME_HEIGHT * FRAME_WIDTH) * TRIALS / (stop.tv_usec - start.tv_usec) * CLOCKS_PER_SEC / 1000000);
    
     printf("Naive Looping C SAD:\n");
     printf("Time (uS)  : %lu us\n", (stop1.tv_sec - start1.tv_sec) * 1000000 + stop1.tv_usec - start1.tv_usec );
     printf("Speed (C)  : %.1f MB/s\n", (double)(FRAME_HEIGHT * FRAME_WIDTH) * TRIALS / (stop1.tv_usec - start.tv_usec) * CLOCKS_PER_SEC / 1000000);
	
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
	
    /*
	 uint64_t frame1[] = {
		2, 7, 5, 8, 6,
		1, 7, 4, 2, 7,
		8, 4, 6, 8, 5
	 };
	 uint64_t template1[] = {
		2, 5, 5,
		4, 0, 7,
		7, 5, 9
	 };
     */
    
     unsigned char frame[] = {
         2, 7, 5, 8, 6,
		1, 7, 4, 2, 7,
		8, 4, 6, 8, 5
    };
     unsigned char template[] = {
		2, 5, 5,
		4, 0, 7,
		7, 5, 9
	 };

     // assert(17 == sad(template, 0, 0, frame, 3, 5));
     int res = 0;
     res = c_sad(template, 3, 3, frame, 5, 3);
     assert(17 == res);
     
     /*
     strtoll_arr(frame, 5, 3);
     strtoll_arr(template, 3, 3);
     printf("%lu %lu\n", frame[0], frame[1]);
     
     res = sad((uint64_t *)template1, 0, 0, (uint64_t*)frame1, 3, 5);
     printf("%d\n", res);
     */

	 return 1;
}

void strtoll_arr(unsigned char* arr, int width, int height) {
    for (int i = 0; i < width * height; i++) {
        arr[i] = (uint64_t) arr[i];
    }
}

