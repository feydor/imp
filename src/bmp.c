#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"
#include "vector.h"

static BMP_error bmp_err = 0;


static int parse_headers(FILE *fp, BMP_file_header *file_header, BMP_info_header *info_header) {
   assert(INFOHEADER_SIZE == sizeof(BMP_info_header));
   fread(&file_header->ftype, sizeof(file_header->ftype), 1, fp);
   if (file_header->ftype != BMP_MAGIC) {
       bmp_err = NOT_A_BMP;
       return -1;
   }

   fread(&file_header->fsize, sizeof(file_header->fsize), 1, fp);
   fread(&file_header->reserved1, sizeof(file_header->reserved1), 1, fp);
   fread(&file_header->reserved2, sizeof(file_header->reserved2), 1, fp);
   fread(&file_header->offset, sizeof(file_header->offset), 1, fp);
   fread(info_header, sizeof(BMP_info_header), 1, fp);
   return 0;
}


// copies and horizontally flips the bytes of src into dest
// ie top-left pixel -> top-right pixel
static void buf_flip_horiz(uchar *dest, uchar *src, uint height, uint width_bytes) {
    assert(src && dest);
    for (size_t row = 0; row < height; ++row) {
        uchar row_buf[width_bytes];
        for (size_t i = 0; i < width_bytes; ++i) {
            size_t end_of_row = ((row + 1) * width_bytes) - 1;
            row_buf[i] = src[end_of_row - i];
        }
        memcpy(dest + (row * width_bytes), row_buf, width_bytes);
    }
}


static void BMP_print_error(const char *filename) {
    char *fmt_str;
    switch (bmp_err) {
        case NOT_A_BMP: fmt_str = "BMP_Error: Corrupt or unsupported filetype: %s\n"; break;
        case NOT_FOUND: fmt_str = "BMP_Error: File not found: %s\n"; break;
        case MALLOC_FAILED: fmt_str = "BMP_Error: Memory allocation failed: %s\n"; break;
        default: return;
    }
    fprintf(stderr, fmt_str, filename);
}


void BMP_print_dimensions(BMP_file *bmp) {
    printf("image dimensions: %d x %d (w x h px)\n", bmp->w, bmp->h);
    printf("image size: %d bytes\n", bmp->nbytes);
}


int BMP_load(BMP_file *bmp, const char *src) {
    assert(bmp && src);
    printf("loading from file: '%s'\n", src);
    FILE *fp = NULL;
    if (!(fp = fopen(src, "rb"))) {
        bmp_err = NOT_FOUND;
        BMP_print_error(src);
        return -1;
    }

    BMP_file_header *file_header = malloc(sizeof(BMP_file_header));
    BMP_info_header *info_header = malloc(sizeof(BMP_info_header));
    if (!file_header || !info_header) {
        bmp_err = MALLOC_FAILED;
        BMP_print_error(src);
        return -1;
    }

    if (parse_headers(fp, file_header, info_header)) {
        BMP_print_error(src);
        return -1;
    }

    bmp->nbytes = info_header->image_size_bytes;
    bmp->w = info_header->width_px;
    bmp->h = info_header->height_px;
    bmp->image_raw = calloc(bmp->nbytes, 1);
    bmp->image_render = calloc(bmp->nbytes, 1);
    bmp->file_header = file_header;
    bmp->info_header = info_header;
    if (!bmp->image_raw || !bmp->image_render) {
        bmp_err = MALLOC_FAILED;
        BMP_print_error(src);
        return -1;
    }

    // read the image data
    fseek(fp, file_header->offset, SEEK_SET);
    fread(bmp->image_raw, 1, bmp->nbytes, fp);

    // generate the raw image for buffer manipulation
    // copy image data to buffer, strip end of row padding
    UCharVec image_buffer;
    UCharVec_init(&image_buffer);
    unsigned width_bytes = bmp->nbytes / bmp->h;
    unsigned padding = width_bytes - 3 * bmp->w;
    for (size_t i = 0; i < bmp->nbytes; i++) {
        // skip last two n bytes of every row (padding)
        if (i % width_bytes > width_bytes - padding - 1)
            continue;
        UCharVec_push(&image_buffer, bmp->image_raw[i]);
    }
    memcpy(bmp->image_raw, image_buffer.arr, image_buffer.size);

    // generate the image used for IO
    // reverse the byte array (first pixel becomes last pixel, second becomes
    // second-to-last, etc) reversed_bmp_buffer is only used for rendering
    // purposes
    size_t buf_size = UCharVec_size(&image_buffer);
    BMP_reverse(bmp->image_render, image_buffer.arr, bmp->h, 3*bmp->w, buf_size);

    UCharVec_free(&image_buffer);
    fclose(fp);
    return 0;
}


int BMP_write(BMP_file *bmp, const char *dest) {
    assert(bmp && dest);
    assert(INFOHEADER_SIZE == sizeof(BMP_info_header));
    printf("writing to file: '%s'\n", dest);

    FILE *fp = NULL;
    if (!(fp = fopen(dest, "wb"))) {
        bmp_err = NOT_FOUND;
        BMP_print_error(dest);
        return -1;
    }

    fwrite(&bmp->file_header->ftype, sizeof(bmp->file_header->ftype), 1, fp);
    fwrite(&bmp->file_header->fsize, sizeof(bmp->file_header->fsize), 1, fp);
    fwrite(&bmp->file_header->reserved1, sizeof(bmp->file_header->reserved1), 1, fp);
    fwrite(&bmp->file_header->reserved2, sizeof(bmp->file_header->reserved2), 1, fp);
    fwrite(&bmp->file_header->offset, sizeof(bmp->file_header->offset), 1, fp);
    fwrite(bmp->info_header, sizeof(BMP_info_header), 1, fp);
    fseek(fp, bmp->file_header->offset, SEEK_SET);

    // add padding back to end of rows
    UCharVec image_with_padding;
    UCharVec_init(&image_with_padding);
    size_t width_bytes = bmp->nbytes / bmp->h;
    size_t padding = width_bytes - 3 * bmp->w;
    for (size_t i = 0; i < bmp->nbytes - (padding * bmp->h); ++i) {
        if (i != 0 && i % (width_bytes - padding) == 0) {
            for (size_t n = 0; n < padding; ++n) {
                UCharVec_push(&image_with_padding, 0x00);
            }
        }

        UCharVec_push(&image_with_padding, bmp->image_raw[i]);
    }

    // add EOF padding bytes
    for (size_t n = 0; n < padding; ++n) {
        UCharVec_push(&image_with_padding, 0x00);
    }

    fwrite(bmp->image_raw, 1, bmp->nbytes, fp);

    UCharVec_free(&image_with_padding);
    fclose(fp);
    return 0;
}


// bottom-left ->top-left
void BMP_reverse(uchar *dest, uchar *src, size_t height, size_t width_bytes, size_t nbytes) {
    for (size_t i = 0; i < nbytes; ++i) {
        dest[i] = src[nbytes - 1 - i];
    }

    buf_flip_horiz(dest, dest, height, width_bytes);
}

void BMP_free(BMP_file *bmp) {
    assert(bmp);
    free(bmp->image_raw);
    free(bmp->image_render);
    free(bmp->file_header);
    free(bmp->info_header);
    bmp->image_raw = NULL;
    bmp->image_render = NULL;
    bmp->file_header = NULL;
    bmp->info_header = NULL;
}
