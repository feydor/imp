#include "imp.h"
#include "layer.h"
#include "ui/buttonpanel.h"
#include "ui/layermenu.h"
#include "canvas.h"
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
    ImpCanvas *canvas;
    ImpCursor cursor;

    ImpButtonMenu **button_menus;
    int n_button_menus;
    ImpLayerMenu *layer_menu;
    SDL_Texture *bg;
    int w_bg, h_bg;
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

    imp->canvas = create_imp_canvas(window, renderer, layer0_texture);

    imp->n_button_menus = DEFAULT_N_BUTTON_MENUS;
    imp->button_menus = malloc(sizeof(ImpButtonMenu *) * imp->n_button_menus);
    if (!imp->button_menus) {
        return NULL;
    }

    int gap_between_menu = 24;
    char *vert_bg = "../res/png/button-menu-vert-jp-lavender.png";
    imp->button_menus[0] = create_imp_buttonmenu(renderer, (SDL_Point){imp->canvas->x - 48 - gap_between_menu - 20, imp->canvas->y},
        12, 48, 48, vert_bg, IMP_VERT, IMP_DOWNWARDS);
    imp_buttonmenu_select(imp->button_menus[0], 0);
    imp_buttonmenu_settask(imp->button_menus[0], 0, IMP_SELECT_CURSOR);
    imp_buttonmenu_settask(imp->button_menus[0], 1, IMP_SELECT_PENCIL);

    char *horiz_bg = "../res/png/button-menu-horiz-jp-lavender.png";
    imp->button_menus[1] = create_imp_buttonmenu(renderer, (SDL_Point){imp->canvas->x, imp->canvas->y + imp->canvas->h + gap_between_menu},
        14, 64, 64, horiz_bg, IMP_HORIZ, IMP_RIGHTWARDS);
    
    SDL_Surface *bg_surf = IMG_Load("../res/patterns/bg.png");
    imp->bg = SDL_CreateTextureFromSurface(renderer, bg_surf);
    imp->w_bg = bg_surf->w;
    imp->h_bg = bg_surf->h;

    return imp;
}


int imp_event(Imp *imp, SDL_Event *e) {
    if (e->type == SDL_MOUSEBUTTONDOWN || e->type == SDL_MOUSEBUTTONUP || e->type == SDL_MOUSEMOTION) {
        imp->cursor.x = e->button.x;
        imp->cursor.y = e->button.y;
    } else if (e->type == SDL_QUIT || (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_ESCAPE)) {
        return 0;
    }

    imp_canvas_event(imp->canvas, e, &imp->cursor);
    imp_buttonmenu_event(imp->button_menus[0], e, &imp->cursor);
    imp_buttonmenu_event(imp->button_menus[1], e, &imp->cursor);
    return 1;
}


void imp_update(Imp *imp, float dt) {
    // TODO
    (void) dt;
    
    const float zoom_rate = 0.1f;
    imp->canvas->dh = imp->nzoom * imp->canvas->h * zoom_rate;
    imp->canvas->dw = imp->nzoom * imp->canvas->w * zoom_rate;
}


void imp_render(Imp *imp, SDL_Window *window) {
    SDL_RenderClear(imp->renderer);

    // render bg/window
    SDL_Rect wind = {0};
    SDL_GetWindowSize(window, &wind.w, &wind.h);
    for (int i = 0; i < wind.w; i += imp->w_bg) {
        for (int j = -20; j < wind.h; j += imp->h_bg) {
            SDL_RenderCopy(imp->renderer, imp->bg, NULL, &(SDL_Rect){ i, j, imp->w_bg, imp->h_bg });
        }
    }

    // render canvas
    imp_canvas_render(imp->renderer, imp->canvas);

    // render ui
    for (int i = 0; i < imp->n_button_menus; ++i)
        imp_buttonmenu_render(imp->renderer, imp->button_menus[i]);
}
