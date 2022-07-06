#ifndef LAYERMENU_H
#define LAYERMENU_H

#include "layer.h"
typedef struct {
    ImpLayer **layers;
    int n_layers;
    int selected_layer;
    SDL_Rect rect;
    SDL_Rect canvas;
    SDL_Rect plus_rect;
    SDL_Rect minus_rect;
    SDL_Texture *plus_texture;
    SDL_Texture *minus_texture;
} ImpLayerMenu;

ImpLayerMenu *create_imp_layermenu(SDL_Renderer *renderer, SDL_Rect menu_rect, SDL_Rect init_layer_rect,
        SDL_Texture *init_layer_texture);
bool imp_cursor_over_layer(ImpLayerMenu *lmenu, ImpCursor *cursor);
void imp_layermenu_scroll_start(ImpLayerMenu *lmenu, ImpCursor *cursor);
void imp_layermenu_scroll_update(ImpLayerMenu *lmenu, ImpCursor *cursor);
void imp_layermenu_event(SDL_Renderer *renderer, ImpLayerMenu *menu, SDL_Event *e, ImpCursor *cursor);
void imp_layermenu_render(SDL_Renderer *renderer, ImpCanvas *canvas, ImpLayerMenu *menu);
#endif