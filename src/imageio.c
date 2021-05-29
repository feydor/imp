/* imageio.c - functions to read and write image files */
#include <assert.h> /* for assert */
#include <errno.h> /* for errno */
#include <stdio.h> /* for FILE, freas, fwrite */
#include <stdint.h> /* for uint16_t */
#include <stdlib.h> /* for malloc, exit */
#include <string.h> /* for memset */
#include "../include/imageio.h"
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
static int swap_colorendian(int32_t *image, size_t size);
static int write_bmp_buf(FILE *fp, const int8_t *image, size_t offset, size_t size);

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
        
        read_bmpheaders(fp, &bfh, &bih);

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
 * Allocates a buffer of height * width bytes.
 * Returns a pointer to the new memory if successful, NULL otherwise.
 * NOTE: Returned pointer must be cast to an appropriate type.
 */
int32_t *
allocate_image_buf(size_t size)
{
    return (int32_t *)malloc(size); 
}

/**
 * Reads size bytes from the file pointed to by src
 * and writes them to the image buffer pointed to by dest.
 * Returns 1 if successful, -1 otherwise.
 **/
int
read_image(const char *src, int32_t *dest, size_t size)
{
    if (!src || !dest) {
        errno = EINVAL;
        return -1;
    }

    FILE *fp = fopen(src, "rb");
    if (!fp)
        return -1;

    int8_t *tempbuf = malloc(size);

    if (isbmp(fp)) {
        /* parses bmp_headers to get the offset to the image data */
        struct bmp_fheader bfh = {0};    
        struct bmp_iheader bih = {0};    
        read_bmpheaders(fp, &bfh, &bih);
        print_bfh(&bfh);
        const int offset = bfh.offset;

        fseek(fp, offset, SEEK_SET);
        fread(tempbuf, size, 1, fp);

        pack(dest, tempbuf, size);
    }
    
    free(tempbuf);
    tempbuf = NULL;
    
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
write_image(int32_t *image, char *src, char *dest, size_t size)
{
    if (!image || !src || !dest) {
        errno = EINVAL;
        return -1;
    }

    FILE *in = fopen(src, "rb");
    FILE *out = fopen(dest, "wb");
    if (!in || !out)
        return -1;
    
    int8_t *tempbuf = malloc(size);

    if (isbmp(in)) {
        /* reads and then writes bmp_headers to the file at dest */
        struct bmp_fheader bfh = {0};    
        struct bmp_iheader bih = {0};    
        read_bmpheaders(in, &bfh, &bih);
        create_bmp_file(out, &bfh, &bih);

        unpack(tempbuf, image, size);
        write_bmp_buf(out, tempbuf, bfh.offset, size);
    }

    free(tempbuf);
    fclose(in);
    fclose(out);
    return 1;
}

/**
 * frees the memory pointed to by image and sets it to NULL
 */
void
free_image_buf(int32_t *image)
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
    assert(fp && "Is validated by the caller.");

    uint16_t ftype = 0;
    fread(&ftype, sizeof(ftype), 1, fp);
    rewind(fp);
    return ftype;
}

/* reads the bmp headers from the file pointed to by fp 
 * returns the fp to the beginning of the file when done
 */
static int
read_bmpheaders(FILE *fp, struct bmp_fheader *bfhead, 
                          struct bmp_iheader *bihead)
{
    assert(fp && "Is validated by the caller");

    fread(&bfhead->ftype, sizeof(uint16_t), 1, fp);
    fread(&bfhead->fsize, sizeof(uint32_t), 1, fp);
    fread(&bfhead->reserved1, sizeof(uint16_t), 1, fp);
    fread(&bfhead->reserved2, sizeof(uint16_t), 1, fp);
    fread(&bfhead->offset, sizeof(uint32_t), 1, fp);
    fread(bihead, sizeof(struct bmp_iheader), 1, fp);
    rewind(fp);
    return 1;
}

/* writes the headers into the file pointed to by fp */
static int
create_bmp_file(FILE *fp, struct bmp_fheader *bfhead, 
                          struct bmp_iheader *bihead) 
{
    assert(fp && "Is validated by the caller");

    fwrite(&bfhead->ftype, sizeof(uint16_t), 1, fp);
    fwrite(&bfhead->fsize, sizeof(uint32_t), 1, fp);
    fwrite(&bfhead->reserved1, sizeof(uint16_t), 1, fp);
    fwrite(&bfhead->reserved2, sizeof(uint16_t), 1, fp);
    fwrite(&bfhead->offset, sizeof(uint32_t), 1, fp);
    fwrite(bihead, sizeof(struct bmp_iheader), 1, fp);
    return 1;
}

/* swaps a BGR pixel to an RGB pixel, ignoring the leading unused byte */
static int
swap_colorendian(int32_t *image, size_t size)
{
    assert(image && "Is validated by the caller.");

    printf("swap_colorendian size: %lu\n", size);

    for (size_t i = 0; i < size / sizeof(int32_t); i++) {
        image[i] = swapbytes((uint32_t)image[i], 1, 3);
    }

    return 1;
}

/** 
 * writes size bytes starting from the offset of the image buffer
 * pointed to by image into the stream pointed to by fp
 */
static int
write_bmp_buf(FILE *fp, const int8_t *image, size_t offset, size_t size)
{
    assert(fp && "Is validated by the caller.");
    assert(image && "Is validated by the caller.");

    fseek(fp, offset, SEEK_SET);
    // fwrite(image, sizeof(image[0]), size / sizeof(image[0]), fp);
    fwrite(image, size, 1, fp);
    return 1;
}

/**
 * pack a byte array into a 4byte one creating a packed "pixel",
 * which follows the following pattern:
 * i0: 0x(bbggrr)(bb,
 * i1: 0xggrr)(bbgg,
 * i2: 0xrr)(bbggrr)
 * where the grouped bytes are RGB pixels and repeats indefinetely
 * NOTE: src must have a size divisble by 4
 */
int
pack(int32_t *dest, int8_t *src, size_t size) {
    size_t count = 0;
    const size_t PACKSIZE = 4;
    uint8_t b1 = 0, b2 = 0, b3 = 0, b4 = 0;
    printf("pack: size = %ld\n", size);
    
    for (size_t i = 0; i < size; i += PACKSIZE) {
        b1 = (uint8_t)(src[i] & 0xFF);
        b2 = (uint8_t)(src[i+1] & 0xFF);
        b3 = (uint8_t)(src[i+2] & 0xFF);
        b4 = (uint8_t)(src[i+3] & 0xFF);
        
        dest[count++] = (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;
    }
    printf("pack: dest_count = %d\n", count);
    return count;
}

int 
unpack(int8_t *dest, int32_t *src, size_t size)
{
    size_t count = 0;
    int8_t b1 = 0, b2 = 0, b3 = 0, b4 = 0;
    printf("unpack: size = %ld\n", size);
    
    for (size_t i = 0; i < size/4; ++i) {
        b4 = (int8_t)(src[i] & 0x000000FF);
        b3 = (int8_t)((src[i] & 0x0000FF00) >> 8);
        b2 = (int8_t)((src[i] & 0x00FF0000) >> 16);
        b1 = (int8_t)((src[i] & 0xFF000000) >> 24);

        dest[count++] = b1;
        dest[count++] = b2;
        dest[count++] = b3;
        dest[count++] = b4;
    }
    printf("unpack: dest_count = %d\n", count);
    return count;
}

