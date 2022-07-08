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

void imp_layer_scroll_update(ImpLayer *l, int x, int y) {
    l->next.x = x - l->rel_click_dist.x;
    l->next.y = y - l->rel_click_dist.y;
}

void imp_layer_scroll_start(ImpLayer *l, int x, int y) {
    l->rel_click_dist.x = x - l->next.x;
    l->rel_click_dist.y = y - l->next.y;
}

void imp_layer_event(ImpLayer *l, SDL_Window *w, SDL_Event *e, ImpCursor *cursor) {
    SDL_Rect cursor_rect = { cursor->x, cursor->y, 1, 1 };
    switch (e->type) {
    case SDL_MOUSEBUTTONDOWN: {
        if (SDL_HasIntersection(&l->rect, &cursor_rect)) {
            if (cursor->mode == IMP_PENCIL) {
                cursor->pencil_locked = true;
            }
        }
    } break;

    case SDL_MOUSEMOTION: {
        if (cursor->pencil_locked && cursor->mode == IMP_PENCIL &&
                SDL_HasIntersection(&l->rect, &cursor_rect)) {
            float aspect_ratio = (float)l->rect.w / l->rect.h;
            int ybuffer = 0, xbuffer = 0;
            int yrel = (cursor->y - l->rect.y + ybuffer) * aspect_ratio;
            // TODO: adjustable pencil size (currently 4x4 pixel)
            SDL_Rect edit_rect = { cursor->x - l->rect.x + xbuffer, yrel, 4, 4*aspect_ratio };
            printf("(w:%d,h:%d)\n", l->rect.w, l->rect.h);
            
            void *raw_data;
            int pitch;
            SDL_LockTexture(l->texture, &edit_rect, &raw_data, &pitch);

            u32 format = SDL_GetWindowPixelFormat(w);
            SDL_PixelFormat *mapping_format = SDL_AllocFormat(format);
            u32 color = SDL_MapRGB(mapping_format, 0xFF, 0, 0);

            u32 *pixels = (u32 *)raw_data;
            int npixels = (pitch / 4) * edit_rect.h;
            for (int i = 0; i < npixels; ++i) {
                pixels[i] = color;
            }
            SDL_UnlockTexture(l->texture);
            raw_data = NULL;
        }
    } break;

    case SDL_MOUSEBUTTONUP: {
        if (cursor->mode == IMP_PENCIL) {
            cursor->pencil_locked = false;
        }
    } break;
    }
}


void imp_layer_render(SDL_Renderer *renderer, ImpCanvas *canvas, ImpLayer *l) {
    if (l->visible) {
        SDL_Rect rect = {
            l->next.x,
            l->next.y,
            l->rect.w + canvas->dw,
            l->rect.h + canvas->dh
        };
        SDL_RenderCopy(renderer, l->texture, NULL, &rect);
    }
}