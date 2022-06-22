/* main.c - the program's UNIX interface */
#include <libgen.h> /* for basename() */
#include <getopt.h> /* for external optarg, opterr, optind, getopt() */
#include <stdio.h> /* for FILE, fprint, fread, stdin, stdout, stderr */
#include <errno.h> /* for external errno variable */
#include <string.h> /* for memcpy, memset, strlen */
#include <stdio.h> /* for FILE */
#include <stdlib.h> /* for stroul, exit() */
#include <stdint.h> /* for uint32_t */
#include <stddef.h> /* for size_t */
#include "main.h"

extern int errno;
extern char *optarg; /* for use with getopt() */
extern int opterr, optind;

static char *DEFAULT_SRC = "../res/simple.bmp";
static char *DEFAULT_DEST = "../result.bmp";
static int BMP_MAGIC = 0x4D42;

void usage(char *progname) {
   fprintf(stderr, USAGE_FMT, progname ? progname : DEFAULT_PROGNAME);
}

// Assumes Little-endian
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

   if (bfheader.ftype == BMP_MAGIC)
      printf("Your image is a BMP file.\n");
   
   fread(&bfheader.fsize, sizeof(bfheader.fsize), 1, fp);
   fread(&bfheader.reserved1, sizeof(bfheader.reserved1), 1, fp);
   fread(&bfheader.reserved2, sizeof(bfheader.reserved2), 1, fp);
   fread(&bfheader.offset, sizeof(bfheader.offset), 1, fp);

   // second read biheader bytes from file
   // NOTE: Assuming biheader follows immediately from bfheader
   // struct bmp_iheader biheader;
   
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

