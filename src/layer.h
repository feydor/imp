#ifndef LAYER_H
#define LAYER_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "imp.h"
typedef struct ImpLayer ImpLayer;

ImpLayer *create_imp_layer(SDL_Rect r, SDL_Texture *t);
void imp_layer_render(SDL_Renderer *r, ImpCanvas *c, ImpLayer *l);

#endif