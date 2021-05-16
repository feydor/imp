/* imagehandler.c - main routine for image processing */
#include <unistd.h> /* for EXIT_FAILURE, EXIT_SUCCESS */
#include <errno.h> /* for EINVAL */
#include <stdio.h> /* for printf */
#include "../include/main.h"
#include "../include/imagehandler.h"
#include "../include/imageproc.h"
#include "../include/sad-test.h"

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
    size_t height, width;

    create_image_file(options->iname, options->oname);
    get_image_size(options->iname, &width, &height);
    image = allocate_image_buf(width, height);
    read_image(options->iname, image, width * height);

    // TODO: call image processing routine here
    
    write_image(options->oname, image, width * height);
    free_image_buf(image);
    
	return EXIT_SUCCESS;
}

/* validates options for filename */
static int
valid_options(options_t *options)
{
    if (!options) {
      errno = EINVAL;
      return EXIT_FAILURE;
    }
    
    if (!options->iname) {
      errno = ENOENT;
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
