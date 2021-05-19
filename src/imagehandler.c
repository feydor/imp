/* imagehandler.c - main routine for image processing */
#include <unistd.h> /* for EXIT_FAILURE, EXIT_SUCCESS */
#include <errno.h> /* for EINVAL */
#include <stdio.h> /* for printf */
#include <stdlib.h> /* for exit */
#include <string.h> /* for strerror */
#include "../include/main.h"
#include "../include/bmp.h"
#include "../include/imagehandler.h"
#include "../include/imageio.h"
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

    if (!get_image_size(options->src, &width, &height))
        perror("get_image_size");

    image = allocate_image_buf(width, height);

    if (!read_image(options->src, image, width * height))
        perror("read_image");

    // TODO: call image processing routine here
    // invert_bmp(image, height * width);
    ordered_dithering(image, width, height);
    
    if (!write_image(image, options->src, options->dest, width * height))
        perror("write_image");

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
