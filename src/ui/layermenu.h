#ifndef LAYERMENU_H
#define LAYERMENU_H

#include "layer.h"
typedef struct {
    ImpLayer **layers;
    int n_layers;
    int selected_layer;
    SDL_Rect rect;
} ImpLayerMenu;

ImpLayerMenu *create_imp_layermenu(SDL_Rect menu_rect, SDL_Rect init_layer_rect,
        SDL_Texture *init_layer_texture);
void imp_layermenu_render(SDL_Renderer *renderer, ImpCanvas *canvas, ImpLayerMenu *menu);
#endif