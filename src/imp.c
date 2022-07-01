#include "imp.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#define MAX(a, b) (a > b ? a : b)

#define DEFAULT_CANVAS_H 200
#define DEFAULT_CANVAS_W 300
#define DEFAULT_WINDOW_H 600
#define DEFAULT_WINDOW_W 800

#define DEFAULT_BUTTON_W 64
#define DEFAULT_BUTTON_H 64
typedef struct {
    SDL_Texture *texture;
    SDL_Rect rect;
} ImpButton;

typedef struct {
    ImpButton **buttons;
    int n;
    SDL_Rect rect;
} ImpButtonMenu;

ImpButtonMenu *create_imp_button_menu(SDL_Renderer *renderer, SDL_Rect area) {
    assert(area.w >= DEFAULT_BUTTON_W && area.h >= DEFAULT_BUTTON_H);
    ImpButtonMenu *menu = malloc(sizeof(ImpButtonMenu));
    if (!menu) {
        return NULL;
    }

    menu->n = area.w / DEFAULT_BUTTON_W;
    menu->rect = (SDL_Rect){ area.x, area.y, menu->n * DEFAULT_BUTTON_W, area.h };
    menu->buttons = malloc(sizeof(ImpButton *) * menu->n);
    if (!menu->buttons) {
        return NULL;
    }

    int xoff = 0;
    for (int i = 0; i < menu->n; ++i) {
        char filename[255];
        sprintf(filename, "../res/icons/horiz-button%d.bmp", i);
        SDL_Surface *surf = SDL_LoadBMP(filename);
        SDL_Texture *text = SDL_CreateTextureFromSurface(renderer, surf);
        ImpButton *button = malloc(sizeof(ImpButton));
        if (!button) {
            return NULL;
        }

        button->texture = text;
        button->rect = (SDL_Rect){ xoff, area.y, DEFAULT_BUTTON_W, area.h };
        xoff += DEFAULT_BUTTON_W;
        menu->buttons[i] = button; // TODO: does this work?

        // SDL_DestroySurface(surf);
    }
    
    return menu;
}

void imp_buttonmenu_render(SDL_Renderer *renderer, ImpButtonMenu *menu) {
    for (int i = 0; i < menu->n; ++i) {
        ImpButton *button = menu->buttons[i];
        SDL_RenderCopy(renderer, button->texture, NULL, &button->rect);
    }
}


typedef enum {
    IMP_CURSOR,
    IMP_PENCIL,
} ImpCursorMode;

typedef enum {
    IMP_NORMAL_EXIT = 1,
} ImpRetCode;

typedef struct Imp {
    SDL_Renderer *renderer;
    u32 color;
    int nzoom;
    
    struct Cursor {
        int x, y;
        ImpCursorMode mode;
        bool pressed;
    } cursor;

    struct Canvas {
        int x, y, w, h;
        SDL_Texture *texture;
    } canvas;

    ImpButtonMenu *button_menu;
} Imp;


Imp *create_imp(SDL_Renderer *renderer) {
    Imp *imp = malloc(sizeof(Imp));
    if (imp == NULL) {
        return NULL;
    }

    imp->renderer = renderer;
    imp->color = 0xFF0000;
    imp->nzoom = 0;

    imp->cursor.x = imp->cursor.y = 0;
    imp->cursor.mode = IMP_CURSOR;
    imp->cursor.pressed = false;

    imp->canvas.x = 0;
    imp->canvas.y = 0;
    imp->canvas.w = DEFAULT_CANVAS_W;
    imp->canvas.h = DEFAULT_CANVAS_H;

    SDL_Rect menu_area = { 0, DEFAULT_WINDOW_H - 64, 128, 64 };
    imp->button_menu = create_imp_button_menu(renderer, menu_area);
    return imp;
}


int imp_event(Imp *imp, SDL_Event *e) {
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
        }
    }

    return 0;
}


void imp_render(Imp *imp, SDL_Window *window) {
    SDL_RenderClear(imp->renderer);

    // render bg
    SDL_SetRenderDrawColor(imp->renderer, 0x33, 0x33, 0x33, 255);
    SDL_Rect win_rect = {0};
    SDL_GetWindowSize(window, &win_rect.w, &win_rect.h);
    SDL_RenderFillRect(imp->renderer, &win_rect);

    // render canvas
    SDL_SetRenderDrawColor(imp->renderer, 0xFF, 0xFF, 0xFF, 255);
    SDL_Rect canvas_rect = { imp->canvas.x, imp->canvas.y, imp->canvas.w, imp->canvas.h };
    SDL_RenderFillRect(imp->renderer, &canvas_rect);

    imp_buttonmenu_render(imp->renderer, imp->button_menu);
}


static void imp_print(ImpRetCode ret) {
    char *message[69];
    switch (ret) {
    case IMP_NORMAL_EXIT: break;
    }
    printf("FIXME");
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

    Imp *imp = create_imp(renderer);
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

        imp_render(imp, window);
        SDL_RenderPresent(renderer);

        uint64_t t1 = SDL_GetTicks64();
        SDL_Delay(MAX(10, dt - (t1 - t0)));
    }
}