/* main.c - the program's UNIX interface */
#include <libgen.h> /* for basename() */
#include <getopt.h> /* for external optarg, opterr, optind, getopt() */
#include <stdio.h> /* for FILE, fprint, fread, stdin, stdout, stderr */
#include <errno.h> /* for external errno variable */
#include <string.h> /* for memcpy, memset, strlen */
#include <stdlib.h> /* for stroul, exit() */
#include "../include/main.h"
#include "../include/imagehandler.h"

extern int errno;
extern char *optarg; /* for use with getopt() */
extern int opterr, optind;

static void usage(char *progname, int opt);

int main(int argc, char *argv[]) {
    int opt;
    options_t options = { 0, 0x0, NULL, NULL, stdin, stdout };

    opterr = 0;

    while ((opt = getopt(argc, argv, OPTSTR)) != EOF)
       switch(opt) {
           case 'i':
              options.iname = optarg;
              break;

           case 'o':
              options.oname = optarg;
              break;
             
           case 'f':
              options.flags = (uint32_t ) strtoul(optarg, NULL, 16);
              break;

           case 'v':
              options.verbose += 1;
              break;

           case 'h':
           default:
              usage(basename(argv[0]), opt);
              /* NOTREACHED */
              break;
       }
    
    if (!handle_image(&options)) {
        perror(ERR_SAD_EXIT);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

void usage(char *progname, int opt) {
   (void) opt;
   fprintf(stderr, USAGE_FMT, progname ? progname : DEFAULT_PROGNAME);
   exit(EXIT_FAILURE);
}

