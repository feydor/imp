#include "image.h"
#include "imp.h"
#include "system/bmp.h"
#include "system/palette.h"
#include "vector.h"
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <time.h>
#define DEFAULT_WINDOW_H 1000
#define DEFAULT_WINDOW_W 1300
#define PROGNAME "imp"
#define MAX(a, b) (a > b ? a : b)

static void usage() { fprintf(stderr, "%s [input]\n", PROGNAME); }

// Assumes Little-endian, 24bit bmp
// bmp data is stored starting at bottom-left corner
static SDL_Texture *make_texture_from_bmp(SDL_Renderer *renderer, BMP_file *bmp) {
    int depth = 24;
    int pitch = 3 * bmp->w;
    uint32_t rmask, gmask, bmask;
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0x0000FF;
        gmask = 0x00FF00;
        bmask = 0xFF0000;
    #else
        rmask = 0xFF0000;
        gmask = 0x00FF00;
        bmask = 0x0000FF;
    #endif

    SDL_Surface *surf =
        SDL_CreateRGBSurfaceFrom(bmp->image_raw, bmp->w, bmp->h, depth, pitch,
                                 rmask, gmask, bmask, 0);

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return texture;
}


static int sdl_ui() {
    SDL_Window *window = SDL_CreateWindow("imp", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, DEFAULT_WINDOW_W,
        DEFAULT_WINDOW_H, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    if (!renderer) {
        return fprintf(stderr, "Could not create SDL Renderer\n");
    }

    IMG_Init(IMG_INIT_PNG);
    SDL_Surface *surf = IMG_Load("res/icons/window.png");
    SDL_SetWindowIcon(window, surf);
    SDL_FreeSurface(surf);
    // TODO: load the user's file
    // SDL_Surface *surf = IMG_Load(src);
    // SDL_Surface *formatted = SDL_ConvertSurfaceFormat(surf, SDL_GetWindowPixelFormat(window), 0);

    Imp *imp = create_imp(renderer, window);
    if (!imp) {
        return fprintf(stderr, "imp was NULL\n");
    }

    SDL_Event e;
    float dt = 1000.0f / 60.0f;
    while (1) {
        uint64_t t0 = SDL_GetTicks64();

        while (SDL_PollEvent(&e)) {
            if (imp_event(imp, &e) == 0) {
                return 0;
            }
        }

        imp_update(imp, dt);
        imp_render(imp, window);
        SDL_RenderPresent(renderer);

        uint64_t t1 = SDL_GetTicks64();
        SDL_Delay(MAX(10, dt - (t1 - t0)));
    }
    return 0;
}


int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;
    srand(time(NULL));

    int ret_code = sdl_ui();
    if (ret_code != 0) {
        perror("main");
        return EXIT_FAILURE;
    }

    return ret_code;
}
