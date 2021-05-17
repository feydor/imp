/* imageproc.c - functions to read and write image files */
#include <stdio.h> /* for FILE, freas, fwrite */
#include <stdint.h> /* for uint16_t */
#include <stdlib.h> /* for malloc, exit */
#include <errno.h> /* for errno */
#include "../include/imageproc.h"
#include "../include/bmp.h"

extern int errno; /* these functions set errno on errors */

/* static function protoypes */
static int isbmp(FILE *fp);
static uint16_t read_ftype(FILE *fp);
static int read_bmpheaders(FILE *fp, struct bmp_fheader *bfhead, 
                                      struct bmp_iheader *bihead);
static int create_bmp_file(FILE *fp, struct bmp_fheader *bfhead, 
                                      struct bmp_iheader *bihead);
static int swap_endian(unsigned char *image, size_t size);
static int write_bmp_buf(FILE *fp, const unsigned char *image, size_t size);

/**
 * Creates an output file at the pathname pointed to by dest
 * containing the file header(s) from the file pointed to by src.
 * Returns 1 if successful, -1 otherwise.
 */
int
create_image_output_file(const char *src, char *dest)
{
    if (!src || !dest) {
        errno = EINVAL;
        return -1;
    }

    FILE *input = fopen(src, "rb");
    FILE *output = fopen(dest, "wb");
    if (!input || !output)
        return -1;

    if (isbmp(input)) {
        struct bmp_fheader bfh;
        struct bmp_iheader bih;
        if (!read_bmpheaders(input, &bfh, &bih))
            return -1;        
        if (!create_bmp_file(input, &bfh, &bih))
            return -1;
    }

    fclose(input);
    fclose(output);

    return 1;
}

/** 
 * Updates the variables pointed to by width and height
 * with the dimensions of the image file pointed to by src.
 * Returns the image size in bytes if successful, -1 otherwise.
 **/
int
get_image_size(const char *src, size_t *width, size_t *height)
{
    if (!src || !width || !height) {
        errno = EINVAL;
        return -1;
    }
    
    FILE *fp = fopen(src, "rb");
    if (!fp) {
        return -1;
    }

    if (isbmp(fp)) {
        struct bmp_fheader bfh;
        struct bmp_iheader bih;
        
        if (!read_bmpheaders(fp, &bfh, &bih))
            return -1;
        

        print_bih(&bih);

        *width = bmp_width(&bih) + bmp_padding((int)bmp_width(&bih));
        *height = bih.imageHeight;

        printf("bmp_width: %lu, padding: %d\n", bmp_width(&bih), bmp_padding(bmp_width(&bih)));
        printf("width: %lu, height: %lu\n", *width, *height);
    }
    
    fclose(fp);

    return *width * *height;
}

/**
 * Allocates an unsigned char buffer of height * width bytes.
 * Returns a pointer to the new memory if successful, NULL otherwise.
 */
unsigned char *
allocate_image_buf(size_t height, size_t width)
{
    unsigned char *buf;
    buf = malloc(height * width); 
    return buf;
}

/**
 * Reads size bytes from the file pointed to by src
 * and writes them to the image buffer pointed to by dest.
 * Returns 1 if successful, -1 otherwise.
 **/
int
read_image(const char *src, unsigned char *dest, size_t size)
{
    if (!src || !dest) {
        errno = EINVAL;
        return -1;
    }

    FILE *fp = fopen(src, "rb");
    if (!fp)
        return -1;

    if (isbmp(fp)) {
        struct bmp_fheader bfh;
        struct bmp_iheader bih;
        if (!read_bmpheaders(fp, &bfh, &bih))
            return -1;
        
        print_bfh(&bfh);

        const int offset = bfh.offset;
        printf("bfh.offset = 0x%X\n", bfh.offset);
        fseek(fp, offset, SEEK_SET);
        
        printf("%ld\n", ftell(fp));

        fread(dest, size, 1, fp);
        swap_endian(dest, size);
        for (size_t i = 0; i < size; i++)
            printf("0x%X ", dest[i]);
    }
    
    fclose(fp);

    return 1;
}

/**
 * Writes size bytes from the image buffer pointed to by src
 * to the file pointed to by dest.
 * Returns 1 if successful, -1 otherwise.
 * NOTE: the image pointed to by src must already exist
 */
int
write_image(const unsigned char *src, char *dest, size_t size)
{
    if (!src || !dest) {
        errno = EINVAL;
        return -1;
    }

    FILE *fp = fopen(dest, "r+b");
    if (!fp)
        return -1;

    if (isbmp(fp)) {
        struct bmp_fheader bfh;    
        struct bmp_iheader bih;    
        if (!read_bmpheaders(fp, &bfh, &bih))
            return -1;
        if (!write_bmp_buf(fp, src, size))
            return -1;
    }
    fclose(fp);

    return 1;
}

/**
 * frees the memory pointed to by image and sets it to NULL
 */
void
free_image_buf(unsigned char *image)
{
    free(image); /* free(NULL) is valid */
    image = NULL;
}


/**
 * static functions start here
 */

/* it is a bmp if the first two bytes of the file are 0x4D42 */
static int
isbmp(FILE *fp)
{
    return read_ftype(fp) == 0x4D42;
}

/** 
 * the first two bytes of an image header specifies its filetype
 * return it. 
 */
static uint16_t
read_ftype(FILE *fp)
{
    if (!fp) {
        errno = EINVAL;
        return -1;
    }

    uint16_t ftype = 0;
    fread(&ftype, sizeof(ftype), 1, fp);
    rewind(fp);
    return ftype;
}

/* reads the bmp headers from the file pointed to by fp 
 * returns the fp to the begining of the gile when done
 */
static int
read_bmpheaders(FILE *fp, struct bmp_fheader *bfhead, 
                          struct bmp_iheader *bihead)
{
    if (!fp || !bfhead || !bihead) {
        errno = EINVAL;
        return -1;
    }

    fread(&bfhead->ftype, sizeof(uint16_t), 1, fp);
    fread(&bfhead->fsize, sizeof(uint32_t), 1, fp);
    fread(&bfhead->reserved1, sizeof(uint16_t), 1, fp);
    fread(&bfhead->reserved2, sizeof(uint16_t), 1, fp);
    fread(&bfhead->offset, sizeof(uint32_t), 1, fp);
    fread(bihead, BIHEADER_SIZE, 1, fp);
    rewind(fp);
    return 1;
}

/* writes the headers into the file pointed to by fp */
static int
create_bmp_file(FILE *fp, struct bmp_fheader *bfhead, 
                          struct bmp_iheader *bihead) 
{
    if (!fp || !bfhead || !bihead) {
        errno = EINVAL;
        return -1;
    }

    fwrite(&bfhead->ftype, sizeof(uint16_t), 1, fp);
    fwrite(&bfhead->fsize, sizeof(uint32_t), 1, fp);
    fwrite(&bfhead->reserved1, sizeof(uint16_t), 1, fp);
    fwrite(&bfhead->reserved2, sizeof(uint16_t), 1, fp);
    fwrite(&bfhead->offset, sizeof(uint32_t), 1, fp);
    fwrite(bihead, BIHEADER_SIZE, 1, fp);
    return 1;
}

/* swap the least-significant byte with the most-significant */
static int
swap_endian(unsigned char *image, size_t size)
{
    if (!image) {
        errno = EINVAL;
        return -1;
    }

    unsigned char temp;
    for (size_t i = 0; i < size; i += 3) {
        temp = image[i];
        image[i] = image[i + 2];
        image[i + 2] = temp;
    }

    return 1;
}

/** 
 * writes size bytes from the image buffer pointed to by image
 * into the stream pointed to by fp
 */
static int
write_bmp_buf(FILE *fp, const unsigned char *image, size_t size)
{
    if (!fp || !image) {
        errno = EINVAL;
        return -1;
    }

    const int offset = BFHEADER_SIZE + BIHEADER_SIZE;
    fseek(fp, offset, SEEK_SET);
    fwrite(image, size, 1, fp);
    return 1;
}
