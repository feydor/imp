/* bmp.c */
#include "../include/common.h"
#include "../include/bmp.h"

// 24bit bmp with RGB colors
unsigned char *parse_24bit_bmp(FILE *file, BITMAPINFOHEADER *biHeader) {
    BITMAPFILEHEADER bfHeader;
    unsigned char *bmpImage;
    
    // read the bmp file header
    fread(&bfHeader, sizeof(BITMAPFILEHEADER), 1, file);

    if (bfHeader.bfType != 0x4D42) {
        perror(ERR_NOT_BMP);
        fclose(file);
        return NULL;
    }
    
    // read the bmp info header
    fread(biHeader, sizeof(BITMAPINFOHEADER), 1, file);
    
    // move the file ptr to begining of bmp data
    fseek(file, bfHeader.bfOffset, SEEK_SET);
    
    // allocate bytes for bmp image data
    if ( !(bmpImage = malloc(biHeader->biImageSize)) ) {
        perror(ERR_MALLOC_NULL);
        free(bmpImage);
        fclose(file);
        return NULL;
    }
    
    // read the bmp image data
    fread(bmpImage, biHeader->biImageSize, 1, file);
    
    if (!bmpImage) {
        perror(ERR_FREAD_INPUT);
        fclose(file);
        return NULL;
    }
    
    // swap bmp BGR to RGB
    unsigned char temp;
    for (unsigned int i = 0; i < biHeader->biImageSize; i += 3) {
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

void print_biHeader(BITMAPINFOHEADER *biHeader) {
    printf("Printing BITMAPINFOHEADER...\n");
    printf("biSize: %u B\n", biHeader->biSize);
    printf("biWidth: %u px\n", biHeader->biWidth);
    printf("biHeight: %u px\n", biHeader->biHeight);
    printf("biBitPerPxl: %u b/px\n", biHeader->biBitPerPxl);
    printf("biImageSize: %u B\n", biHeader->biImageSize);
}
