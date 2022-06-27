/* main.c - the program's UNIX interface */
#include <assert.h> /* for assert() */
#include <ctype.h> /* for isdigit() */
#include <libgen.h> /* for basename() */
#include <getopt.h> /* for external optarg, opterr, optind, getopt() */
#include <stdio.h> /* for FILE, fprint, fread, stdin, stdout, stderr */
#include <errno.h> /* for external errno variable */
#include <string.h> /* for memcpy, memset, strlen */
#include <stdlib.h> /* for stroul, exit() */
#include <stdint.h> /* for uint32_t */
#include <stddef.h> /* for size_t */
#include <string.h>
#include <time.h>
#include "main.h"
#include "image.h"
#include "vector.h"

extern int errno;
extern char *optarg; /* for use with getopt() */
extern int opterr, optind;
static int BMP_MAGIC = 0x4D42;

static void usage(const char *progname) {
   fprintf(stderr, USAGE_FMT, progname ? progname : DEFAULT_PROGNAME);
}

static void read_bmp_headers(FILE *fp, struct bmp_fheader *file_header, struct bmp_iheader *info_header) {
   assert(BIHEADER_SIZE == sizeof(struct bmp_iheader));
   fread(&file_header->ftype, sizeof(file_header->ftype), 1, fp);
   fread(&file_header->fsize, sizeof(file_header->fsize), 1, fp);
   fread(&file_header->reserved1, sizeof(file_header->reserved1), 1, fp);
   fread(&file_header->reserved2, sizeof(file_header->reserved2), 1, fp);
   fread(&file_header->offset, sizeof(file_header->offset), 1, fp);

   fread(info_header, sizeof(struct bmp_iheader), 1, fp);
}

static int write_bmp(const char *dest, struct bmp_fheader *file_header, struct bmp_iheader *info_header, const uchar *buf) {
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

/**
 * @brief loads the <palette>, if not found loads a default
 */
static int load_palette(U32Vec *buffer, const char *palette) {
   if (palette) {
      FILE *palette_fp = NULL;
      if (!(palette_fp = fopen(palette, "r"))) {
         fprintf(stderr, "Palette file not found: '%s'\n", palette);
         return -1;
      }

      int MAX_CHARS = 1024;
      char line[MAX_CHARS];
      while (fgets(line, MAX_CHARS, palette_fp)) {
         char *tok = NULL;
         tok = strtok(line, ",");
         while (tok && isalnum(*tok)) {
            uint32_t n = strtoul(tok, NULL, 16);
            U32Vec_push(buffer, n);
            tok = strtok(NULL, ",");
         }
      }
      fclose(palette_fp);
   } else {
      uint32_t default_palette[] = {
         0x800000,0x9A6324,0x808000,0x469990,0x000075,
         0x000000,0xe6194B,0xf58231,0xffe119,0xbfef45,
         0x3cb44b,0x42d4f4,0x4363d8,0x911eb4,0xf032e6,
         0xa9a9a9,0xfabed4,0xffd8b1,0xfffac8,0xaaffc3,
         0xdcbeff,0xffffff
      };
      U32Vec_from(buffer, default_palette, sizeof(default_palette) / sizeof(default_palette[0]));
   }
   return 0;
}

static int load_image(UCharVec *image_buffer, struct bmp_fheader *bfheader, struct bmp_iheader *biheader, const char *src) {
   printf("opening the source file: %s\n", src);
   FILE *fp = NULL;
   if (!(fp = fopen(src, "rb"))) {
      fprintf(stderr, "Input file not found: '%s'\n", src);
      return -1;
   }

   // read the headers
   read_bmp_headers(fp, bfheader, biheader);
   if (bfheader->ftype != BMP_MAGIC) {
      fprintf(stderr, "Not a BMP file: '%s'\n", src);
      return -1;
   }
   printf("image dimensions: %d x %d (w x h px)\n", biheader->width_px, biheader->height_px);
   printf("image size: %d bytes\n", biheader->image_size_bytes);

   // read the image data
   uchar raw_image_data[biheader->image_size_bytes];
   fseek(fp, bfheader->offset, SEEK_SET);
   fread(raw_image_data, 1, biheader->image_size_bytes, fp);

   // copy image data to buffer, strip the end of row padding
   int total_bytes_x = biheader->image_size_bytes / biheader->height_px;
   size_t padding = total_bytes_x - (biheader->width_px * 3);
   for (size_t i = 0; i < biheader->image_size_bytes; i++) {
      // skip last two n bytes of every row (padding)
      if (i % total_bytes_x > total_bytes_x - padding - 1) continue;
      UCharVec_push(image_buffer, raw_image_data[i]);
   }

   return 0;
}

static void imagebuf_to_outputbuf(UCharVec *image_buffer, uchar *output_buffer, unsigned size_bytes, unsigned width_bytes, unsigned padding_bytes) {
   UCharVec image_with_padding;
   UCharVec_init(&image_with_padding);

   for (size_t i = 0; i < UCharVec_size(image_buffer); ++i) {
      if (i != 0 && i % (width_bytes - padding_bytes) == 0) {
         for (size_t n = 0; n < padding_bytes; ++n) {
            UCharVec_push(&image_with_padding, 0x00);
         }
      } 
      
      UCharVec_push(&image_with_padding, UCharVec_get(image_buffer, i));
   }

   // add EOF padding bytes
   for (size_t n = 0; n < padding_bytes; ++n) {
      UCharVec_push(&image_with_padding, 0x00);
   }

   UCharVec_copyto(&image_with_padding, output_buffer, size_bytes);
   UCharVec_free(&image_with_padding);
}

// Assumes Little-endian, 24bit bmp
// bmp data is stored starting at bottom-left corner
// flags and palette are optional
static int handle_image(const char *src, const char *dest, const char *flags, const char *palette) {
   if (!src || !dest) {
      errno = EINVAL;
      return -1;
   }

   U32Vec palette_buffer;
   U32Vec_init(&palette_buffer);
   if (load_palette(&palette_buffer, palette) != 0) {
      return -1;
   }
   UCharVec image_buffer;
   struct bmp_fheader bfheader;
   struct bmp_iheader biheader;
   UCharVec_init(&image_buffer);
   if (load_image(&image_buffer, &bfheader, &biheader, src) != 0) {
      return -1;
   }

   // manipulate buffer
   if (flags) {
      for (size_t i = 0; i < strlen(flags); ++i) {
         char flag = flags[i];
         switch (flag) {
            case 'd':
               printf("performing ordered dithering...\n");
               ordered_dithering(image_buffer.arr, UCharVec_size(&image_buffer), biheader.width_px,
                  palette_buffer.arr, U32Vec_size(&palette_buffer));
               break;
            case 'g':
               printf("performing grayscale...\n");
               grayscale(image_buffer.arr, UCharVec_size(&image_buffer));
               break;
            case 'i':
               printf("performing invert...\n");
               invert(image_buffer.arr, UCharVec_size(&image_buffer));
               break;
            case 'n':
               printf("applying uniform noise...\n");
               add_uniform_bernoulli_noise(image_buffer.arr, UCharVec_size(&image_buffer));
               break;
            case 'p':
               printf("performing palette quantization...\n");
               palette_quantization(image_buffer.arr, UCharVec_size(&image_buffer),
                  palette_buffer.arr, U32Vec_size(&palette_buffer));
               break;
            default:
               printf("'%c' is not a valid flag\n", flag);
               break;
         }
      }
   }

   // back to output buffer, restore padding
   uchar output_buffer[biheader.image_size_bytes];
   int width_bytes = biheader.image_size_bytes / biheader.height_px;
   int padding_bytes = width_bytes - (biheader.width_px * 3);
   imagebuf_to_outputbuf(&image_buffer, output_buffer, biheader.image_size_bytes, width_bytes, padding_bytes);
   
   // open the output file and write the headers followed by the image data
   if (write_bmp(dest, &bfheader, &biheader, output_buffer) != 0) {
      return -1;
   }

   U32Vec_free(&palette_buffer);
   UCharVec_free(&image_buffer);
   return 0;
}

int main(int argc, char *argv[]) {
   srand(time(NULL));
   int opt = 0, option_index = 0;
   opterr = 0;
   char *src = NULL, *dest = NULL, *palette = NULL, *flags = NULL;

   while ((opt = getopt_long(argc, argv, OPTSTR, long_options, &option_index)) != -1) {
      switch(opt) {
         case 'h': usage(basename(argv[0])); exit(0);
         case 'p': palette = optarg; break;
         case 'f': flags = optarg; break;
         case 'i': src = optarg; break;
         case 'o': dest = optarg; break;
         case '?':
            fprintf(stderr, "Unknown option %c\n", optopt);
            usage(basename(argv[0])); exit(0);
         case ':':
            fprintf(stderr, "Missing option for %c\n", optopt);
            usage(basename(argv[0])); exit(0);
         default:
            fprintf(stderr, "getopt returned character code 0%o\n", opt);
            exit(-1);
      }
   }

   // get non-option arguments, ie the filename
   while (optind < argc) {
      src = argv[optind++];
   }

   if (!src || !dest) {
      usage(basename(argv[0]));
      exit(0);
   }

   if (handle_image(src, dest, flags, palette) != 0) {
      perror(ERR_HANDLEIMAGE); // error message is chosen based on value of errno before this
      exit(EXIT_FAILURE);
   }

   return EXIT_SUCCESS;
}
