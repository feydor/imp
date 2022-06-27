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
            U32Vec_push(&palette_buffer, n);
            tok = strtok(NULL, ",");
         }
      }
      fclose(palette_fp);
   } else {
      uint32_t default_palette[] = {0xf8f9fa,0xe9ecef,0xdee2e6,0xced4da,0xadb5bd,0x6c757d,0x495057,0x343a40,0x212529};
      U32Vec_from(&palette_buffer, default_palette, sizeof(default_palette) / sizeof(default_palette[0]));
   }

   printf("opening the source file: %s\n", src);
   FILE *fp = NULL;
   if (!(fp = fopen(src, "rb"))) {
      fprintf(stderr, "Input file not found: '%s'\n", src);
      return -1;
   }
   
   // read headers
   struct bmp_fheader bfheader;
   struct bmp_iheader biheader;
   read_bmp_headers(fp, &bfheader, &biheader);

   // print header info
   if (bfheader.ftype != BMP_MAGIC) {
      fprintf(stderr, "Not a BMP file: '%s'\n", src);
      return -1;
   }
   printf("image dimensions: %d x %d (w x h px)\n", biheader.width_px, biheader.height_px);
   printf("image size: %d bytes\n", biheader.image_size_bytes);

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
   for (size_t i = 0; i < biheader.image_size_bytes; i++) {
      // skip last two n bytes of every row (padding)
      if (i % total_bytes_x > total_bytes_x - padding - 1) continue;
      UCharVec_push(&raw_image, image_buffer[i]);
   }

   // manipulate vector
   if (flags) {
      for (size_t i = 0; i < strlen(flags); ++i) {
         char flag = flags[i];
         switch (flag) {
            case 'd':
               printf("performing ordered dithering...\n");
               ordered_dithering(raw_image.arr, UCharVec_size(&raw_image), biheader.width_px,
                  palette_buffer.arr, U32Vec_size(&palette_buffer));
               break;
            case 'g':
               printf("performing grayscale...\n");
               grayscale(raw_image.arr, UCharVec_size(&raw_image));
               break;
            case 'i':
               printf("performing invert...\n");
               invert(raw_image.arr, UCharVec_size(&raw_image));
               break;
            case 'n':
               printf("applying uniform noise...\n");
               add_uniform_bernoulli_noise(raw_image.arr, UCharVec_size(&raw_image));
               break;
            case 'p':
               printf("performing palette quantization...\n");
               palette_quantization(raw_image.arr, UCharVec_size(&raw_image),
                  palette_buffer.arr, U32Vec_size(&palette_buffer));
               break;
            default:
               printf("'%c' is not a valid flag\n", flag);
               break;
         }
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
   U32Vec_free(&palette_buffer);
   UCharVec_free(&raw_image);
   UCharVec_free(&image_with_padding);
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
