/** bmp.h - types and functions for IO of bmp files */
#ifndef BMP_H
#define BMP_H

#include <stddef.h>
#include <stdint.h>

#define FILEHEADER_SIZE 14
#define INFOHEADER_SIZE 40
#define BMP_MAGIC 0x4D42

typedef unsigned char uchar;

typedef struct {
    uint16_t  ftype;        /* specifies the filetype, 0x424D specifies BMP */
    uint32_t  fsize;        /* specifies the total size in bytes, header + data */
    uint16_t  reserved1;    /* reserved, must be 0 */
    uint16_t  reserved2;    /* reserved, must be 0 */
    uint32_t  offset;       /* specifies the offset in bytes from header to data */
} BMP_file_header;

// for use with 24bit bitmaps
typedef struct {
    uint32_t  header_size;        /* specifies the size of the info header in bytes */
    uint32_t  width_px;       /* specifies width in pixels */
    uint32_t  height_px;      /* specifies height in pixels */
    uint16_t  planes;      /* specifies the number of color planes, must be 1 */
    uint16_t  bits_per_pixel;   /* specifies the number of bits per pixel */
    uint32_t  compression_type; /* specifies the type of compression */
    uint32_t  image_size_bytes;   /* specifies the image size in bytes */
    uint32_t  x_resolution_ppm; /* specifies the number of pixels per meter, x axis */
    uint32_t  y_resolution_ppm; /* specifies the number of pixels per meter, y axis */
    uint32_t  colors_used;      /* number of colors used */
    uint32_t  colors_important; /* number of colors that are important */
} BMP_info_header;

typedef enum {
    NOT_A_BMP = 1,
    NOT_FOUND,
    MALLOC_FAILED,
} BMP_error;

/**
 * @brief holds the two headers and the raw image data for IO, data MUST include padding bytes
 * see https://en.wikipedia.org/wiki/BMP_file_format#Example_1
 */
typedef struct {
    BMP_file_header *file_header;
    BMP_info_header *info_header;
    uchar *image_raw;               // image without end of row padding
    uchar *image_render;            // image as accepted by SDL
    unsigned w;
    unsigned h;
    unsigned nbytes;
} BMP_file;

/** all BMP_* functions set the global bmp_err variable on error and then return -1*/
int BMP_load(BMP_file *bmp, const char *src);
int BMP_write(BMP_file *file, const char *dest);
void BMP_print_dimensions(BMP_file *bmp);
void BMP_free(BMP_file *bmp);

#endif