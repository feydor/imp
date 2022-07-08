#include "layermenu.h"
#include <SDL2/SDL_image.h>

#define DEFAULT_N_LAYERS 1
#define DEFAULT_BUTTON_SIZE 50

typedef unsigned char uc;

ImpLayerMenu *create_imp_layermenu(SDL_Renderer *renderer, SDL_Rect menu_rect, SDL_Rect init_layer_rect,
        SDL_Texture *init_layer_texture) {
    ImpLayerMenu *menu = malloc(sizeof(ImpLayerMenu));
    if (!menu) {
        return NULL;
    }

    menu->n_layers = DEFAULT_N_LAYERS;
    menu->rect = menu_rect;
    menu->canvas = init_layer_rect;
    menu->layers = malloc(sizeof(ImpLayer *) * menu->n_layers);
    if (!menu->layers) {
        return NULL;
    }
    
    menu->selected_layer = 0;
    for (int i = 0; i < menu->n_layers; ++i) {
        menu->layers[i] = create_imp_layer(init_layer_rect, init_layer_texture);
    }

    SDL_Surface *plus = SDL_LoadBMP("../res/icons/plus-button.bmp");
    menu->plus_texture = SDL_CreateTextureFromSurface(renderer, plus);
    menu->plus_rect = (SDL_Rect){ menu->rect.x,
                                  menu->rect.y,
                                  DEFAULT_BUTTON_SIZE,
                                  DEFAULT_BUTTON_SIZE };
    SDL_Surface *minus = SDL_LoadBMP("../res/icons/minus-button.bmp");
    menu->minus_texture = SDL_CreateTextureFromSurface(renderer, minus);
    menu->minus_rect = (SDL_Rect){ menu->rect.x + DEFAULT_BUTTON_SIZE,
                                   menu->rect.y,
                                   DEFAULT_BUTTON_SIZE,
                                   DEFAULT_BUTTON_SIZE };

    IMG_Init(IMG_INIT_PNG);
    SDL_Surface *surf = IMG_Load("../res/png/red-stickers.png");
    menu->unfinished = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);

    return menu;
}

static void imp_layermenu_add_default_layer(SDL_Renderer *renderer, ImpLayerMenu *menu) {
    menu->layers = realloc(menu->layers, sizeof(ImpLayer *) * ++menu->n_layers);

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING, menu->canvas.w, menu->canvas.w);
    menu->layers[++menu->selected_layer] = create_imp_layer(menu->canvas, texture);
}

static void imp_layermenu_delete_selected_layer(ImpLayerMenu *menu) {
    if (menu->n_layers < 1) {
        return;
    }
    menu->layers = realloc(menu->layers, sizeof(ImpLayer *) * --menu->n_layers);
    --menu->selected_layer;
}

bool imp_cursor_over_selected_layer(ImpLayerMenu *lmenu, ImpCursor *cursor) {
    if (lmenu->n_layers < 1) {
        return false;
    }
    ImpLayer *l = lmenu->layers[lmenu->selected_layer];
    return SDL_HasIntersection(&(SDL_Rect){ cursor->x, cursor->y, 1, 1 },
                               &l->rect);
}

void imp_layermenu_event(ImpLayerMenu *menu, SDL_Event *e, ImpCursor *cursor) {
    switch (e->type) {
    case SDL_MOUSEBUTTONDOWN: {
        if (imp_cursor_over_selected_layer(menu, cursor)) {
            cursor->scroll_locked = true;
            imp_layer_scroll_start(menu->layers[menu->selected_layer], cursor->x, cursor->y);
        }
    } break;

    case SDL_MOUSEMOTION: {
        if (cursor->mode == IMP_CURSOR && cursor->scroll_locked) {
            imp_layer_scroll_update(menu->layers[menu->selected_layer], cursor->x, cursor->y);
        }
    } break;
    }

    for (int i = 0; i < menu->n_layers; ++i) {
        ImpLayer *l = menu->layers[i];
        imp_layer_event(l, e, cursor);
    }
}


// TODO: fix layers rendering after delete (specifically after drawing)
void imp_layermenu_render(SDL_Renderer *renderer, ImpCanvas *canvas, ImpLayerMenu *menu) {
    // first render the layers themselves, then the ui menu
    for (int i = 0; i < menu->n_layers; ++i)
        imp_layer_render(renderer, canvas, menu->layers[i]);
    
    // render the plus/minus buttons
    SDL_RenderCopy(renderer, menu->plus_texture, NULL, &menu->plus_rect);
    SDL_RenderCopy(renderer, menu->minus_texture, NULL, &menu->minus_rect);

    uc r, g, b, a;
    int ystart = menu->rect.y - DEFAULT_BUTTON_SIZE;
    SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
    for (int i = 0; i < menu->n_layers; ++i) {
        // TODO: change to textures
        switch (i % 3) {
        case 0: SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 255); break;
        case 1: SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 255); break;
        case 2: SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 255); break;
        }

        SDL_Rect dest = (SDL_Rect){menu->rect.x,
                                   ystart - i * menu->rect.h,
                                   menu->rect.w,
                                   menu->rect.h};
        SDL_RenderFillRect(renderer, &dest);

        if (i == menu->selected_layer) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
            SDL_RenderFillRect(renderer, &dest);
        }
    }

    SDL_Rect menu_rect = (SDL_Rect){menu->rect.x, ystart, menu->rect.w, 2 * menu->rect.h};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255 / 2);
    SDL_RenderFillRect(renderer, &menu_rect);
    SDL_RenderCopy(renderer, menu->unfinished, NULL, &menu_rect);
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}