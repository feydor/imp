/* bmp.c */
#include "../include/common.h"
#include "../include/bmp.h"

/*
// 24bit bmp with RGB colors
unsigned char *parse_24bit_bmp(FILE *file, struct bmp_iheader *bih) 
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
    fread(bih, sizeof(struct bmp_iheader), 1, file);
    
    // move the file ptr to begining of bmp data
    fseek(file, bfHeader.offset, SEEK_SET);
    
    // allocate bytes for bmp image data
    if ( !(bmpImage = malloc(bih->imageSize)) ) {
        perror(ERR_MALLOC_NULL);
        free(bmpImage);
        fclose(file);
        return NULL;
    }
    
    // read the bmp image data
    fread(bmpImage, bih->imageSize, 1, file);
    
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

