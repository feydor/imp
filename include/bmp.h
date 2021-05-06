/**
 * bmp.h
 * NOTE: Structs are 1-to-1 byte packed to mimic the BMP headers.
 * specification: https://en.wikipedia.org/wiki/BMP_file_format#Example_1
 */
#ifndef BMP_H
#define BMP_H

#pragma pack(push, 1) // WORD should be 2B
typedef struct {
    uint16_t  ftype;        /* specifies the filetype, 0x424D specifies BMP */
    uint32_t  fsize;        /* specifies the total size in bytes, header + data */
    uint16_t  reserved1;    /* reserved, must be 0 */
    uint16_t  reserved2;    /* reserved, must be 0 */
    uint32_t  offset;       /* specifies the offset in bytes from header to data */
} BMPFileHeader;
#pragma pack(pop)

// for use with 24bit bitmap with pixel format RGB24
// size: 36 bytes
typedef struct {
    uint32_t  size;        /* specifies the size of the info header in bytes */
    uint32_t  imageWidth;       /* specifies width in pixels */
    uint32_t  imageHeight;      /* specifies height in pixels */
    uint16_t  planes;      /* specifies the number of color planes, must be 1 */
    uint16_t  bitsPerPxl;   /* specifies the number of bits per pixel */
    uint32_t  compressionType; /* specifies the type of compression */
    uint32_t  imageSize;   /* specifies the image size in bytes */
    uint32_t  XpxlsPerMeter; /* specifies the number of pixels per meter, x axis */
    uint32_t  YPxlsPerMeter; /* specifies the number of pixels per meter, y axis */
    uint32_t  colorsUsed;      /* number of colors used */
    uint32_t  colorsImportant; /* number of colors that are important */
} BMPInfoHeader;

/*
// for use with 32bit bitmap with pixel format ARGB32
// Size: 96 Bytes
typedef struct {
    uint32_t  bV4Size;
    uint32_t  bV4Width;
    uint32_t  bV4Height;
    uint16_t   bV4Planes;
    uint16_t   bV4BitCount;
    uint32_t  bV4V4Compression;
    uint32_t  bV4ImageSizw;
    uint32_t  bV4XPxlsPerMeter;
    uint32_t  bV4YPxlsPerMeter;
    uint32_t  bV4ClrsUsed;
    uint32_t  bV4ClrsImportant;
    uint32_t  bV4RedMask;
    uint32_t  bV4GreenMask;
    uint32_t  bV4BlueMask;
    uint32_t  bV4AlphaMask;
    uint32_t  bV4CSType; // 60
    
    // bV4Endpoints, 12B
    uint32_t  bV4RedX;
    uint32_t  bV4RedY;
    uint32_t  bV4RedZ;
    uint32_t  bV4GreenX;
    uint32_t  bV4GreenY;
    uint32_t  bV4GreenZ;
    uint32_t  bV4BlueX;
    uint32_t  bV4BlueY;
    uint32_t  bV4BlueZ;
    
    uint32_t  bV4GammaRed;
    uint32_t  bV4GammaGreen;
    uint32_t  bV4GammaBlue;
} BitmapV4Header;
*/

/* function prototypes */
unsigned char *parse_24bit_bmp(FILE *f, BMPInfoHeader *new_biHeader);
unsigned char *parse_24bit_bmp_filename(char *fname, BMPInfoHeader *new_biHeader);
unsigned char *invert_24bit_bmp(unsigned char *bmp, BMPInfoHeader *biHeader);
int bmp_row_padding(int width_bytes);
void print_biHeader(BMPInfoHeader *biHeader);

#endif
