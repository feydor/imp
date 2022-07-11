#include "canvas.h"
#include <stdlib.h>
#include <SDL2/SDL_image.h>

ImpCanvas *create_imp_canvas(SDL_Window *window, SDL_Renderer *renderer, SDL_Texture *layer0_text) {
    ImpCanvas *canvas = malloc(sizeof(ImpCanvas));
    if (!canvas) {
        return NULL;
    }

    int w, h;
    const int W_RESOLUTION = 1080, H_RESOLUTION = 720;
    SDL_GetWindowSize(window, &w, &h);
    canvas->x = w/2 - W_RESOLUTION/2;
    canvas->y = h/2 - H_RESOLUTION/2;
    canvas->w = W_RESOLUTION;
    canvas->h = H_RESOLUTION;
    
    canvas->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING, canvas->w, canvas->h);
    void *text_bytes;
    int text_pitch;
    SDL_LockTexture(canvas->texture, NULL, &text_bytes, &text_pitch);
    memset((u32 *)text_bytes, 0xFFFFFF, canvas->w * canvas->h * 4);
    SDL_UnlockTexture(canvas->texture);

    SDL_Surface *bg = IMG_Load("../res/png/border.png");
    canvas->bg = SDL_CreateTextureFromSurface(renderer, bg);
    canvas->bg_rect = (SDL_Rect){
        canvas->x, canvas->y, bg->w, bg->h
    };

    return canvas;
}

static void print_rect(SDL_Rect *r, char *msg) {
    printf("%s: { x: %d, y: %d, w: %d, h: %d }\n", msg, r->x, r->y, r->w, r->h);
}

static void imp_canvas_pencil_draw(ImpCanvas *c, ImpCursor *cursor, SDL_Rect *canvas_rect) {
    // TODO: adjustable pencil size (currently 4x4 pixel)
    int ybuffer = 1, xbuffer = 1;
    SDL_Rect edit_rect = {cursor->x - canvas_rect->x + xbuffer, cursor->y - canvas_rect->y + ybuffer, 4, 4};

    void *raw_data;
    int pitch;
    SDL_LockTexture(c->texture, &edit_rect, &raw_data, &pitch);

    u32 *pixels = (u32 *)raw_data;
    int npixels = (pitch / 4) * edit_rect.h;
    for (int i = 0; i < npixels; ++i) {
        pixels[i] = cursor->color;
    }
    SDL_UnlockTexture(c->texture);
}

static void imp_canvas_draw_line(ImpCanvas *c, ImpCursor *cursor, SDL_Point *start,
        SDL_Point *end, SDL_Rect *canvas) {
    // TODO: Bresenhams algorithm
    int x1 = start->x - canvas->x;
    int y1 = start->y - canvas->y;
    int x2 = end->x - canvas->x;
    int y2 = end->y - canvas->y;

    SDL_Rect edit = { x1, y1, fmax(4, x2-x1), fmax(4, y2-y1) };
    void *rawdata;
    int pitch;
    SDL_LockTexture(c->texture, &edit, &rawdata, &pitch);
    u32 *pixels = (u32 *)rawdata;
    int npixels = (pitch / 4) * edit.h;
    for (int i = 0; i < npixels; ++i) {
        int x = i % abs((int)fmax(4, x2-x1));
        int y = i / abs((int)fmax(4, x2-x1));

        if (x == y) {
            pixels[i] = cursor->color;
        }
    }
    SDL_UnlockTexture(c->texture);
}

void imp_canvas_event(ImpCanvas *c, SDL_Event *e, ImpCursor *cursor) {
    SDL_Point prev_cursor = { cursor->x, cursor->y };
    SDL_Rect canvas_rect = { c->x, c->y, c->w, c->h };

    int xcur, ycur;
    SDL_GetMouseState(&xcur, &ycur);
    SDL_Rect cur_cursor = { xcur, ycur, 1, 1 };
    
    switch (e->type) {
    case SDL_MOUSEBUTTONDOWN: {
        if (SDL_HasIntersection(&canvas_rect, &cur_cursor)) {
            if (cursor->mode == IMP_PENCIL) {
                cursor->pencil_locked = true;
                imp_canvas_pencil_draw(c, cursor, &canvas_rect);
            }
        }
    } break;

    case SDL_MOUSEMOTION: {
        if (cursor->pencil_locked && cursor->mode == IMP_PENCIL &&
                SDL_HasIntersection(&canvas_rect, &cur_cursor)) {
            imp_canvas_draw_line(c, cursor, &prev_cursor, &(SDL_Point){ xcur, ycur }, &canvas_rect);
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
        &(SDL_Rect){c->x + 16, c->y + 16, c->w - 16, c->h - 16});
}
