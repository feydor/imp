#include "layer.h"
typedef unsigned char uc;

typedef struct ImpLayer {
    SDL_Texture *texture;
    SDL_Rect rect;
    bool visible;
} ImpLayer;

ImpLayer *create_imp_layer(SDL_Rect rect, SDL_Texture *t) {
    ImpLayer *l = malloc(sizeof(ImpLayer));
    if (!l) {
        return NULL;
    }
    l->rect = rect;
    l->texture = t;
    l->visible = true;
    return l;
}

// TODO
// void imp_layer_update(ImpLayer *l);

void imp_layer_render(SDL_Renderer *renderer, ImpCanvas *canvas, ImpLayer *l) {
    if (l->visible) {
        uc r, g, b, a;
        SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0xFF, 255);
        SDL_Rect rect = (SDL_Rect){
            l->rect.x,
            l->rect.y,
            l->rect.w + canvas->dw,
            l->rect.h + canvas->dh };
        // TODO: eventually switch this to render the layer's texture
        // SDL_RenderCopy(renderer, l->texture, NULL, &l->rect);
        SDL_RenderDrawRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, r, g, b, a);
    }
}