#include "layermenu.h"

#define DEFAULT_N_LAYERS 1

typedef unsigned char uc;

ImpLayerMenu *create_imp_layermenu(SDL_Rect menu_rect, SDL_Rect init_layer_rect,
        SDL_Texture *init_layer_texture) {
    ImpLayerMenu *menu = malloc(sizeof(ImpLayerMenu));
    if (!menu) {
        return NULL;
    }

    menu->n_layers = DEFAULT_N_LAYERS;
    menu->rect = menu_rect;
    menu->layers = malloc(sizeof(ImpLayer *) * menu->n_layers);
    if (!menu->layers) {
        return NULL;
    }
    
    menu->selected_layer = 0;
    for (int i = 0; i < menu->n_layers; ++i) {
        menu->layers[i] = create_imp_layer(init_layer_rect, init_layer_texture);
    }

    return menu;
}

void imp_layermenu_render(SDL_Renderer *renderer, ImpCanvas *canvas, ImpLayerMenu *menu) {
    // first render the layers themselves, then the ui menu
    for (int i = 0; i < menu->n_layers; ++i)
        imp_layer_render(renderer, canvas, menu->layers[i]);

    uc r, g, b, a;
    SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
    

    for (int i = 0; i < menu->n_layers + 1; ++i) {
        // render stacked rectangles
        switch (i % 3) {
        case 0: SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 255); break;
        case 1: SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 255); break;
        case 2: SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 255); break;
        }
        SDL_RenderFillRect(renderer, &(SDL_Rect){menu->rect.x,
                                                 menu->rect.y - i * menu->rect.h,
                                                 menu->rect.w,
                                                 menu->rect.h});
    }
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}