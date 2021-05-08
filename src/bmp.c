/* bmp.c */
#include "../include/common.h"
#include "../include/bmp.h"

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

/* wrapper to handle filename strings */
unsigned char *parse_24bit_bmp_filename(char *fname, BMPInfoHeader *biHeader) 
{
	FILE *fp;
	if ( !(fp = fopen(fname, "rb")) ) {
		perror(ERR_FOPEN_INPUT);
        exit(EXIT_FAILURE);
	}
	
	parse_24bit_bmp(fp, biHeader);
}

/*
unsigned char *parse_32bit_bmp(FILE *file, BITMAPV4HEADER *bv4Header) {
    // parse_bmp(file, (void *) bv4Header);
}
*/

/* returns the same bmp but with its rgb data inverted */
unsigned char *invert_24bit_bmp(unsigned char *bmp, BMPInfoHeader *biHeader) 
{
    for (unsigned int i = 0; i < biHeader->imageSize; i += 3) {
        bmp[i] = 255 - bmp[i];     // !R
        bmp[i+1] = 255 - bmp[i+1]; // !G
        bmp[i+2] = 255 - bmp[i+2]; // !B
    }
    return bmp;
}

/**
 * returns the number of bytes added to a bmp row to make it a multiple of 4
 */
unsigned int 
bmp_row_padding(unsigned int row_bytes) 
{
    return row_bytes % 4 == 0
           ? 0
           : abs((row_bytes % 4) - 4);
}

/**
 * returns the image width in bytes
 */
unsigned int
image_width_bytes(BMPInfoHeader *biHeader)
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

