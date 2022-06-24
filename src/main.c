/* main.c - the program's UNIX interface */
#include <assert.h> /* for assert() */
#include <libgen.h> /* for basename() */
#include <getopt.h> /* for external optarg, opterr, optind, getopt() */
#include <stdio.h> /* for FILE, fprint, fread, stdin, stdout, stderr */
#include <errno.h> /* for external errno variable */
#include <string.h> /* for memcpy, memset, strlen */
#include <stdlib.h> /* for stroul, exit() */
#include <stdint.h> /* for uint32_t */
#include <stddef.h> /* for size_t */
#include "main.h"
#include "vector.h"

extern int errno;
extern char *optarg; /* for use with getopt() */
extern int opterr, optind;

static char *DEFAULT_SRC = "../lol.bmp";
static char *DEFAULT_DEST = "../RESULT.bmp";
static int BMP_MAGIC = 0x4D42;

void usage(char *progname) {
   fprintf(stderr, USAGE_FMT, progname ? progname : DEFAULT_PROGNAME);
}

void invert(uchar *buf, size_t size) {
   for (size_t i = 0; i < size; ++i)
      buf[i] = 255 - buf[i];
}

int rgb_to_gray(uchar r, uchar g, uchar b) {
   return (int) (0.2989 * r + 0.5870 * g + 0.1140 * b);
}

void grayscale(uchar *buf, size_t size_bytes) {
   size_t adjusted_end = size_bytes - (size_bytes % 3);
   for (size_t px = 0; px < adjusted_end; px += 3) {
      uchar gray = rgb_to_gray(buf[px + 2], buf[px + 1], buf[px]);
      buf[px + 2] = gray;
      buf[px + 1] = gray;
      buf[px] = gray;
   }
}

void print_buffer(uchar *buf, size_t size) {
   printf("--------------------------\n");
   for (size_t i = 0; i < size; ++i)
      printf("%#01X ", buf[i]);
   printf("\n--------------------------\n");
}

// Assumes Little-endian, 24bit bmp
// bmp data is stored starting at bottom-left corner
int handle_image(char *src, char *dest) {
   if (!src || !dest) {
      errno = EINVAL;
      return -1;
   }

   printf("src: %s, dest: %s\n", src, dest);
   FILE *fp = NULL;
   if (!(fp = fopen(src, "rb")))
      return -1;

   // first read bfheader bytes from file
   struct bmp_fheader bfheader;
   fread(&bfheader.ftype, sizeof(bfheader.ftype), 1, fp);   
   fread(&bfheader.fsize, sizeof(bfheader.fsize), 1, fp);
   fread(&bfheader.reserved1, sizeof(bfheader.reserved1), 1, fp);
   fread(&bfheader.reserved2, sizeof(bfheader.reserved2), 1, fp);
   fread(&bfheader.offset, sizeof(bfheader.offset), 1, fp);

   if (bfheader.ftype == BMP_MAGIC) printf("Your image is a BMP file.\n");

   // next read biheader bytes from file
   struct bmp_iheader biheader;
   assert(40 == sizeof(biheader));
   fread(&biheader, sizeof(biheader), 1, fp);
   printf("image dimensions: %d x %d (w x h px)\n", biheader.width_px, biheader.height_px);
   printf("image size: %d bytes\n", biheader.image_size_bytes);
   printf("offset to data: %#01X\n", bfheader.offset);

   fseek(fp, bfheader.offset, SEEK_SET);

   uchar image_buffer[biheader.image_size_bytes];
   fread(image_buffer, 1, biheader.image_size_bytes, fp);
   printf("width (including padding): %u bytes\n", biheader.image_size_bytes / biheader.height_px);

   // copy to vector, strip padding
   UCharVec raw_image;
   if (UCharVec_init(&raw_image) != 0) {
      perror("UCharVec_init failed");
      exit(EXIT_FAILURE);
   }

   int total_bytes_x = biheader.image_size_bytes / biheader.height_px;
   size_t padding = total_bytes_x - (biheader.width_px * 3);
   printf("padding: %ld bytes/row\n", padding);
   int padding_count = 0;
   for (size_t i = 0; i < biheader.image_size_bytes; i++) {
      // skip last two n bytes of every row (padding)
      if (i % total_bytes_x > total_bytes_x - padding - 1) {
         padding_count++;
         continue;
      }
      UCharVec_push(&raw_image, image_buffer[i]);
   }

   printf("counted %d bytes of padding\n", padding_count);
   printf("size of bytes without padding: %d bytes\n", UCharVec_size(&raw_image));

   // manipulate vector
   grayscale(raw_image.arr, UCharVec_size(&raw_image));

   // back to buffer, add padding
   UCharVec image_with_padding;
   if (UCharVec_init(&image_with_padding) != 0) {
      perror("UCharVec_init failed");
      exit(EXIT_FAILURE);
   }

   for (size_t i = 0; i < UCharVec_size(&raw_image); ++i) {
      // every xth byte, add back in padding
      if (i != 0 && i % (total_bytes_x - padding) == 0) {
         for (size_t n = 0; n < padding; ++n) {
            UCharVec_push(&image_with_padding, 0x00);
         }
      } 
      
      UCharVec_push(&image_with_padding, UCharVec_get(&raw_image, i));
   }

   // add EOF padding bytes
   for (size_t n = 0; n < padding; ++n) {
      UCharVec_push(&image_with_padding, 0x00);
   }
   printf("size of bytes with padding: %d bytes\n", UCharVec_size(&image_with_padding));

   UCharVec_copyto(&image_with_padding, image_buffer, biheader.image_size_bytes);

   // write image_buffer to dest file
   printf("writing to dest file: '%s'\n", dest);
   FILE *dest_fp = fopen(dest, "w");
   if (!dest_fp) return -1;

   // write bfheader and biheader
   fwrite(&bfheader.ftype, sizeof(bfheader.ftype), 1, dest_fp);   
   fwrite(&bfheader.fsize, sizeof(bfheader.fsize), 1, dest_fp);
   fwrite(&bfheader.reserved1, sizeof(bfheader.reserved1), 1, dest_fp);
   fwrite(&bfheader.reserved2, sizeof(bfheader.reserved2), 1, dest_fp);
   fwrite(&bfheader.offset, sizeof(bfheader.offset), 1, dest_fp);

   // NOTE: biheader is properly aligned, so no padding, so just write the whole struct
   fwrite(&biheader, sizeof(biheader), 1, dest_fp);

   // actually write the buffer now
   fseek(dest_fp, bfheader.offset, SEEK_SET);
   size_t bytes_written = fwrite(&image_buffer, 1, biheader.image_size_bytes, dest_fp);
   assert(bytes_written == biheader.image_size_bytes && "fwrite failed somehow");

   fclose(fp);
   fclose(dest_fp);
   UCharVec_free(&raw_image);
   UCharVec_free(&image_with_padding);
   return -1;
}

int main(int argc, char *argv[]) {
   int opt;
   opterr = 0;
   char *src = DEFAULT_SRC;
   char *dest = DEFAULT_DEST;

   while ((opt = getopt(argc, argv, OPTSTR)) != EOF)
      switch(opt) {
         case 'i': src = optarg; break;
         case 'o': dest = optarg; break;
         case 'h':
         default: usage(basename(argv[0])); exit(0);
      }

   if (handle_image(src, dest) != 0) {
      perror(ERR_HANDLEIMAGE); // error message is chosen based on value of errno before this
      exit(EXIT_FAILURE);
   }

   return EXIT_SUCCESS;
}

