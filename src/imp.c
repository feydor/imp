#include "imp.h"
#include "layer.h"
#include "ui/button_bar.h"
#include "ui/layermenu.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL_image.h>
#define MAX(a, b) (a > b ? a : b)

typedef struct Imp {
    SDL_Renderer *renderer;
    SDL_Window *window;
    int nzoom;
    ImpCanvas canvas;
    ImpCursor cursor;

    ImpButtonMenu **button_menus;
    int n_button_menus;
    ImpLayerMenu *layer_menu;
} Imp;

#define DEFAULT_N_LAYERS 1
#define DEFAULT_N_BUTTON_MENUS 2
#define N_BUTTON_VERT 2
#define N_BUTTON_HORIZ 4
#define DEFAULT_PENCIL_COLOR 0xFF0000

Imp *create_imp(SDL_Renderer *renderer, SDL_Window *window, SDL_Texture *layer0_texture) {
    Imp *imp = malloc(sizeof(Imp));
    if (!imp) {
        return NULL;
    }

    imp->renderer = renderer;
    imp->window = window;
    imp->nzoom = 0;

    imp->cursor.x = imp->cursor.y = 0;
    imp->cursor.mode = IMP_CURSOR;
    imp->cursor.scroll_locked = false;
    imp->cursor.pencil_locked = false;
    imp->cursor.color = DEFAULT_PENCIL_COLOR;

    int w, h;
    int margin = 20, left_padding = 64, bottom_padding = 64;
    SDL_GetWindowSize(window, &w, &h);
    imp->canvas.x = margin + left_padding;
    imp->canvas.y = margin;
    imp->canvas.w = w - 2*margin - left_padding;
    imp->canvas.h = h - 2*margin - bottom_padding;
    imp->canvas.dw = 0;
    imp->canvas.dh = 0;

    SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormat(0, imp->canvas.w, imp->canvas.h, 32, SDL_PIXELFORMAT_RGBA32);
    fill_rounded_box_b(surf, 0, 0, imp->canvas.w, imp->canvas.h, 20, 0xFFFFFFFF);
    imp->canvas.texture = SDL_CreateTextureFromSurface(renderer, surf);

    SDL_Surface *border = IMG_Load("../res/png/border.png");
    imp->canvas.border = SDL_CreateTextureFromSurface(renderer, border);

    imp->n_button_menus = DEFAULT_N_BUTTON_MENUS;
    imp->button_menus = malloc(sizeof(ImpButtonMenu *) * imp->n_button_menus);
    if (!imp->button_menus) {
        return NULL;
    }

    imp->button_menus[0] = create_imp_button_menu(renderer, (SDL_Point){0, 0},
        N_BUTTON_VERT, IMP_VERT, IMP_DOWNWARDS);
    imp_buttonmenu_select(imp->button_menus[0], 0);
    imp_buttonmenu_settask(imp->button_menus[0], 0, IMP_SELECT_CURSOR);
    imp_buttonmenu_settask(imp->button_menus[0], 1, IMP_SELECT_PENCIL);

    imp->button_menus[1] = create_imp_button_menu(renderer, (SDL_Point){0, h-64},
        N_BUTTON_HORIZ, IMP_HORIZ, IMP_RIGHTWARDS);

    int w_wind, h_wind;
    SDL_GetWindowSize(window, &w_wind, &h_wind);
    SDL_Rect menu_rect = { w_wind - 100, h_wind - 50, 100, 50};
    SDL_Rect layer0_rect = { imp->canvas.x, imp->canvas.y, imp->canvas.w, imp->canvas.h };
    imp->layer_menu = create_imp_layermenu(renderer, menu_rect, layer0_rect, layer0_texture);

    return imp;
}


int imp_event(Imp *imp, SDL_Event *e) {
    switch (e->type) {
        case SDL_QUIT: return 0;
        case SDL_KEYDOWN: {
            if (e->key.keysym.sym == SDLK_ESCAPE) return 0;
        } break;

        case SDL_MOUSEMOTION: {
            imp->cursor.x = e->button.x;
            imp->cursor.y = e->button.y;
        } break;

        case SDL_MOUSEBUTTONUP: {
            if (imp->cursor.mode == IMP_CURSOR) {
                imp->cursor.scroll_locked = false;
            }
        } break;
    }


    imp_buttonmenu_event(imp->button_menus[0], e, &imp->cursor);
    imp_buttonmenu_event(imp->button_menus[1], e, &imp->cursor);
    imp_layermenu_event(imp->layer_menu, imp->window, e, &imp->cursor);
    return 1;
}


void imp_update(Imp *imp, float dt) {
    // TODO
    (void) dt;
    
    // update canvas dw and dh
    const float zoom_rate = 0.1f;
    imp->canvas.dh = imp->nzoom * imp->canvas.h * zoom_rate;
    imp->canvas.dw = imp->nzoom * imp->canvas.w * zoom_rate;
}


void imp_render(Imp *imp, SDL_Window *window) {
    SDL_RenderClear(imp->renderer);

    // render bg/window
    SDL_SetRenderDrawColor(imp->renderer, 0x33, 0x33, 0x33, 255);
    SDL_Rect win_rect = {0};
    SDL_GetWindowSize(window, &win_rect.w, &win_rect.h);
    SDL_RenderFillRect(imp->renderer, &win_rect);

    // render canvas + border
    SDL_Rect canvas_rect = {
        imp->canvas.x,
        imp->canvas.y,
        fmax(imp->canvas.w + imp->canvas.dw, 0),
        fmax(imp->canvas.h + imp->canvas.dh, 0) };
    SDL_RenderCopy(imp->renderer, imp->canvas.texture, NULL, &canvas_rect);
    SDL_RenderCopy(
        imp->renderer, imp->canvas.border, NULL,
        &(SDL_Rect){canvas_rect.x - 6, canvas_rect.y - 6, canvas_rect.w + 12, canvas_rect.h + 12});

    // render layers + menu ui
    imp_layermenu_render(imp->renderer, &imp->canvas, imp->layer_menu);

    // render ui
    for (int i = 0; i < imp->n_button_menus; ++i)
        imp_buttonmenu_render(imp->renderer, imp->button_menus[i]);
}

int fill_rounded_box_b(SDL_Surface *dst, int xo, int yo, int w, int h, int r, Uint32 color) {
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
