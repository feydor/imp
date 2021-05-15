#ifndef TEMPLATE_H
#define TEMPLATE_H
/* datatypes */

/* function prototypes */
void create_image_file(char *in, char *out);
void get_image_size(FILE *fp, size_t *width, size_t *height);
unsigned char * allocate_image_buf(size_t height, size_t width);
void read_image(char *in, unsigned char *img, size_t imgsize);
void write_image(char *out, unsigned char *img, size_t imgsize);
void free_image_buf(unsigned char *img);

#endif

