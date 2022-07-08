#ifndef LAYER_H
#define LAYER_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "imp.h"
typedef struct ImpLayer {
    SDL_Texture *texture;
    SDL_Rect rect;
    SDL_Point next;
    int dw, dh;
    SDL_Point rel_click_dist;
    bool visible;
} ImpLayer;

ImpLayer *create_imp_layer(SDL_Rect r, SDL_Texture *t);
void imp_layer_scroll_update(ImpLayer *l, int x, int y);
void imp_layer_scroll_start(ImpLayer *l, int x, int y);
void imp_layer_event(ImpLayer *l, SDL_Window *w, SDL_Event *e, ImpCursor *cursor);
void imp_layer_render(SDL_Renderer *r, ImpCanvas *c, ImpLayer *l);

#endif