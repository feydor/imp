#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"

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
    printf("image dimensions: %d x %d (w x h px)\n", bmp->width_px, bmp->height_px);
    printf("image size: %ld bytes\n", bmp->image_size_bytes);
}


int BMP_load(BMP_file *bmp, const char *src) {
    assert(bmp && src);
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

    bmp->image_size_bytes = info_header->image_size_bytes;
    bmp->width_px = info_header->width_px;
    bmp->height_px = info_header->height_px;
    bmp->raw_image = calloc(bmp->image_size_bytes, 1);
    bmp->file_header = file_header;
    bmp->info_header = info_header;
    if (!bmp->raw_image) {
        bmp_err = MALLOC_FAILED;
        BMP_print_error(src);
        return -1;
    }

    // read the image data
    fseek(fp, file_header->offset, SEEK_SET);
    fread(bmp->raw_image, 1, bmp->image_size_bytes, fp);

    fclose(fp);
    return 0;
}


int BMP_write(BMP_file *bmp, const char *dest) {
    assert(bmp && dest);
    assert(INFOHEADER_SIZE == sizeof(BMP_info_header));

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
    fwrite(bmp->raw_image, 1, bmp->image_size_bytes, fp);

    fclose(fp);
    return 0;
}


void BMP_free(BMP_file *bmp) {
    assert(bmp);
    free(bmp->raw_image);
    free(bmp->file_header);
    free(bmp->info_header);
    bmp->raw_image = NULL;
    bmp->file_header = NULL;
    bmp->info_header = NULL;
}


// int main(int argc, char *argv[]) {
//     if (argc != 2) {
//         printf("requires an argument\n");
//         return -1;
//     }
//     BMP_file file;
//     BMP_load(&file, argv[1]);
//     BMP_print_dimensions(&file);
//     BMP_write(&file, "test.bmp");
//     BMP_free(&file);
//     return 0;
// }