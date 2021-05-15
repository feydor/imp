/* bmp.c */
#include "../include/common.h"
#include "../include/bmp.h"

/*
// 24bit bmp with RGB colors
unsigned char *parse_24bit_bmp(FILE *file, BMPInfoHeader *biHeader) 
{
    BMPFileHeader bfHeader;
    unsigned char *bmpImage;
    
    // read the bmp file header
    fread(&bfHeader, sizeof(BMPFileHeader), 1, file);

    if (bfHeader.ftype != 0x4D42) {
        perror(ERR_NOT_BMP);
        fclose(file);
        return NULL;
    }
    
    // read the bmp info header
    fread(biHeader, sizeof(BMPInfoHeader), 1, file);
    
    // move the file ptr to begining of bmp data
    fseek(file, bfHeader.offset, SEEK_SET);
    
    // allocate bytes for bmp image data
    if ( !(bmpImage = malloc(biHeader->imageSize)) ) {
        perror(ERR_MALLOC_NULL);
        free(bmpImage);
        fclose(file);
        return NULL;
    }
    
    // read the bmp image data
    fread(bmpImage, biHeader->imageSize, 1, file);
    
    if (!bmpImage) {
        perror(ERR_FREAD_INPUT);
        fclose(file);
        return NULL;
    }
 
    fclose(file);
    return bmpImage;
}
*/

/* returns the same bmp but with its rgb data inverted */
unsigned char *
invert_bmp(unsigned char *bmp, size_t size) 
{
    for (size_t i = 0; i < bmpsize; i += 3) {
        bmp[i] = 255 - bmp[i];     // !R
        bmp[i+1] = 255 - bmp[i+1]; // !G
        bmp[i+2] = 255 - bmp[i+2]; // !B
    }
    return bmp;
}

/**
 * returns the number of bytes added to a bmp row to make it a multiple of 4
 */
size_t 
bmp_padding(size_t rowbytes) 
{
    return rowbytes % 4 == 0
           ? 0
           : abs((rowbytes % 4) - 4);
}

/**
 * returns the bmp width in bytes
 */
size_t
bmp_width(BMPInfoHeader *biHeader)
{
  return biHeader->imageWidth * biHeader->bitsPerPxl / 8;
}

void print_biHeader(BMPInfoHeader *biHeader) 
{
    printf("Printing BMPInfoHeader...\n");
    printf("size: %u B\n", biHeader->size);
    printf("imageWidth: %u px\n", biHeader->imageWidth);
    printf("imageHeight: %u px\n", biHeader->imageHeight);
    printf("bitsPerPxl: %u b/px\n", biHeader->bitsPerPxl);
    printf("imageSize: %u B\n", biHeader->imageSize);
}

