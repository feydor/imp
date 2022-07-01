#include "layer.h"

typedef unsigned char uc;

ImpLayer *create_imp_layer(SDL_Rect rect) {
    ImpLayer *l = malloc(sizeof(ImpLayer));
    if (!l) {
        return NULL;
    }
    l->rect = rect;
    l->texture = NULL;
    l->visible = true;
    return l;
}

// TODO
// void imp_layer_update(ImpLayer *l);

void imp_layer_render(SDL_Renderer *renderer, ImpLayer *l) {
    if (l->visible) {
        uc r, g, b, a;
        SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
        SDL_SetRenderDrawColor(renderer, 0xFF, 0, 0, 255);
        // TODO: eventually switch this to render the layer's texture
        // SDL_RenderCopy(renderer, l->texture, NULL, &l->rect);
        SDL_RenderDrawRect(renderer,
            &(SDL_Rect){ l->rect.x + 50, l->rect.y + 50, l->rect.w - 100, l->rect.h - 100 });
        SDL_SetRenderDrawColor(renderer, r, g, b, a);
    }
}