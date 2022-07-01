#ifndef LAYER_H
#define LAYER_H

#include <SDL2/SDL.h>
#include <stdbool.h>

typedef struct {
    SDL_Texture *texture;
    SDL_Rect rect;
    bool visible;
} ImpLayer;

ImpLayer *create_imp_layer(SDL_Rect r);
void imp_layer_render(SDL_Renderer *renderer, ImpLayer *l);

#endif