#include "imp.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#define MAX(a, b) (a > b ? a : b)

#define DEFAULT_CANVAS_H 400
#define DEFAULT_CANVAS_W 600
#define DEFAULT_WINDOW_H 600
#define DEFAULT_WINDOW_W 800
#define DEFAULT_N_LAYERS 1
#define DEFAULT_N_BUTTON_MENUS 2
#define N_BUTTON_VERT 2
#define N_BUTTON_HORIZ 4

Imp *create_imp(SDL_Renderer *renderer, SDL_Window *window) {
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
    imp->cursor.pressed = false;

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    imp->canvas.x = w / 2 - DEFAULT_CANVAS_W / 2;
    imp->canvas.y = h / 2 - DEFAULT_CANVAS_H / 2;
    imp->canvas.w = DEFAULT_CANVAS_W;
    imp->canvas.h = DEFAULT_CANVAS_H;

    imp->n_button_menus = DEFAULT_N_BUTTON_MENUS;
    imp->button_menus = malloc(sizeof(ImpButtonMenu *) * imp->n_button_menus);
    if (!imp->button_menus) {
        return NULL;
    }

    imp->button_menus[0] = create_imp_button_menu(renderer, (SDL_Point){0, 0},
        N_BUTTON_VERT, IMP_VERT, IMP_DOWNWARDS);
    imp->button_menus[1] = create_imp_button_menu(renderer, (SDL_Point){0, h-64},
        N_BUTTON_HORIZ, IMP_HORIZ, IMP_RIGHTWARDS);

    imp->n_layers = DEFAULT_N_LAYERS;
    imp->layers = malloc(sizeof(ImpLayer *) * imp->n_layers);
    if (!imp->layers) {
        return NULL;
    }
    for (int i = 0; i < imp->n_layers; ++i) {
        imp->layers[i] = create_imp_layer(
            (SDL_Rect){ imp->canvas.x, imp->canvas.y, imp->canvas.w, imp->canvas.h});
    }

    return imp;
}


int imp_event(Imp *imp, SDL_Event *e) {
    (void)imp;
    switch (e->type) {
        case SDL_QUIT: return IMP_NORMAL_EXIT;
        case SDL_KEYDOWN: {
            if (e->key.keysym.sym == SDLK_ESCAPE) return IMP_NORMAL_EXIT;
        } break;
        case SDL_MOUSEBUTTONDOWN: {
        switch (e->button.button) {
        case SDL_BUTTON_LEFT: {
            // check these things in order:
            // is mouse over a button, then run its task
            // is mouse over a layer, then run a task based on the current mouse mode
            // is mouse over canvas, then run current mouse mode over canvas
            // otherwise do nothing

            // check all buttons, if clicked
            // fire that buttons task
            // if (imp->buttons.mouse_over(imp->mouse)) {
            //     // TODO
            // } else if (imp->layers.mouse_over(imp->mouse)) {

            // } else if (imp->canvas.mouse_over(imp->mouse)) {
            //    // TODO
            // }
            }
        } break;
        case SDL_BUTTON_RIGHT: {
            // TODO
        } break;
        } break;
    }

    return 0;
}


void imp_update(Imp *imp, float dt) {
    // TODO
    (void) imp;
    (void) dt;
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
    SDL_Rect canvas_rect = { imp->canvas.x, imp->canvas.y, imp->canvas.w, imp->canvas.h };
    SDL_RenderFillRect(imp->renderer, &canvas_rect);

    // render ui
    for (int i = 0; i < imp->n_button_menus; ++i)
        imp_buttonmenu_render(imp, imp->button_menus[i]);
    for (int i = 0; i < imp->n_layers; ++i)
        imp_layer_render(imp->renderer, imp->layers[i]);
}


static void imp_print(ImpRetCode ret) {
    char message[69];
    switch (ret) {
    case IMP_NORMAL_EXIT: sprintf(message, "Exiting imp succesfully: %d\n", ret); break;
    }
    puts(message);
}


int main(void) {
    SDL_Window *window = SDL_CreateWindow("imp", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, DEFAULT_WINDOW_W,
        DEFAULT_WINDOW_H, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    
    if (!renderer) {
        exit(fprintf(stderr, "Could not create SDL Renderer\n"));
    }

    Imp *imp = create_imp(renderer, window);
    if (!imp) {
        exit(fprintf(stderr, "imp was NULL\n"));
    }

    SDL_Event e;

    float dt = 1000.0f / 60.0f;
    while (1) {
        uint64_t t0 = SDL_GetTicks64();

        while (SDL_PollEvent(&e)) {
            int ret_code;
            if ((ret_code = imp_event(imp, &e)) != 0) {
                imp_print(ret_code);
                exit(ret_code);
            }
        }

        imp_update(imp, dt);
        imp_render(imp, window);
        SDL_RenderPresent(renderer);

        uint64_t t1 = SDL_GetTicks64();
        SDL_Delay(MAX(10, dt - (t1 - t0)));
    }
}