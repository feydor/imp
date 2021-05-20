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
    // struct image_t image = { 0 };
    unsigned char *image = NULL;
    size_t w = 0, h = 0;

    if (!get_image_size(options->src, &w, &h))
        perror("get_image_size");

    image = allocate_image_buf(w, h);

    if (!read_image(options->src, image, w * h))
        perror("read_image");

    // TODO: call image processing routine here
    // invert_bmp(image.buf, height * width);
    // ordered_dithering(image);
    
    if (!write_image(image, options->src, options->dest, w * h))
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
