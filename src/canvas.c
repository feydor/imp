#include "canvas.h"
#include <stdlib.h>
#include <SDL2/SDL_image.h>

static int fill_rounded_box_b(SDL_Surface *dst, int xo, int yo, int w, int h, int r, Uint32 color) {
    int yd = dst->pitch / dst->format->BytesPerPixel;
    Uint32 *pixels = NULL;

    int x, y, i, j;
    int rpsqrt2 = (int)(r / sqrt(2));

    w /= 2;
    h /= 2;

    xo += w;
    yo += h;

    w -= r;
    h -= r;

    if (w < 0 || h < 0)
        return 0;

    SDL_LockSurface(dst);
    pixels = (Uint32 *)(dst->pixels);

    int sy = (yo - h) * yd;
    int ey = (yo + h) * yd;
    int sx = (xo - w);
    int ex = (xo + w);
    for (i = sy; i <= ey; i += yd)
        for (j = sx - r; j <= ex + r; j++)
            pixels[i + j] = color;

    int d = -r;
    int x2m1 = -1;
    y = r;
    for (x = 0; x <= rpsqrt2; x++) {
        x2m1 += 2;
        d += x2m1;
        if (d >= 0) {
            y--;
            d -= (y * 2);
        }

        for (i = sx - x; i <= ex + x; i++)
            pixels[sy - y * yd + i] = color;

        for (i = sx - y; i <= ex + y; i++)
            pixels[sy - x * yd + i] = color;

        for (i = sx - y; i <= ex + y; i++)
            pixels[ey + x * yd + i] = color;

        for (i = sx - x; i <= ex + x; i++)
            pixels[ey + y * yd + i] = color;
    }
    SDL_UnlockSurface(dst);
    return 1;
}               

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
    canvas->dw = 0;
    canvas->dh = 0;
    u32 format = SDL_GetWindowPixelFormat(window);
    SDL_PixelFormat *pixel_format = SDL_AllocFormat(format);
    SDL_Surface *surf =
        SDL_CreateRGBSurfaceWithFormat(0, canvas->w, canvas->h, 32, pixel_format->format);
    fill_rounded_box_b(surf, 0, 0, canvas->w, canvas->h, 20, 0xFFFFFFFF);
    
    canvas->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING, canvas->w, canvas->h);
    void *text_bytes;
    int text_pitch;
    SDL_LockTexture(canvas->texture, NULL, &text_bytes, &text_pitch);
    memset((u32 *)text_bytes, 0xFFFFFF, canvas->w * canvas->h * 4);
    SDL_UnlockTexture(canvas->texture);

    // canvas->texture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_Surface *border = IMG_Load("../res/png/border.png");
    canvas->border = SDL_CreateTextureFromSurface(renderer, border);

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
    SDL_Rect canvas_rect = {c->x, c->y, fmax(c->w + c->dw, 0), fmax(c->h + c->dh, 0)};
    SDL_RenderCopy(renderer, c->texture, NULL, &canvas_rect);
    SDL_RenderCopy(
        renderer, c->border, NULL,
        &(SDL_Rect){canvas_rect.x - 8, canvas_rect.y - 9, canvas_rect.w + 18, canvas_rect.h + 20});
}
