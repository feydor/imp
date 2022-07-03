#include "layer.h"
typedef unsigned char uc;

ImpLayer *create_imp_layer(SDL_Rect rect, SDL_Texture *t) {
    ImpLayer *l = malloc(sizeof(ImpLayer));
    if (!l) {
        return NULL;
    }
    l->rect = rect;
    l->rel_click_dist = (SDL_Point){ 0, 0 };
    l->next = (SDL_Point){ rect.x, rect.y };
    l->dw = l->dh = 0;
    l->texture = t;
    l->visible = true;
    return l;
}

void imp_layer_cursor_scroll_update(ImpLayer *l, int x, int y) {
    l->next.x = x - l->rel_click_dist.x;
    l->next.y = y - l->rel_click_dist.y;
}

void imp_layer_cursor_scroll_start(ImpLayer *l, int x, int y) {
    l->rel_click_dist.x = x - l->next.x;
    l->rel_click_dist.y = y - l->next.y;
}

void imp_layer_render(SDL_Renderer *renderer, ImpCanvas *canvas, ImpLayer *l) {
    if (l->visible) {
        uc r, g, b, a;
        SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0xFF, 255);
        SDL_Rect rect = {
            l->next.x,
            l->next.y,
            l->rect.w + canvas->dw,
            l->rect.h + canvas->dh
        };
        SDL_RenderCopy(renderer, l->texture, NULL, &rect);
        // SDL_RenderDrawRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, r, g, b, a);
    }
}