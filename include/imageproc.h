#ifndef TEMPLATE_H
#define TEMPLATE_H

/* datatypes */

/* function prototypes */
int create_image_output_file(const char *src, char *dest);
int get_image_size(const char *src, size_t *width, size_t *height);
unsigned char * allocate_image_buf(size_t height, size_t width);
int read_image(const char *src, unsigned char *dest, size_t size);
int write_image(unsigned char *src, char *dest, size_t size);
void free_image_buf(unsigned char *image);

#endif

