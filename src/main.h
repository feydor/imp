/* main.h - the program's UNIX interface*/
#ifndef MAIN_H
#define MAIN_H

#define ERR_FOPEN_INPUT  "fopen(input, r)"
#define ERR_FOPEN_OUTPUT "fopen(output, w)"
#define ERR_FREAD_INPUT "fread failed"
#define ERR_HANDLEIMAGE "handle_image blew up"
#define ERR_NOT_BMP "filetype is not a bmp"
#define ERR_MALLOC_NULL "malloc returned null"
#define DEFAULT_PROGNAME "imp"
    
#define OPTSTR "i:o:p:f:h"
#define USAGE_FMT "Usage: %s [OPTIONS] INPUT\n\nOptions:\n\
    -h, --help               Display this message\n    -f, --flags              Image processing passes\n\
    -i, --input              Input filename\n    -o, --output             Write output to filename\n\n\
Flags:\n     d     ordered dithering\n     g     grayscale\n     i     invert\n     n     uniform noise\n\
     p     palette quantization\n"

static struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"palette", required_argument, NULL, 'p'},
    {"flags", required_argument, NULL, 'f'},
    {"input", required_argument, NULL, 'i'},
    {"output", required_argument, NULL, 'o'},
    {NULL, 0, NULL, 0}
};

#define BFHEADER_SIZE 14
#define BIHEADER_SIZE 40

typedef unsigned char uchar;

// size: 16 bytes (14 bytes  + 2 bytes padding after ftype)
typedef struct {
    uint16_t  ftype;        /* specifies the filetype, 0x424D specifies BMP */
    uint32_t  fsize;        /* specifies the total size in bytes, header + data */
    uint16_t  reserved1;    /* reserved, must be 0 */
    uint16_t  reserved2;    /* reserved, must be 0 */
    uint32_t  offset;       /* specifies the offset in bytes from header to data */
} BMP_file_header;

// for use with 24bit bitmap with pixel format RGB24
// size: 40 bytes
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

/**
 * @brief holds the two headers and the raw image data for IO, data MUST include padding bytes
 * see https://en.wikipedia.org/wiki/BMP_file_format#Example_1
 */
typedef struct {
    BMP_file_header fheader;
    BMP_info_header iheader;
    uchar *raw_image;
    unsigned width_px;
    unsigned height_px;
    size_t image_size_bytes;
} BMP_file;

#endif

