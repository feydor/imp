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
#include <time.h>
#include "main.h"
#include "image.h"
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

void read_bmp_headers(FILE *fp, struct bmp_fheader *file_header, struct bmp_iheader *info_header) {
   assert(BIHEADER_SIZE == sizeof(struct bmp_iheader));
   fread(&file_header->ftype, sizeof(file_header->ftype), 1, fp);
   fread(&file_header->fsize, sizeof(file_header->fsize), 1, fp);
   fread(&file_header->reserved1, sizeof(file_header->reserved1), 1, fp);
   fread(&file_header->reserved2, sizeof(file_header->reserved2), 1, fp);
   fread(&file_header->offset, sizeof(file_header->offset), 1, fp);

   fread(info_header, sizeof(struct bmp_iheader), 1, fp);
}

int write_bmp(char *dest, struct bmp_fheader *file_header, struct bmp_iheader *info_header, uchar *buf) {
   printf("writing to file: '%s'\n", dest);
   FILE *dest_fp = fopen(dest, "w");
   if (!dest_fp) return -1;

   // actually write the buffer now
   assert(BIHEADER_SIZE == sizeof(struct bmp_iheader));
   fwrite(&file_header->ftype, sizeof(file_header->ftype), 1, dest_fp);
   fwrite(&file_header->fsize, sizeof(file_header->fsize), 1, dest_fp);
   fwrite(&file_header->reserved1, sizeof(file_header->reserved1), 1, dest_fp);
   fwrite(&file_header->reserved2, sizeof(file_header->reserved2), 1, dest_fp);
   fwrite(&file_header->offset, sizeof(file_header->offset), 1, dest_fp);
   fwrite(info_header, sizeof(struct bmp_iheader), 1, dest_fp);

   fseek(dest_fp, file_header->offset, SEEK_SET);
   fwrite(buf, 1, info_header->image_size_bytes, dest_fp);
   fclose(dest_fp);
   return 0;
}

// Assumes Little-endian, 24bit bmp
// bmp data is stored starting at bottom-left corner
int handle_image(char *src, char *dest, char *flags) {
   if (!src || !dest) {
      errno = EINVAL;
      return -1;
   }

   printf("opening the source file: %s\n", src);
   FILE *fp = NULL;
   if (!(fp = fopen(src, "rb")))
      return -1;
   
   // read headers
   struct bmp_fheader bfheader;
   struct bmp_iheader biheader;
   read_bmp_headers(fp, &bfheader, &biheader);

   // print header info
   if (bfheader.ftype != BMP_MAGIC) {
      errno = EINVAL;
      return -1;
   }

   printf("image dimensions: %d x %d (w x h px)\n", biheader.width_px, biheader.height_px);
   printf("image size: %d bytes\n", biheader.image_size_bytes);
   printf("width (including padding): %u bytes\n", biheader.image_size_bytes / biheader.height_px);

   // read the image bytes
   uchar image_buffer[biheader.image_size_bytes];
   fseek(fp, bfheader.offset, SEEK_SET);
   fread(image_buffer, 1, biheader.image_size_bytes, fp);
   
   // copy to vector, strip padding
   UCharVec raw_image;
   if (UCharVec_init(&raw_image) != 0) {
      perror("UCharVec_init failed");
      exit(EXIT_FAILURE);
   }

   int total_bytes_x = biheader.image_size_bytes / biheader.height_px;
   size_t padding = total_bytes_x - (biheader.width_px * 3);
   printf("padding: %ld bytes/row\n", padding);
   for (size_t i = 0; i < biheader.image_size_bytes; i++) {
      // skip last two n bytes of every row (padding)
      if (i % total_bytes_x > total_bytes_x - padding - 1) continue;
      UCharVec_push(&raw_image, image_buffer[i]);
   }

   // manipulate vector
   if (flags) {
      if (strstr(flags, "g") != NULL) {
         printf("performing grayscale...\n");
         grayscale(raw_image.arr, UCharVec_size(&raw_image));
      }

      if (strstr(flags, "i") != NULL) {
         printf("performing invert...\n");
         invert(raw_image.arr, UCharVec_size(&raw_image));
      }
   }

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

   UCharVec_copyto(&image_with_padding, image_buffer, biheader.image_size_bytes);

   // open the output file and write the headers followed by the image data
   if (write_bmp(dest, &bfheader, &biheader, image_buffer) != 0) {
      return -1;
   }

   fclose(fp);
   UCharVec_free(&raw_image);
   UCharVec_free(&image_with_padding);
   return 0;
}

int main(int argc, char *argv[]) {

   srand(time(NULL));
   int opt;
   opterr = 0;
   char *src = DEFAULT_SRC;
   char *dest = DEFAULT_DEST;
   char *flags = NULL;

   while ((opt = getopt(argc, argv, OPTSTR)) != EOF)
      switch(opt) {
         case 'i': src = optarg; break;
         case 'o': dest = optarg; break;
         case 'f': flags = optarg; break;
         case 'h':
         default: usage(basename(argv[0])); exit(0);
      }

   if (handle_image(src, dest, flags) != 0) {
      perror(ERR_HANDLEIMAGE); // error message is chosen based on value of errno before this
      exit(EXIT_FAILURE);
   }

   return EXIT_SUCCESS;
}
