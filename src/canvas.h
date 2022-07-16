#ifndef IMP_CANVAS_H
#define IMP_CANVAS_H
#include "cursor.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

typedef uint32_t u32;

typedef struct ImpCircleGuide ImpCircleGuide;

typedef struct  {
    SDL_Rect rect;
    SDL_Surface *surf;
    SDL_Texture *bg;
    SDL_Rect bg_rect;
    SDL_Window *window_ref;
    SDL_PixelFormat *pixel_format;
    SDL_Rect rectangle_guide;
    ImpCircleGuide *circle_guide;
    size_t pitch; // bytes per row
    size_t depth; // RGBA bits
} ImpCanvas;

ImpCanvas *create_imp_canvas(SDL_Window *window, SDL_Renderer *renderer);
void imp_canvas_event(ImpCanvas *c, SDL_Event *e, ImpCursor *cursor);
void imp_canvas_render(SDL_Renderer *renderer, ImpCanvas *c);

void imp_canvas_bounds_checking(ImpCanvas *canvas, int *x, int *y, int xoff, int yoff);
u32 imp_rgba(ImpCanvas *c, u32 color);
#endif