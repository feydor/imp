/* imageproc.c */
#include "../include/imageproc.h"

/* static function protoypes */
static int isbmp(FILE *fp);
static uint16_t read_ftype(FILE *fp);
static void read_bmpheaders(FILE *fp, BMPFileHeader *bfHeader, 
                                      BMPInfoHeader *biHeader);
static void create_bmp_file(FILE *fp, BMPFileHeader *bfHeader, 
                                      BMPInfoHeader *biHeader);
static void swap_endian(unsigned char *image, size_t size);
static void write_bmp_buf(FILE *fp, unsigned char *image, size_t size);

/* creates an output file with the headers copied from the input file */
void
create_image_file(char *in, char *out)
{
    FILE *ifp = fopen(in, "rb");
    FILE *ofp = fopen(out, "wb");
    if (!ifp || !ofp) {
        printf("Error opening file, create_image_file\n");
        exit(1);
    }

    if (isbmp(fp)) {
        BMPFileHeader bfh;
        BMPInfoHeader bih;
        read_bmpheaders(ifp, &bfh, &bih);
        create_bmp_file(ifp, &bfh, &bih);
    }

    fclose(ifp);
    fclose(ofp);
}

/* modifies width and height to hold the dimensions of file fp in bytes */
void
get_image_size(FILE *fp, size_t *width, size_t *height)
{
    
    FILE *fp = fopen(input, "rb");
    if (!fp) {
        printf("Error opening files, get_image_size\n");
        exit(1);
    }

    if (isbmp(fp)) {
        struct BMPFileHeader bfh;
        struct BMPInfoHeader bih;
        
        read_bmpheaders(input, &bfh, &bih);
        *width = bmp_width(&bih) + bmp_padding(bmp_width(&bih));
        *height = bih.imageHeight;
    }
    
    fclose(fp);
}

/* returns a heap-allocated unsigned char buffer */
unsigned char *
allocate_image_buf(size_t height, size_t width)
{
    unsigned char *buf;
    buf = malloc(height * width); 
    return buf;
}

/* returns the image bytes in file in; image is assumed preallocated */
void
read_image(char *in, unsigned char *image, size_t size)
{
    FILE *fp = fopen(in, "rb");
    if (!fp) {
        printf("Error opening file, read_image\n");
        exit(1);
    }

    if (isbmp(fp)) {
        const int offset = BFHEADER_SIZE + BIHEADER_SIZE;
        fseek(fp, offset, SEEK_SET);
        fread(image, size, 1, fp);
        swap_endian(image);
    }
    
    fclose(fp);
}

void
write_image(char *out, unsigned char *image, size_t size)
{
    FILE *fp = fopen(out, "wb");
    if (!fp) {
        printf("Error opening file, write_image\n");
        exit(1);
    }

    if (isbmp(fp)) {
        BMPFileHeader bfh;    
        BMPInfoHeader bih;    
        read_bmpheaders(fp, &bfh, &bih);
        write_buf_buf(fp, image, size);
    }

    fclose(fp);
}

void
free_image_buf(unsigned char *image)
{
    if (image)
        free(image);
}


/**
 * static functions start here
 */

/* it is a bmp if the first two bytes of the file are 0x4D42 */
static int
isbmp(FILE *fp)
{
    return read_ftype(fname) == 0x4D42;
}

/* the first two bytes of an image header specifies its filetype; return it */
static uint16_t
read_ftype(FILE *fp)
{
    if (!fp)
        return 0;
    fread(&ftype, sizeof(ftype), 1, fp);
    rewind(fp);
    return ftype;
}

static void
read_bmpheaders(FILE *fp, BMPFileHeader *bfHeader, 
                          BMPInfoHeader *biHeader)
{
    if (!fp)
        return;
    fread(bfHeader, BFHEADER_SIZE, 1, fp);
    fread(biHeader, BIHEADER_SIZE, 1, fp);
}

/* writes the headers into a new file named fname */
static void
create_bmp_file(FILE *fp, BMPFileHeader *bfHeader, 
                          BMPInfoHeader *biHeader) 
{
    if (!fp) {
        printf("Error! Writing file.");
        exit(1);
    }
    fwrite(bfHeader, BFHEADER_SIZE, 1, fp);
    fwrite(biHeader, BIHEADER_SIZE, 1, fp);
}

/* swap the least-significant byte with the most-significant */
static void
swap_endian(unsigned char *image, size_t size)
{
    unsigned char temp;
    for (size_t = 0; i < size; i += 3) {
        temp = image[i];
        image[i] = image[i + 2];
        image[i + 2] = temp;
    }
}

/** 
 * writes the size bytes from the buffer image into the file fp;
 * fseeks just past both bmp headers
 */
static void
write_bmp_buf(FILE *fp, unsigned char *image, size_t size)
{
    const int offset = BFHEADER_SIZE + BIHEADER_SIZE;
    fseek(fp, offset, SEEK_SET);
    fwrite(image, size, 1, fp);
}
