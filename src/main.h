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
    
#define OPTSTR "i:o:h"
#define USAGE_FMT  "Usage: %s [-i inputfile] [-o outputfile] [-h]\n"

#define BFHEADER_SIZE 14
#define BIHEADER_SIZE 40

typedef unsigned char uchar;

// size: 16 bytes (14 bytes  + 2 bytes padding after ftype)
struct bmp_fheader {
    uint16_t  ftype;        /* specifies the filetype, 0x424D specifies BMP */
    uint32_t  fsize;        /* specifies the total size in bytes, header + data */
    uint16_t  reserved1;    /* reserved, must be 0 */
    uint16_t  reserved2;    /* reserved, must be 0 */
    uint32_t  offset;       /* specifies the offset in bytes from header to data */
};

// for use with 24bit bitmap with pixel format RGB24
// size: 40 bytes
struct bmp_iheader {
    uint32_t  size;        /* specifies the size of the info header in bytes */
    uint32_t  imageWidth;       /* specifies width in pixels */
    uint32_t  imageHeight;      /* specifies height in pixels */
    uint16_t  planes;      /* specifies the number of color planes, must be 1 */
    uint16_t  bitsPerPxl;   /* specifies the number of bits per pixel */
    uint32_t  compressionType; /* specifies the type of compression */
    uint32_t  imageSize;   /* specifies the image size in bytes */
    uint32_t  XpxlsPerMeter; /* specifies the number of pixels per meter, x axis */
    uint32_t  YpxlsPerMeter; /* specifies the number of pixels per meter, y axis */
    uint32_t  colorsUsed;      /* number of colors used */
    uint32_t  colorsImportant; /* number of colors that are important */
};

#endif

