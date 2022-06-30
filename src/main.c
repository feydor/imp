/* main.c - the program's UNIX interface */
#include "main.h"
#include "bmp.h"
#include "image.h"
#include "ui/imp_button.h"
#include "vector.h"
#include <assert.h>  /* for assert() */
#include <ctype.h>   /* for isdigit() */
#include <errno.h>   /* for external errno variable */
#include <libgen.h>  /* for basename() */
#include <stdbool.h> /* for boolean */
#include <stdint.h>  /* for uint32_t */
#include <stdio.h>   /* for FILE, fprint, fread, stdin, stdout, stderr */
#include <stdlib.h>  /* for stroul, exit() */
#include <string.h>  /* for memcpy, memset, strlen */
#include <string.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#define WINDOW_WIDTH 832
#define WINDOW_HEIGHT 640
#define UI_MARGIN_TOP 16
#define UI_MARGIN_RIGHT 16
#define UI_MARGIN_BOTTOM 64
#define UI_MARGIN_LEFT 64
#define BUTTON_SIZE 64
#define mouse_over_image(x, y, image_x, image_y, image_w, image_h)             \
    (x > image_x && x < image_x + image_w && y > image_y &&                    \
     y < image_y + image_h)
SDL_Window *window;
SDL_Renderer *renderer;
SDL_Surface *image_surface;
SDL_Texture *image_texture;
#define NUM_HORIZ_BUTTONS 10
ImpButton *horiz_button_bar[NUM_HORIZ_BUTTONS];

typedef struct {
    int x;
    int y;
} Point;

extern int errno;
extern char *optarg; /* for use with getopt() */
extern int opterr, optind;

static void usage(const char *progname) {
    fprintf(stderr, USAGE_FMT, progname ? progname : DEFAULT_PROGNAME);
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
            0x800000, 0x9A6324, 0x808000, 0x469990, 0x000075, 0x000000,
            0xe6194B, 0xf58231, 0xffe119, 0xbfef45, 0x3cb44b, 0x42d4f4,
            0x4363d8, 0x911eb4, 0xf032e6, 0xa9a9a9, 0xfabed4, 0xffd8b1,
            0xfffac8, 0xaaffc3, 0xdcbeff, 0xffffff};
        U32Vec_from(buffer, default_palette,
                    sizeof(default_palette) / sizeof(default_palette[0]));
    }
    return 0;
}


static void imagebuf_to_outputbuf(UCharVec *image_buffer, uchar *output_buffer,
                                  unsigned size_bytes, unsigned width_bytes,
                                  unsigned padding_bytes) {
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


void cleanup_sdl() {
    printf("cleaning up after SDL\n");
    SDL_DestroyTexture(image_texture);
    SDL_FreeSurface(image_surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}


void update_global_image_render(uchar *buffer, size_t width_px,
                                size_t height_px) {
    int depth = 24;
    int pitch = 3 * width_px;
    image_surface =
        SDL_CreateRGBSurfaceFrom(buffer, width_px, height_px, depth, pitch,
                                 0x0000FF, 0x00FF00, 0xFF0000, 0);
    image_texture = SDL_CreateTextureFromSurface(renderer, image_surface);
}


void create_button_bars() {
    // TODO: free these buttons at clean up (ImpButton_free)
    for (int i = 0; i < NUM_HORIZ_BUTTONS; ++i) {
      horiz_button_bar[i] = ImpButton_create(i * BUTTON_SIZE, WINDOW_HEIGHT - BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE);
      
      // TODO: make the filenames NOT relative to the executable
      char filename[80];
      sprintf(filename, "../res/icons/horiz-button1.bmp", i);
      horiz_button_bar[i]->surface = SDL_LoadBMP(filename);
    }

    horiz_button_bar[0]->task = IMP_SAVE;
    horiz_button_bar[1]->task = IMP_INVERT;
    horiz_button_bar[2]->task = IMP_GRAYSCALE;
    horiz_button_bar[3]->task = IMP_UNIFORM_NOISE;
    horiz_button_bar[4]->task = IMP_PALETTE_QUANTIZATION;
    horiz_button_bar[5]->task = IMP_ORDERED_DITHERING;
    horiz_button_bar[6]->task = IMP_SAVE;
}

void clicked_button_dispatch(ImpButtonTask task, size_t width_px,
                             uchar *image_buf, size_t image_bytes,
                             uint32_t *palette_buf, size_t palette_bytes) {
    switch (task) {
    case IMP_INVERT: {
        printf("performing invert...\n");
        invert(image_buf, image_bytes);
        break;
    } break;

    case IMP_GRAYSCALE: {
        printf("performing grayscale...\n");
        grayscale(image_buf, image_bytes);
    } break;

    case IMP_UNIFORM_NOISE: {
        printf("applying uniform noise...\n");
        add_uniform_bernoulli_noise(image_buf, image_bytes);
    } break;

    case IMP_ORDERED_DITHERING: {
        printf("performing ordered dithering...\n");
        ordered_dithering_single_channel(image_buf, image_bytes, width_px,
                                         palette_buf, palette_bytes);
    } break;

    case IMP_PALETTE_QUANTIZATION: {
        palette_quantization(image_buf, image_bytes, palette_buf,
                             palette_bytes);
    } break;
    case IMP_DISABLED: {
      printf("this button is disabled\n");
    }
    }
}

// Assumes Little-endian, 24bit bmp
// bmp data is stored starting at bottom-left corner
// flags and palette are optional
static int handle_image(const char *src, const char *dest, const char *flags,
                        const char *palette) {
    if (!src || !dest) {
        errno = EINVAL;
        return -1;
    }

    U32Vec palette_buffer;
    U32Vec_init(&palette_buffer);
    if (load_palette(&palette_buffer, palette) != 0) {
        return -1;
    }

    BMP_file bmp_file;
    if (BMP_load(&bmp_file, src) != 0) {
        return -1;
    }
    BMP_print_dimensions(&bmp_file);

    // copy image data to buffer, strip end of row padding
    UCharVec image_buffer;
    UCharVec_init(&image_buffer);
    int width_bytes = bmp_file.image_size_bytes / bmp_file.height_px;
    size_t padding = width_bytes - (bmp_file.width_px * 3);
    for (size_t i = 0; i < bmp_file.image_size_bytes; i++) {
        // skip last two n bytes of every row (padding)
        if (i % width_bytes > width_bytes - padding - 1)
            continue;
        UCharVec_push(&image_buffer, bmp_file.raw_image[i]);
    }

    // reverse the byte array (first pixel becomes last pixel, second becomes
    // second-to-last, etc) reversed_bmp_buffer is only used for rendering
    // purposes
    uchar reversed_bmp_buffer[bmp_file.image_size_bytes];
    size_t buf_size = UCharVec_size(&image_buffer);
    for (size_t i = 0; i < buf_size; ++i) {
        reversed_bmp_buffer[i] = UCharVec_get(&image_buffer, buf_size - 1 - i);
    }

    // then do row-wise swap
    for (size_t row = 0; row < bmp_file.height_px; ++row) {
        size_t bytes_per_row = 3 * bmp_file.width_px;
        uchar row_buf[bytes_per_row];
        for (size_t i = 0; i < bytes_per_row; ++i) {
            size_t end_of_row = ((row + 1) * bytes_per_row) - 1;
            row_buf[i] = reversed_bmp_buffer[end_of_row - i];
        }
        memcpy(reversed_bmp_buffer + (row * bytes_per_row), row_buf,
               bytes_per_row);
    }

    // write buffer to screen
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        exit(fprintf(stderr, "SDL could not be initialized.\nSDL error: %s\n",
                     SDL_GetError()));
    }

    window = SDL_CreateWindow("imp", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH,
                              WINDOW_HEIGHT, SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        exit(fprintf(stderr, "Could not create SDL renderer\n"));
    }

    update_global_image_render(reversed_bmp_buffer, bmp_file.width_px,
                               bmp_file.height_px);

    // create ui/buttons surfaces
    create_button_bars();

    if (atexit(cleanup_sdl) != 0) {
        fprintf(stderr, "atexit failed to register cleanup_sdl\n");
    }

    // TODO: keep image variables related to rendering in one place
    bool image_scroll_lock = false;
    Point clicked_distance = {0, 0};
    SDL_Rect image_rect = {UI_MARGIN_LEFT, UI_MARGIN_TOP, bmp_file.width_px,
                           bmp_file.height_px};

    SDL_Event event;
    while (1) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: {
                    exit(0);
                } break;
                }
            } break;

            case SDL_MOUSEMOTION: {
                if (image_scroll_lock) {
                    image_rect.x = event.motion.x - clicked_distance.x;
                    image_rect.y = event.motion.y - clicked_distance.y;
                }
            } break;

            case SDL_MOUSEBUTTONDOWN: {
                switch (event.button.button) {
                case SDL_BUTTON_LEFT: {
                    // check if any horizontal buttons are clicked
                    for (int i = 0; i < NUM_HORIZ_BUTTONS; ++i) {
                        ImpButton *button = horiz_button_bar[i];
                        if (ImpButton_mouseover(button, event.button.x,
                                                event.button.y)) {
                            clicked_button_dispatch(
                                button->task, bmp_file.width_px,
                                reversed_bmp_buffer, bmp_file.image_size_bytes,
                                palette_buffer.arr, palette_buffer.size);
                            update_global_image_render(reversed_bmp_buffer,
                                                       bmp_file.width_px,
                                                       bmp_file.height_px);
                        }
                    }

                    if (mouse_over_image(event.button.x, event.button.y,
                                         image_rect.x, image_rect.y,
                                         image_rect.w, image_rect.h) &&
                        !image_scroll_lock) {
                        image_scroll_lock = true;
                        clicked_distance.x = event.button.x - image_rect.x;
                        clicked_distance.y = event.button.y - image_rect.y;
                    }
                } break;
                }
            } break;

            case SDL_MOUSEBUTTONUP: {
                switch (event.button.button) {
                case SDL_BUTTON_LEFT: {
                    if (image_scroll_lock) {
                        image_scroll_lock = false;
                    }
                } break;
                }
            } break;

            case SDL_QUIT: {
                exit(0);
            } break;
            }
        }

        SDL_RenderClear(renderer);

        // render the working area
        SDL_SetRenderDrawColor(renderer, 0x01, 0x01, 0x01, 0);
        SDL_RenderFillRect(renderer,
                           &(SDL_Rect){UI_MARGIN_LEFT, 0,
                                       WINDOW_WIDTH - UI_MARGIN_LEFT,
                                       WINDOW_HEIGHT - UI_MARGIN_BOTTOM});

        // render the working image
        SDL_RenderCopy(renderer, image_texture, NULL,
                       &(SDL_Rect){image_rect.x, image_rect.y, image_rect.w,
                                   image_rect.h});

        // render the ui
        SDL_SetRenderDrawColor(renderer, 0xF7, 0xF7, 0XF7, 0);
        SDL_RenderFillRect(renderer,
                           &(SDL_Rect){UI_MARGIN_LEFT,
                                       WINDOW_HEIGHT - UI_MARGIN_BOTTOM,
                                       WINDOW_WIDTH, BUTTON_SIZE});
        SDL_RenderFillRect(renderer,
                           &(SDL_Rect){0, 0, BUTTON_SIZE, WINDOW_HEIGHT});
        for (int i = 0; i < NUM_HORIZ_BUTTONS; ++i) {
            ImpButton *button = horiz_button_bar[i];
            SDL_Texture *button_texture =
                SDL_CreateTextureFromSurface(renderer, button->surface);
            SDL_RenderCopy(
                renderer, button_texture, NULL,
                &(SDL_Rect){button->x, button->y, button->w, button->h});
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 60);
    }

    // manipulate buffer
    if (flags) {
        for (size_t i = 0; i < strlen(flags); ++i) {
            char flag = flags[i];
            switch (flag) {
            case 'd':
                printf("performing ordered dithering...\n");
                ordered_dithering_single_channel(
                    image_buffer.arr, UCharVec_size(&image_buffer),
                    bmp_file.width_px, palette_buffer.arr,
                    U32Vec_size(&palette_buffer));
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
                add_uniform_bernoulli_noise(image_buffer.arr,
                                            UCharVec_size(&image_buffer));
                break;
            case 'p':
                printf("performing palette quantization...\n");
                palette_quantization(
                    image_buffer.arr, UCharVec_size(&image_buffer),
                    palette_buffer.arr, U32Vec_size(&palette_buffer));
                break;
            default:
                printf("'%c' is not a valid flag\n", flag);
                break;
            }
        }
    }

    // back to output buffer, restore end of row padding
    imagebuf_to_outputbuf(&image_buffer, bmp_file.raw_image,
                          bmp_file.image_size_bytes, width_bytes, padding);

    // open the output file and write the headers followed by the image data
    if (BMP_write(&bmp_file, dest) != 0) {
        return -1;
    }

    U32Vec_free(&palette_buffer);
    UCharVec_free(&image_buffer);
    free(bmp_file.raw_image);
    return 0;
}


int main(int argc, char *argv[]) {
    srand(time(NULL));
    int opt = 0, option_index = 0;
    opterr = 0;
    char *src = NULL, *dest = NULL, *palette = NULL, *flags = NULL;

    while ((opt = getopt_long(argc, argv, OPTSTR, long_options,
                              &option_index)) != -1) {
        switch (opt) {
        case 'h':
            usage(basename(argv[0]));
            exit(0);
        case 'p':
            palette = optarg;
            break;
        case 'f':
            flags = optarg;
            break;
        case 'i':
            src = optarg;
            break;
        case 'o':
            dest = optarg;
            break;
        case '?':
            fprintf(stderr, "Unknown option %c\n", optopt);
            usage(basename(argv[0]));
            exit(0);
        case ':':
            fprintf(stderr, "Missing option for %c\n", optopt);
            usage(basename(argv[0]));
            exit(0);
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
        perror(ERR_HANDLEIMAGE);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
