/**
 * bmp.h
 * NOTE: Structs are 1-to-1 byte packed to mimic the BMP headers.
 * specification: https://en.wikipedia.org/wiki/BMP_file_format#Example_1
 */
#ifndef BMP_H
#define BMP_H

/*
extern uint64_t LONG;
extern uint32_t DWORD;
extern uint16_t WORD;
extern uint8_t  BYTE;
*/

#pragma pack(push, 1) // WORD should be 2B
typedef struct {
    WORD  bfType;        /* specifies the filetype, 0x424D specifies BMP */
    DWORD bfSize;        /* specifies the total size in bytes, header + data */
    WORD  bfReserved1;   /* reserved, must be 0 */
    WORD  bfReserved2;   /* reserved, must be 0 */
    DWORD bfOffBits;     /* specifies the offset in bytes from header to data */
} BITMAPFILEHEADER;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    DWORD biSize;        /* specifies the size of the info header in bytes */
    LONG  biWidth;       /* specifies width in pixels */
    LONG  biHeight;      /* specifies height in pixels */
    WORD  biPlanes;      /* specifies the number of color planes, must be 1 */
    WORD  biBitCount;    /* specifies the number of bits per pixel */
    DWORD biCompression; /* specifies the type of compression */
    DWORD biSizeImage;   /* specifies the image size in bytes */
    LONG  biXPxlsPerMeter; /* specifies the number of pixels per meter, x axis */
    LONG  biYPxlsPerMeter; /* specifies the number of pixels per meter, y axis */
    DWORD biClrsUsed;      /* number of colors used */
    DWORD biClrsImportant; /* number of colors that are important */
} BITMAPINFOHEADER;
#pragma pack(pop)

/* function prototypes */
int parse_24bit_bmp(FILE *);

#endif
