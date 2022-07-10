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
    int margin = 20, left_padding = 64, bottom_padding = 64;
    SDL_GetWindowSize(window, &w, &h);
    canvas->x = margin + left_padding;
    canvas->y = margin;
    canvas->w = w - 2 * margin - left_padding;
    canvas->h = h - 2 * margin - bottom_padding;
    canvas->dw = 0;
    canvas->dh = 0;
    u32 format = SDL_GetWindowPixelFormat(window);
    SDL_PixelFormat *pixel_format = SDL_AllocFormat(format);
    SDL_Surface *surf =
        SDL_CreateRGBSurfaceWithFormat(0, canvas->w, canvas->h, 32, pixel_format->format);
    fill_rounded_box_b(surf, 0, 0, canvas->w, canvas->h, 20, 0xFFFFFFFF);
    
    canvas->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING, canvas->w, canvas->h);

    // canvas->texture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_Surface *border = IMG_Load("../res/png/border.png");
    canvas->border = SDL_CreateTextureFromSurface(renderer, border);

    return canvas;
}

void print_rect(SDL_Rect *r, char *msg) {
    printf("%s: { x: %d, y: %d, w: %d, h: %d }\n", msg, r->x, r->y, r->w, r->h);
}

void imp_canvas_event(ImpCanvas *c, SDL_Window *window, SDL_Event *e, ImpCursor *cursor) {
    SDL_Rect cursor_rect = { cursor->x, cursor->y, 1, 1 };
    SDL_Rect canvas_rect = { c->x, c->y, c->w, c->h };
    switch (e->type) {
    case SDL_MOUSEBUTTONDOWN: {
        if (SDL_HasIntersection(&canvas_rect, &cursor_rect)) {
            if (cursor->mode == IMP_PENCIL) {
                cursor->pencil_locked = true;
            }
        }
    } break;

    case SDL_MOUSEMOTION: {
        if (cursor->pencil_locked && cursor->mode == IMP_PENCIL &&
                SDL_HasIntersection(&canvas_rect, &cursor_rect)) {
            float aspect_ratio = (float)canvas_rect.w / canvas_rect.h;
            int ybuffer = 1, xbuffer = 1;
            int yrel = cursor->y - canvas_rect.y + ybuffer;
            // TODO: adjustable pencil size (currently 4x4 pixel)
            SDL_Rect edit_rect = { cursor->x - canvas_rect.x + xbuffer, yrel, 4, 4*aspect_ratio };
            
            void *raw_data;
            int pitch;
            SDL_LockTexture(c->texture, &edit_rect, &raw_data, &pitch);

            u32 format = SDL_GetWindowPixelFormat(window);
            SDL_PixelFormat *mapping_format = SDL_AllocFormat(format);
            u32 color = SDL_MapRGB(mapping_format, 0xFF, 0, 0);

            u32 *pixels = (u32 *)raw_data;
            int npixels = (pitch / 4) * edit_rect.h;
            for (int i = 0; i < npixels; ++i) {
                pixels[i] = color;
            }
            SDL_UnlockTexture(c->texture);
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

void imp_canvas_render(SDL_Renderer *renderer, ImpCanvas *c) {
    SDL_Rect canvas_rect = {c->x, c->y, fmax(c->w + c->dw, 0), fmax(c->h + c->dh, 0)};
    SDL_RenderCopy(renderer, c->texture, NULL, &canvas_rect);
    SDL_RenderCopy(
        renderer, c->border, NULL,
        &(SDL_Rect){canvas_rect.x - 6, canvas_rect.y - 6, canvas_rect.w + 12, canvas_rect.h + 12});
}
