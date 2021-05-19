Specification for libsaru_imageproc

Images are defined as arrays of unsigned chars.
Each image type has an expected endianness.
Bmp functions expect 24bit color encoding.

int get_image_size(const char *src, size_t *width, size_t height)
    src is a string containing the location of the image file
        must be an existing image file
        must be non-NULL
    returns the image size in bytes if successful, -1 otherwise
        if error, width and height are unchanged

unsigned char * allocate_image_buf(size_t height, size_t width)
    caller is responsible for freeing the returned pointer
    returns a pointer to the allocated buffer if successful, NULL otherwise

int read_image(const char *src, unsigned char *dest, size_t size)
    the dest buffer will be big-endian (RGB) encoded when returned
    bmp files are expected to be little-endian (BGR) encoded
    returns 1 if successful, -1 otherwise 
        src buffer overflow is not checked

int write_image(const unsigned char *image, char *src, char *dest, size_t size)
    the file pointed to by dest will contain the image headers from the file
    pointed to by src followed by the image data pointed to by image
    returns 1 if successful, -1 otherwise
        src buffer overflow is not checked

void free_image_buf(unsigned char *image)
    sets the pointer to NULL


