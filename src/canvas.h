#ifndef IMP_CANVAS_H
#define IMP_CANVAS_H
#include "cursor.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

typedef uint32_t u32;

typedef struct  {
    SDL_Rect rect;
    SDL_Texture *texture;
    SDL_Texture *bg;
    SDL_Rect bg_rect;
    SDL_Window *window_ref;
} ImpCanvas;

ImpCanvas *create_imp_canvas(SDL_Window *window, SDL_Renderer *renderer, SDL_Texture *layer0_text);
void imp_canvas_event(ImpCanvas *c, SDL_Event *e, ImpCursor *cursor);
void imp_canvas_render(SDL_Renderer *renderer, ImpCanvas *c);
#endif