/* main.h - the program's UNIX interface*/
#ifndef MAIN_H
#define MAIN_H

#define ERR_FOPEN_INPUT  "fopen(input, r)"
#define ERR_FOPEN_OUTPUT "fopen(output, w)"
#define ERR_FREAD_INPUT "fread failed"
#define ERR_HANDLEIMAGE "handle_image blew up"
#define ERR_NOT_BMP "filetype is not a bmp"
#define ERR_MALLOC_NULL "malloc returned null"
#define DEFAULT_PROGNAME "imp"
    
#define OPTSTR "i:o:p:f:h"
#define USAGE_FMT "Usage: %s [OPTIONS] INPUT\n\nOptions:\n\
    -h, --help               Display this message\n    -f, --flags              Image processing passes\n\
    -i, --input              Input filename\n    -o, --output             Write output to filename\n\n\
Flags:\n     d     ordered dithering\n     g     grayscale\n     i     invert\n     n     uniform noise\n\
     p     palette quantization\n"

static struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"palette", required_argument, NULL, 'p'},
    {"flags", required_argument, NULL, 'f'},
    {"input", required_argument, NULL, 'i'},
    {"output", required_argument, NULL, 'o'},
    {NULL, 0, NULL, 0}
};

#endif

