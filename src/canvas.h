#ifndef IMP_CANVAS_H
#define IMP_CANVAS_H
#include "cursor.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

typedef uint32_t u32;   
typedef enum ImpTool ImpTool;
typedef struct ImpCircleGuide ImpCircleGuide;

typedef struct ImpColorMasks {
    u32 r, g, b;
} ImpColorMasks;

typedef struct ImpCircleGuide {
    int x, y, r;
} ImpCircleGuide;

typedef struct ImpLineGuide {
    int x1, x2, y1, y2;
} ImpLineGuide;

typedef struct  {
    SDL_Rect rect;
    SDL_Surface *surf;
    SDL_Texture *bg;
    SDL_Rect bg_rect;
    SDL_Window *window_ref;
    SDL_PixelFormat *pixel_format;
    SDL_Rect rectangle_guide;
    ImpCircleGuide circle_guide;
    ImpLineGuide line_guide;
    ImpColorMasks masks;
    size_t pitch; // bytes per row
    size_t depth; // RGBA bits
    size_t size_line;
    char *output;
    bool save_lock;
} ImpCanvas;

ImpCanvas *create_imp_canvas(SDL_Window *window, SDL_Renderer *renderer, char *output);
void imp_canvas_event(ImpCanvas *c, SDL_Event *e, ImpCursor *cursor, ImpTool currtool);
void imp_canvas_render(SDL_Renderer *renderer, ImpCanvas *c);

void imp_canvas_bounds_checking(ImpCanvas *canvas, int *x, int *y, int xoff, int yoff);
u32 imp_rgba(ImpCanvas *c, u32 color);
#endif