/**
 * bmp.h
 * NOTE: Structs are 1-to-1 byte packed to mimic the BMP headers.
 * specification: https://en.wikipedia.org/wiki/BMP_file_format#Example_1
 */
#ifndef BMP_H
#define BMP_H

typedef uint64_t LONG;    // LONG = unsigned 64 bit value
typedef uint32_t DWORD;   // DWORD = unsigned 32 bit value
typedef uint16_t WORD;    // WORD = unsigned 16 bit value
typedef uint8_t  BYTE;    // BYTE = unsigned 8 bit value

#pragma pack(push, 1) // WORD should be 2B
typedef struct {
    WORD  bfType;        /* specifies the filetype, 0x424D specifies BMP */
    DWORD bfSize;        /* specifies the total size in bytes, header + data */
    WORD  bfReserved1;   /* reserved, must be 0 */
    WORD  bfReserved2;   /* reserved, must be 0 */
    DWORD bfOffset;      /* specifies the offset in bytes from header to data */
} BITMAPFILEHEADER;
#pragma pack(pop)

#pragma pack(push, 1)
// for use with 24bit bitmap with pixel format RGB24
typedef struct {
    DWORD  biSize;        /* specifies the size of the info header in bytes */
    DWORD  biWidth;       /* specifies width in pixels */
    DWORD  biHeight;      /* specifies height in pixels */
    WORD   biPlanes;      /* specifies the number of color planes, must be 1 */
    WORD   biBitPerPxl;   /* specifies the number of bits per pixel */
    DWORD  biCompression; /* specifies the type of compression */
    DWORD  biImageSize;   /* specifies the image size in bytes */
    DWORD  biXPxlsPerMeter; /* specifies the number of pixels per meter, x axis */
    DWORD  biYPxlsPerMeter; /* specifies the number of pixels per meter, y axis */
    DWORD  biClrsUsed;      /* number of colors used */
    DWORD  biClrsImportant; /* number of colors that are important */
} BITMAPINFOHEADER;
#pragma pack(pop)

#pragma pack(push, 1)
// for use with 32bit bitmap with pixel format ARGB32
// Size: 96 Bytes
typedef struct {
    DWORD  bV4Size;
    DWORD  bV4Width;
    DWORD  bV4Height;
    WORD   bV4Planes;
    WORD   bV4BitCount;
    DWORD  bV4V4Compression;
    DWORD  bV4ImageSizw;
    DWORD  bV4XPxlsPerMeter;
    DWORD  bV4YPxlsPerMeter;
    DWORD  bV4ClrsUsed;
    DWORD  bV4ClrsImportant;
    DWORD  bV4RedMask;
    DWORD  bV4GreenMask;
    DWORD  bV4BlueMask;
    DWORD  bV4AlphaMask;
    DWORD  bV4CSType; // 60
    
    // bV4Endpoints, 12B
    DWORD  bV4RedX;
    DWORD  bV4RedY;
    DWORD  bV4RedZ;
    DWORD  bV4GreenX;
    DWORD  bV4GreenY;
    DWORD  bV4GreenZ;
    DWORD  bV4BlueX;
    DWORD  bV4BlueY;
    DWORD  bV4BlueZ;
    
    DWORD  bV4GammaRed;
    DWORD  bV4GammaGreen;
    DWORD  bV4GammaBlue;
} BITMAPV4HEADER;
#pragma pack(pop)

/* function prototypes */
unsigned char *parse_24bit_bmp(FILE *, BITMAPINFOHEADER *);
int bmp_row_padding(int width_bytes);
void print_biHeader(BITMAPINFOHEADER *);

#endif
