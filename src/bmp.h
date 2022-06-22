/* bmp.h */
#ifndef BMP_H
#define BMP_H

#include <stdint.h> /* for uint32_t */
#include <stddef.h> /* for size_t */

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
unsigned char *invert_bmp(unsigned char *bmp, size_t size);
int bmp_padding(int width); 
size_t bmp_width(struct bmp_iheader *bih);
void print_bih(struct bmp_iheader *bih);
void print_bfh(struct bmp_fheader *bfh);

#endif
