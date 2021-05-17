/* bmp.c */
#include "../include/bmp.h"
#include <stdio.h> /* for printf */
#include <stdlib.h> /* for abs */

/* returns the same bmp but with its rgb data inverted */
unsigned char *
invert_bmp(unsigned char *bmp, size_t size) 
{
    for (size_t i = 0; i < size; i += 3) {
        bmp[i] = 255 - bmp[i];     // !R
        bmp[i+1] = 255 - bmp[i+1]; // !G
        bmp[i+2] = 255 - bmp[i+2]; // !B
    }
    return bmp;
}

/**
 * returns the number of bytes added to a bmp row to make it a multiple of 4
 */
int 
bmp_padding(int width) 
{
    return width % 4 == 0
           ? 0
           : abs((width % 4) - 4);
}

/**
 * returns the bmp width in bytes
 */
size_t
bmp_width(struct bmp_iheader *bih)
{
  return bih->imageWidth * bih->bitsPerPxl / 8;
}

void print_bih(struct bmp_iheader *bih) 
{
    printf("Printing struct bmp_iheader...\n");
    printf("size: %u B\n", bih->size);
    printf("imageWidth: %u px\n", bih->imageWidth);
    printf("imageHeight: %u px\n", bih->imageHeight);
    printf("bitsPerPxl: %u b/px\n", bih->bitsPerPxl);
    printf("imageSize: %u B\n", bih->imageSize);
}

void print_bfh(struct bmp_fheader *bfh) 
{
    printf("Printing struct bmp_fheader...\n");
    printf("ftype: 0x%X \n", bfh->ftype);
    printf("reserved1: 0x%X \n", bfh->reserved1);
    printf("reserved2: 0x%X \n", bfh->reserved2);
    printf("fsize: 0x%X B\n", bfh->fsize);
    printf("offset: 0x%X B\n", bfh->offset);

}
