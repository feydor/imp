/* imagehandler.c - main routine for image processing */
#include <unistd.h> /* for EXIT_FAILURE, EXIT_SUCCESS */
#include <errno.h> /* for EINVAL */
#include <stdio.h> /* for printf */
#include <stdlib.h> /* for exit */
#include <string.h> /* for strerror */
#include "../include/main.h"
#include "../include/imagehandler.h"
#include "../include/imageproc.h"
#include "../include/sad-test.h"

extern int errno;

/* static function prototypes */
static int valid_options(options_t *options);

int 
handle_image(options_t *options)
{
    if (!valid_options(options))
        return EXIT_FAILURE;
    
    sad_selftest();
    printf("Self test passed!\n");
    
    /* parse the image into the char buffer */
    unsigned char *image = NULL;
    size_t height = 0, width = 0;

    if (!create_image_output_file(options->src, options->dest)) {
        perror("create_image_output_file");
        return -1;
    }

    if (!get_image_size(options->src, &width, &height)) {
        perror("get_image_size");
        return -1;
    }

    if (!(image = allocate_image_buf(width, height))) {
        perror("allocate_image_buf");
        return -1;
    }

    if (!read_image(options->src, image, width * height)) {
        perror("read_image");
        return -1;
    }

    // TODO: call image processing routine here
    
    if (!write_image(image, options->dest, width * height)) {
        perror("write_image");
        return -1;
    }

    free_image_buf(image);
    
	return 1;
}

/* validates options for filename */
static int
valid_options(options_t *options)
{
    if (!options) {
      errno = EINVAL;
      return EXIT_FAILURE;
    }
    
    if (!options->src) {
      errno = ENOENT;
      return EXIT_FAILURE;
    }

    return 1;
}
