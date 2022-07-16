#include "imp.h"
#include "canvas.h"
#include "cursor.h"
#include "ui/actionmenu.h"
#include "ui/colormenu.h"
#include "ui/toolmenu.h"
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MAX(a, b) (a > b ? a : b)

typedef struct Imp {
    SDL_Renderer *renderer;
    SDL_Window *window;
    ImpCanvas *canvas;
    ImpCursor *cursor;
    ImpToolMenu *toolmenu;
    ImpActionMenu *actionmenu;
    ImpColorMenu *colormenu;

    SDL_Texture *bg;
    int w_bg, h_bg;
} Imp;

Imp *create_imp(SDL_Renderer *renderer, SDL_Window *window) {
    Imp *imp = malloc(sizeof(Imp));
    if (!imp) {
        return NULL;
    }

    imp->renderer = renderer;
    imp->window = window;
    imp->cursor = create_imp_cursor();
    imp->canvas = create_imp_canvas(window, renderer);
    imp->colormenu = create_imp_colormenu(renderer, imp->canvas);

    char *vert_bg = "../res/png/button-menu-vert-jp-lavender.png";
    imp->toolmenu = create_imp_toolmenu(renderer, imp->canvas, vert_bg);

    char *horiz_bg = "../res/png/button-menu-horiz-jp-lavender.png";
    imp->actionmenu = create_imp_actionmenu(renderer, imp->canvas, horiz_bg);
    
    SDL_Surface *bg_surf = IMG_Load("../res/patterns/bg.png");
    imp->bg = SDL_CreateTextureFromSurface(renderer, bg_surf);
    imp->w_bg = bg_surf->w;
    imp->h_bg = bg_surf->h;

    return imp;
}


int imp_event(Imp *imp, SDL_Event *e) {
    if (e->type == SDL_MOUSEBUTTONDOWN || e->type == SDL_MOUSEBUTTONUP || e->type == SDL_MOUSEMOTION) {
        imp->cursor->rect.x = e->button.x;
        imp->cursor->rect.y = e->button.y;
    } else if (e->type == SDL_QUIT || (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_ESCAPE)) {
        return 0;
    }

    imp_canvas_event(imp->canvas, e, imp->cursor);

    ImpTool returned;
    if ((returned = imp_toolmenu_event(imp->toolmenu, e, imp->cursor))) {
        imp_actionmenu_ontoolchange(imp->actionmenu, returned);
    }

    imp_actionmenu_event(imp->actionmenu, e, imp->cursor);
    imp_colormenu_event(imp->colormenu, e, imp->cursor);
    return 1;
}


void imp_update(Imp *imp, float dt) {
    // TODO
    (void) dt;
    (void) imp;
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
    imp_toolmenu_render(imp->renderer, imp->toolmenu);
    imp_actionmenu_render(imp->renderer, imp->actionmenu);
    imp_colormenu_render(imp->renderer, imp->colormenu);
}
