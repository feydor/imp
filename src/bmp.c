/* bmp.c */
#include "../include/common.h"
#include "../include/bmp.h"

// 24bit bmp with RGB colors
unsigned char *parse_24bit_bmp(FILE *file, BitmapInfoHeader *biHeader) {
    BitmapFileHeader bfHeader;
    unsigned char *bmpImage;
    
    // read the bmp file header
    fread(&bfHeader, sizeof(BitmapFileHeader), 1, file);

    if (bfHeader.ftype != 0x4D42) {
        perror(ERR_NOT_BMP);
        fclose(file);
        return NULL;
    }
    
    // read the bmp info header
    fread(biHeader, sizeof(BitmapInfoHeader), 1, file);
    
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
    
    // swap bmp BGR to RGB
    unsigned char temp;
    for (unsigned int i = 0; i < biHeader->imageSize; i += 3) {
        temp = bmpImage[i];
        bmpImage[i] = bmpImage[i + 2];
        bmpImage[i + 2] = temp;
    }
    
    fclose(file);
    return bmpImage;
}

/*
unsigned char *parse_32bit_bmp(FILE *file, BITMAPV4HEADER *bv4Header) {
    // parse_bmp(file, (void *) bv4Header);
}
*/

/**
 * gets the number of bytes added to a bmp row to make it a multiple of 4
 */
int bmp_row_padding(int row_bytes) {
    return row_bytes % 4 == 0
           ? 0
           : abs((row_bytes % 4) - 4);
}

void print_biHeader(BitmapInfoHeader *biHeader) {
    printf("Printing BitmapInfoHeader...\n");
    printf("size: %u B\n", biHeader->size);
    printf("imageWidth: %u px\n", biHeader->imageWidth);
    printf("imageHeight: %u px\n", biHeader->imageHeight);
    printf("bitsPerPxl: %u b/px\n", biHeader->bitsPerPxl);
    printf("imageSize: %u B\n", biHeader->imageSize);
}

