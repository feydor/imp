#include "imp.h"
#include "layer.h"
#include "ui/button_bar.h"
#include "ui/layermenu.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#define MAX(a, b) (a > b ? a : b)

typedef struct Imp {
    SDL_Renderer *renderer;
    SDL_Window *window;
    u32 color;
    int nzoom;
    ImpCanvas canvas;
    ImpCursor cursor;

    ImpButtonMenu **button_menus;
    int n_button_menus;
    ImpLayerMenu *layer_menu;
} Imp;

#define DEFAULT_CANVAS_H 400
#define DEFAULT_CANVAS_W 600
#define DEFAULT_WINDOW_H 600
#define DEFAULT_WINDOW_W 800
#define DEFAULT_N_LAYERS 1
#define DEFAULT_N_BUTTON_MENUS 2
#define N_BUTTON_VERT 2
#define N_BUTTON_HORIZ 4

Imp *create_imp(SDL_Renderer *renderer, SDL_Window *window, SDL_Texture *layer0_texture) {
    Imp *imp = malloc(sizeof(Imp));
    if (!imp) {
        return NULL;
    }

    imp->renderer = renderer;
    imp->window = window;
    imp->color = 0xFF0000;
    imp->nzoom = 0;

    imp->cursor.x = imp->cursor.y = 0;
    imp->cursor.mode = IMP_CURSOR;
    imp->cursor.scroll_locked = false;

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    imp->canvas.x = w / 2 - DEFAULT_CANVAS_W / 2;
    imp->canvas.y = h / 2 - DEFAULT_CANVAS_H / 2;
    imp->canvas.w = DEFAULT_CANVAS_W;
    imp->canvas.h = DEFAULT_CANVAS_H;
    imp->canvas.dw = 0;
    imp->canvas.dh = 0;

    imp->n_button_menus = DEFAULT_N_BUTTON_MENUS;
    imp->button_menus = malloc(sizeof(ImpButtonMenu *) * imp->n_button_menus);
    if (!imp->button_menus) {
        return NULL;
    }

    imp->button_menus[0] = create_imp_button_menu(renderer, (SDL_Point){0, 0},
        N_BUTTON_VERT, IMP_VERT, IMP_DOWNWARDS);
    imp->button_menus[1] = create_imp_button_menu(renderer, (SDL_Point){0, h-64},
        N_BUTTON_HORIZ, IMP_HORIZ, IMP_RIGHTWARDS);

    int w_wind, h_wind;
    SDL_GetWindowSize(window, &w_wind, &h_wind);
    SDL_Rect menu_rect = { w_wind - 100, h_wind - 50, 100, 50};
    SDL_Rect layer0_rect = { imp->canvas.x, imp->canvas.y, imp->canvas.w, imp->canvas.h };
    imp->layer_menu = create_imp_layermenu(menu_rect, layer0_rect, layer0_texture);

    return imp;
}


int imp_event(Imp *imp, SDL_Event *e) {
    (void)imp;
    switch (e->type) {
        case SDL_QUIT: return 0;
        case SDL_KEYDOWN: {
            if (e->key.keysym.sym == SDLK_ESCAPE) return 0;
        } break;

        case SDL_MOUSEMOTION: {
            imp->cursor.x = e->button.x;
            imp->cursor.y = e->button.y;

            if (imp->cursor.mode == IMP_CURSOR && imp->cursor.scroll_locked) {
                imp_layermenu_scroll_update(imp->layer_menu, &imp->cursor);
            }
        } break;

        case SDL_MOUSEWHEEL: {
            if (e->wheel.y > 0) {
                imp->nzoom += 1;
            } else if (e->wheel.y < 0) {
                imp->nzoom -= 1;
            }
        } break;

        case SDL_MOUSEBUTTONDOWN: {
            if (imp_cursor_over_layer(imp->layer_menu, &imp->cursor)) {
                if (imp->cursor.mode == IMP_CURSOR) {
                    imp->cursor.scroll_locked = true;
                    imp_layermenu_scroll_start(imp->layer_menu, &imp->cursor);
                }
            }
        } break;

        case SDL_MOUSEBUTTONUP: {
            if (imp->cursor.mode == IMP_CURSOR) {
                imp->cursor.scroll_locked = false;
            }
        } break;
    }


    imp_buttonmenu_event(imp->button_menus[0], e, &imp->cursor);
    imp_buttonmenu_event(imp->button_menus[1], e, &imp->cursor);
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

    // render canvas
    SDL_SetRenderDrawColor(imp->renderer, 0xFF, 0xFF, 0xFF, 255);
    SDL_Rect canvas_rect = {
        imp->canvas.x,
        imp->canvas.y,
        fmax(imp->canvas.w + imp->canvas.dw, 0),
        fmax(imp->canvas.h + imp->canvas.dh, 0) };
    SDL_RenderFillRect(imp->renderer, &canvas_rect);

    // render layers + menu ui
    imp_layermenu_render(imp->renderer, &imp->canvas, imp->layer_menu);

    // render ui
    for (int i = 0; i < imp->n_button_menus; ++i)
        imp_buttonmenu_render(imp->renderer, imp->button_menus[i]);
}
