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
#define BUTTON_MARGIN 5
#define MOUSE_ZOOM_RATE 0.1f
#define MIN_VIEW_SIZE 5
#define mouse_over_image(x, y, image_x, image_y, image_w, image_h)             \
    (x > image_x && x < image_x + image_w && y > image_y &&                    \
     y < image_y + image_h)
SDL_Window *window;
SDL_Renderer *renderer;
SDL_Surface *image_surface;
SDL_Texture *image_texture;
SDL_Surface *bg_surface;
SDL_Texture *bg_texture;
#define NUM_HORIZ_BUTTONS 10
ImpButton *horiz_button_bar[NUM_HORIZ_BUTTONS];

typedef struct {
    int x;
    int y;
} Point;

typedef enum {
    IMP_CURSOR,
    IMP_PENCIL,
} ImpMouseMode;
ImpMouseMode imp_mousemode = IMP_PENCIL;

extern int errno;
extern char *optarg; /* for use with getopt() */
extern int opterr, optind;
char *SRC = NULL, *DEST = NULL;

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

void create_button_bars() {
    for (int i = 0; i < NUM_HORIZ_BUTTONS; ++i) {
        horiz_button_bar[i] =
            ImpButton_create(i * BUTTON_SIZE + BUTTON_MARGIN + 2*i*BUTTON_MARGIN,
                             WINDOW_HEIGHT - BUTTON_SIZE - BUTTON_MARGIN,
                             BUTTON_SIZE, BUTTON_SIZE);

        // TODO: make the filenames NOT relative to the executable
        char filename[80];
        sprintf(filename, "../res/icons/horiz-button1.bmp");
        horiz_button_bar[i]->surface = SDL_LoadBMP(filename);
        sprintf(filename, "../res/icons/horiz-button1-click.bmp");
        horiz_button_bar[i]->clicked = SDL_LoadBMP(filename);
    }

    horiz_button_bar[0]->task = IMP_SAVE;
    horiz_button_bar[0]->surface = SDL_LoadBMP("../res/icons/horiz-button0.bmp");
    horiz_button_bar[1]->task = IMP_INVERT;
    horiz_button_bar[2]->task = IMP_GRAYSCALE;
    horiz_button_bar[3]->task = IMP_UNIFORM_NOISE;
    horiz_button_bar[4]->task = IMP_HORIZ_FLIP;
}


void cleanup_sdl() {
    printf("cleaning up after SDL\n");
    SDL_DestroyTexture(image_texture);
    SDL_FreeSurface(image_surface);
    SDL_FreeSurface(bg_surface);
    SDL_DestroyTexture(bg_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}


void cleanup_imp_ui() {
    for (int i = 0; i < NUM_HORIZ_BUTTONS; ++i) {
        ImpButton_free(horiz_button_bar[i]);
    }
}


void update_global_image_render(uchar *buffer, size_t width_px,
                                size_t height_px) {
    int depth = 24;
    int pitch = 3 * width_px;
    uint32_t rmask, gmask, bmask;
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0x0000FF;
        gmask = 0x00FF00;
        bmask = 0xFF0000;
    #else // little endian
        rmask = 0xFF0000;
        gmask = 0x00FF00;
        bmask = 0x0000FF;
    #endif

    if (image_surface != NULL) {
        SDL_FreeSurface(image_surface);
        image_surface = NULL;
    }

    if (image_texture != NULL) {
        SDL_DestroyTexture(image_texture);
        image_texture = NULL;
    }

    image_surface =
        SDL_CreateRGBSurfaceFrom(buffer, width_px, height_px, depth, pitch,
                                 rmask, gmask, bmask, 0);
    image_texture = SDL_CreateTextureFromSurface(renderer, image_surface);
}


void clicked_button_dispatch(ImpButtonTask task, BMP_file *bmp,
                             uint32_t *palette_buf, size_t palette_bytes) {
    switch (task) {
    case IMP_SAVE: {
        printf("saving to file: %s\n", DEST);
        BMP_write(bmp, DEST);
    } break;
    case IMP_INVERT: {
        invert(bmp->image_raw, bmp->size_bytes);
        break;
    } break;
    case IMP_GRAYSCALE: {
        grayscale(bmp->image_raw, bmp->size_bytes);
    } break;
    case IMP_UNIFORM_NOISE: {
        add_uniform_bernoulli_noise(bmp->image_raw, bmp->size_bytes);
    } break;
    case IMP_ORDERED_DITHERING: {
        printf("performing ordered dithering...\n");
        ordered_dithering_single_channel(bmp->image_raw, bmp->size_bytes, bmp->w,
                                         palette_buf, palette_bytes);
    } break;
    case IMP_PALETTE_QUANTIZATION: {
        printf("performing palette quantization...\n");
        palette_quantization(bmp->image_raw, bmp->size_bytes, palette_buf,
                             palette_bytes);
    } break;
    case IMP_HORIZ_FLIP: {
        buf_flip_horiz(bmp->image_raw, bmp->image_raw, bmp->h, bmp->w * 3);
    } break;
    case IMP_DISABLED: {
      printf("this button is disabled\n");
    } break;
    }
}

static int sdl_ui(BMP_file *bmp, U32Vec *palette) {
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
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    update_global_image_render(bmp->image_raw, bmp->w, bmp->h);
    create_button_bars();
    bg_surface = SDL_LoadBMP("../res/patterns/gray-brick.bmp");
    bg_texture = SDL_CreateTextureFromSurface(renderer, bg_surface);

    if (atexit(cleanup_sdl) != 0 && atexit(cleanup_imp_ui) != 0) {
        fprintf(stderr, "atexit failed to register cleanup_sdl\n");
    }

    // TODO: keep image variables related to rendering in one place
    bool image_scroll_lock = false;
    Point clicked_distance = {0, 0};
    SDL_Rect image_rect = {0, 0, bmp->w, bmp->h};
    int zoom_level = 0;
    int dx_zoom = 0, dy_zoom = 0;

    uint32_t curr_pencil_color = 0xFF0000;

    SDL_Event event;
    while (1) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: return 0;
                }
            } break;

            case SDL_MOUSEMOTION: {
                if (image_scroll_lock) {
                    image_rect.x = event.motion.x - clicked_distance.x;
                    image_rect.y = event.motion.y - clicked_distance.y;
                } else if (imp_mousemode == IMP_PENCIL &&
                           (event.motion.state & SDL_BUTTON_LMASK) != 0) {
                    float drift = sqrt(pow(dx_zoom, 2) + pow(dy_zoom, 2));
                    int xrel = event.motion.x - image_rect.x - drift;
                    int yrel = event.motion.y - image_rect.y - drift;

                    for (int i = 0; i < 4; ++i)
                        for (int j = 0; j < 4; j++)
                            BMP_set_pixel(bmp, 3*j + (xrel*3), yrel + i, curr_pencil_color);
                    update_global_image_render(bmp->image_raw, bmp->w, bmp->h);
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
                            button->pressed = true;
                            clicked_button_dispatch(button->task, bmp,
                                                    palette->arr,
                                                    palette->size);
                            update_global_image_render(bmp->image_raw, bmp->w, bmp->h);
                        }
                    }

                    if (mouse_over_image(event.button.x, event.button.y, image_rect.x, image_rect.y,
                                         image_rect.w, image_rect.h)) {

                        if (imp_mousemode == IMP_CURSOR && !image_scroll_lock) {
                            image_scroll_lock = true;
                            clicked_distance.x = event.button.x - image_rect.x;
                            clicked_distance.y = event.button.y - image_rect.y;
                        } else if (imp_mousemode == IMP_PENCIL) {
                            // TODO
                        }
                    }
                } break;
                case SDL_BUTTON_RIGHT: {
                    if (mouse_over_image(event.button.x, event.button.y, image_rect.x, image_rect.y,
                                         image_rect.w, image_rect.h)) {

                        if (imp_mousemode == IMP_PENCIL && !image_scroll_lock) {
                            image_scroll_lock = true;
                            clicked_distance.x = event.button.x - image_rect.x;
                            clicked_distance.y = event.button.y - image_rect.y;
                        }
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

                    for (int i = 0; i < NUM_HORIZ_BUTTONS; ++i) {
                        horiz_button_bar[i]->pressed = false;
                    }
                } break;
                case SDL_BUTTON_RIGHT: {
                    if (image_scroll_lock) {
                        image_scroll_lock = false;
                    }
                } break;
                }
            } break;

            case SDL_MOUSEWHEEL: {
                if (event.wheel.y > 0) {
                    int dy = fmax(image_rect.h * MOUSE_ZOOM_RATE, MIN_VIEW_SIZE);
                    int dx = fmax(image_rect.w * MOUSE_ZOOM_RATE, MIN_VIEW_SIZE);
                    image_rect.h += dy;
                    image_rect.w += dx;
                    dx_zoom = dx;
                    dy_zoom = dy;
                    zoom_level += 1;
                } else if (event.wheel.y < 0) {
                    int dy = fmax(image_rect.h * MOUSE_ZOOM_RATE, MIN_VIEW_SIZE);
                    int dx = fmax(image_rect.w * MOUSE_ZOOM_RATE, MIN_VIEW_SIZE);
                    image_rect.h -= dy;
                    image_rect.w -= dx;
                    dx_zoom = dx;
                    dy_zoom = dy;
                    zoom_level -= 1;
                }
            } break;

            case SDL_QUIT: return 0;
            }
        }

        SDL_RenderClear(renderer);

        // render the working area
        SDL_SetRenderDrawColor(renderer, 0x33, 0x33, 0x33, 255);
        SDL_RenderFillRect(renderer, &(SDL_Rect){0, 0, WINDOW_WIDTH, WINDOW_HEIGHT});

        // render the working image's shadow
        const int shadowoff = 1;
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &(SDL_Rect){image_rect.x + shadowoff, image_rect.y + shadowoff, image_rect.w + shadowoff, image_rect.h + shadowoff});

        // render the working image
        SDL_RenderCopy(renderer, image_texture, NULL, &image_rect);

        // render the ui
        for (int i = 0; i < NUM_HORIZ_BUTTONS; ++i) {
            ImpButton *button = horiz_button_bar[i];
            SDL_Texture *button_txt = button->pressed
                                          ? SDL_CreateTextureFromSurface(renderer, button->clicked)
                                          : SDL_CreateTextureFromSurface(renderer, button->surface);

            SDL_RenderCopy(renderer, button_txt, NULL,
                           &(SDL_Rect){button->x, button->y, button->w, button->h});

            // print transparent overlay over disabled buttons
            if (button->task == IMP_DISABLED) {
                SDL_SetRenderDrawColor(renderer, 0x01, 0x01, 0x01, 200);
                SDL_RenderFillRect(renderer, &(SDL_Rect){button->x, button->y, button->w, button->h});
            }

            SDL_DestroyTexture(button_txt);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 60);
    }
}


int cli_ui(BMP_file *bmp, U32Vec *palette, const char *flags) {
    // manipulate buffer based on flags
    if (flags) {
        for (size_t i = 0; i < strlen(flags); ++i) {
            char flag = flags[i];
            switch (flag) {
            case 'd':
                printf("performing ordered dithering...\n");
                ordered_dithering_single_channel(
                    bmp->image_raw, bmp->size_bytes,
                    bmp->w, palette->arr,
                    U32Vec_size(palette));
                break;
            case 'g':
                printf("performing grayscale...\n");
                grayscale(bmp->image_raw, bmp->size_bytes);
                break;
            case 'i':
                printf("performing invert...\n");
                invert(bmp->image_raw, bmp->size_bytes);
                break;
            case 'n':
                printf("applying uniform noise...\n");
                add_uniform_bernoulli_noise(bmp->image_raw,
                                            bmp->size_bytes);
                break;
            case 'p':
                printf("performing palette quantization...\n");
                palette_quantization(
                    bmp->image_raw, bmp->size_bytes,
                    palette->arr, U32Vec_size(palette));
                break;
            default:
                printf("'%c' is not a valid flag\n", flag);
                break;
            }
        }
    }

    if (BMP_write(bmp, DEST) != 0) {
        return -1;
    }

    return 0;
}


// Assumes Little-endian, 24bit bmp
// bmp data is stored starting at bottom-left corner
// flags and palette are optional
int main(int argc, char *argv[]) {
    srand(time(NULL));
    int opt = 0, option_index = 0;
    opterr = 0;
    char *palette = NULL, *flags = NULL;
    bool cli = false;

    while ((opt = getopt_long(argc, argv, OPTSTR, long_options,
                              &option_index)) != -1) {
        switch (opt) {
        case 'h':
            usage(basename(argv[0]));
            exit(0);
        case 'c': cli = true; break;
        case 'p': palette = optarg; break;
        case 'f': flags = optarg; break;
        case 'i': SRC = optarg; break;
        case 'o': DEST = optarg; break;
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
        SRC = argv[optind++];
    }

    if (!SRC || !DEST) {
        usage(basename(argv[0]));
        exit(0);
    }

    U32Vec palette_buffer;
    U32Vec_init(&palette_buffer);
    if (load_palette(&palette_buffer, palette) != 0) {
        return -1;
    }

    BMP_file bmp;
    if (BMP_load(&bmp, SRC) != 0) {
        return -1;
    }
    BMP_print_dimensions(&bmp);

    int ret_code = cli ? cli_ui(&bmp, &palette_buffer, flags)
                       : sdl_ui(&bmp, &palette_buffer);

    if (ret_code != 0) {
        perror("main");
        return EXIT_FAILURE;
    }

    U32Vec_free(&palette_buffer);
    BMP_free(&bmp);
    return EXIT_SUCCESS;
}
