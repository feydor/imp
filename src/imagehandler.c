/* imagehandler.c - main routine for image processing */
#include "../include/imagehandler.h"
#include "../include/main.h"

#include <unistd.h> /* for EXIT_FAILURE, EXIT_SUCCESS */

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
    get_image_size(options->input, &width, &height);
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
    
    if (!options->fname) {
      errno = ENOENT;
      return EXIT_FAILURE;
    }
}
