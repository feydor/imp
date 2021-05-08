/* sadx64.c */
#include "../include/common.h"
#include "../include/bmp.h"
#include "../include/sad.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <sys/time.h>
#include <libgen.h> /* for basename() */

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
#define FRAME_HEIGHT 10
#define FRAME_WIDTH 20

// Link this program with an external C or x86-64 compression function
extern int sad(uint64_t* template, uint64_t starting_row,
	uint64_t starting_col, uint64_t* frame, uint64_t f_height, uint64_t f_width);
int handle_bmp(options_t *options);
static void template_dim_from_frame_dim(unsigned int frame_w, unsigned int frame_h, unsigned int *x, unsigned int *y);
static int self_test(void);
static void run_benchmarks();

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
    BMPInfoHeader biHeader;
    unsigned char *bmpData; // must be free'd
    if( !(bmpData = parse_24bit_bmp(options->input, &biHeader)) ) {
        errno = ENOENT;
        return 0;
        /* NOTREACHED */
    }
    
    printf("Filename: %s\n", basename(options->fname));
    print_biHeader(&biHeader);
    
    // calculate frame height and width
    // TODO: is frame_height in pixels or bytes? check biHeader.bitsPerPixel
    unsigned int frame_width = image_width_bytes(&biHeader) + bmp_row_padding(image_width_bytes(&biHeader));
    unsigned int frame_height = biHeader.imageHeight;
    printf("frame_width: %d\nframe_height: %d\n", frame_width, frame_height);
    
    // find a square template that fits the frame 
    // TODO: Let the user choose the template
    unsigned int template_width, template_height;
    template_dim_from_frame_dim(frame_width, frame_height, &template_width, &template_height);
    printf("template_width: %d\ntemplate_height: %d\n", template_width, template_height);
    
    // create and assign UCharBuffer structs for template and frame
    UCharBuffer *template;
    UCharBuffer *frame;
    template = create_UCharBuffer(template_width, template_height);
    frame = create_UCharBuffer_from_uchar(bmpData, frame_width, frame_height);
    
    // start algorithms
    int res = 0;
    // res = sad((uint64_t *) template, 0, 0, (uint64_t *) bmpData, 4, 4);
    
    res = c_sad( template, frame );
    printf("Result of C_SAD: %d\n", res);
    
    //free(bmpData);
    destroy_UCharBuffer(template);
    destroy_UCharBuffer(frame);
    return 1;
}

/* Finds a square template that fits the given frame dimensions */
static void 
template_dim_from_frame_dim(unsigned int frame_w, unsigned int frame_h,
        unsigned int *x, unsigned int *y)
{
    *x = frame_w / 2;
    *y = frame_h / 2;
}

static void
run_benchmarks() 
{
    const long TRIALS = 1000;
    struct timeval stop, start, stop1, start1;
	
	uint64_t frame [FRAME_HEIGHT * FRAME_WIDTH];
	uint64_t template [3 * 3];
    UCharBuffer *tb;
    UCharBuffer *fb;
    tb = create_UCharBuffer(3, 3);
    fb = create_UCharBuffer(FRAME_WIDTH, FRAME_HEIGHT);
	
	// fill both frame and template with random bytes
	memcpy(frame, (void*) memcpy, FRAME_HEIGHT * FRAME_WIDTH * sizeof(uint64_t));
	memcpy(template, (void*) memcpy, 3 * 3 * sizeof(uint64_t));
    memcpy(fb->buffer, (void*) memcpy, fb->size);
	memcpy(tb->buffer, (void*) memcpy, tb->size);
	
    printf("Printing frame...\n");
    unsigned char *fbp = fb->buffer;
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
     // assert(17 == sad(template, 0, 0, frame, 3, 5));
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
     UCharBuffer *template;
     UCharBuffer *frame;
     template = create_UCharBuffer_from_uchar(tb, 3, 3);
     frame = create_UCharBuffer_from_uchar(fb, 5, 3); 

     int res = 0;
     res = c_sad(template, frame);
     assert(17 == res);
     // TODO: Assert frame->row = ?? and frame->col = ??, results location

	 return 1;
}

