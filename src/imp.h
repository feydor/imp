#ifndef IMP_H
#define IMP_H

#include "button_bar.h"
#include "layer.h"
#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>

typedef uint32_t u32;

typedef enum {
    IMP_CURSOR,
    IMP_PENCIL,
} ImpCursorMode;

typedef enum {
    IMP_NORMAL_EXIT = 1,
} ImpRetCode;

typedef struct Imp {
    SDL_Renderer *renderer;
    SDL_Window *window;
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

    ImpButtonMenu **button_menus;
    int n_button_menus;
    ImpLayer **layers;
    int n_layers;
} Imp;

Imp *create_imp(SDL_Renderer *renderer, SDL_Window *window);
int imp_event(Imp *imp, SDL_Event *e);
void imp_update(Imp *imp, float dt);
void imp_render(Imp *imp, SDL_Window *window);

#endif