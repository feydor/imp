#include "canvas.h"
#include <stdlib.h>
#include <SDL2/SDL_image.h>

#define W_CANVAS_RESOLUTION 1080
#define H_CANVAS_RESOLUTION 720

ImpCanvas *create_imp_canvas(SDL_Window *window, SDL_Renderer *renderer, SDL_Texture *layer0_text) {
    ImpCanvas *canvas = malloc(sizeof(ImpCanvas));
    if (!canvas) {
        return NULL;
    }

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    canvas->rect.x = w/2 - W_CANVAS_RESOLUTION/2;
    canvas->rect.y = h/2 - H_CANVAS_RESOLUTION/2;
    canvas->rect.w = W_CANVAS_RESOLUTION;
    canvas->rect.h = H_CANVAS_RESOLUTION;
    
    canvas->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING, canvas->rect.w, canvas->rect.h);
    void *text_bytes;
    int text_pitch;
    SDL_LockTexture(canvas->texture, NULL, &text_bytes, &text_pitch);
    memset((u32 *)text_bytes, 0xFFFFFF, canvas->rect.w * 4*canvas->rect.h);
    SDL_UnlockTexture(canvas->texture);

    SDL_Surface *bg = IMG_Load("../res/png/border.png");
    canvas->bg = SDL_CreateTextureFromSurface(renderer, bg);
    canvas->bg_rect = (SDL_Rect){
        canvas->rect.x, canvas->rect.y, bg->w, bg->h
    };

    return canvas;
}

static void print_rect(SDL_Rect *r, char *msg) {
    printf("%s: { x: %d, y: %d, w: %d, h: %d }\n", msg, r->x, r->y, r->w, r->h);
}

static void imp_canvas_pencil_draw(ImpCanvas *canvas, ImpCursor *cursor) {
    // TODO: adjustable pencil size (currently 4x4 pixel)
    int ybuffer = 1, xbuffer = 1;
    SDL_Rect edit_rect = {cursor->rect.x - canvas->rect.x + xbuffer,
                          cursor->rect.y - canvas->rect.y + ybuffer,
                          cursor->w_pencil,
                          cursor->h_pencil};

    void *raw_data;
    int pitch;
    SDL_LockTexture(canvas->texture, &edit_rect, &raw_data, &pitch);

    u32 *pixels = (u32 *)raw_data;
    int npixels = (pitch / 4) * edit_rect.h;
    for (int i = 0; i < npixels; ++i) {
        pixels[i] = cursor->color;
    }
    SDL_UnlockTexture(canvas->texture);
}

static void imp_canvas_draw_line(ImpCanvas *canvas, ImpCursor *cursor, SDL_Point *start, SDL_Point *end) {
    // TODO: Bresenhams algorithm
    int x1 = start->x - canvas->rect.x;
    int y1 = start->y - canvas->rect.y;
    int x2 = end->x - canvas->rect.x;
    int y2 = end->y - canvas->rect.y;

    SDL_Rect edit = { x1, y1, fmax(4, x2-x1), fmax(4, y2-y1) };
    void *rawdata;
    int pitch;
    SDL_LockTexture(canvas->texture, &edit, &rawdata, &pitch);
    u32 *pixels = (u32 *)rawdata;
    int npixels = (pitch / 4) * edit.h;
    for (int i = 0; i < npixels; ++i) {
        int x = i % abs((int)fmax(4, x2-x1));
        int y = i / abs((int)fmax(4, x2-x1));

        if (x == y) {
            pixels[i] = cursor->color;
        }
    }
    SDL_UnlockTexture(canvas->texture);
}

void imp_canvas_event(ImpCanvas *canvas, SDL_Event *e, ImpCursor *cursor) {
    SDL_Point prev_cursor = { cursor->rect.x, cursor->rect.y };

    int xcur, ycur;
    SDL_GetMouseState(&xcur, &ycur);
    SDL_Rect cur_cursor = { xcur, ycur, 1, 1 };
    
    switch (e->type) {
    case SDL_MOUSEBUTTONDOWN: {
        if (SDL_HasIntersection(&canvas->rect, &cur_cursor)) {
            if (cursor->mode == IMP_PENCIL) {
                cursor->pencil_locked = true;
                imp_canvas_pencil_draw(canvas, cursor);
            }
        }
    } break;

    case SDL_MOUSEMOTION: {
        if (cursor->pencil_locked && cursor->mode == IMP_PENCIL &&
                SDL_HasIntersection(&canvas->rect, &cur_cursor)) {
            imp_canvas_draw_line(canvas, cursor, &prev_cursor, &(SDL_Point){ xcur, ycur });
        }
    } break;

    case SDL_MOUSEBUTTONUP: {
        if (cursor->mode == IMP_PENCIL) {
            cursor->pencil_locked = false;
        }
    } break;
    }
}

void imp_canvas_render(SDL_Renderer *renderer, ImpCanvas *c) {
    SDL_RenderCopy(renderer, c->bg, NULL, &c->bg_rect);
    SDL_RenderCopy(
        renderer, c->texture, NULL,
        &(SDL_Rect){c->rect.x + 16, c->rect.y + 16, c->rect.w - 16, c->rect.h - 16});
}
