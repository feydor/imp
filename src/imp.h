#ifndef IMP_H
#define IMP_H

#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>

typedef uint32_t u32;

typedef struct Imp Imp;
typedef enum ImpCursorMode ImpCursorMode;

typedef enum ImpCursorMode {
    IMP_CURSOR,
    IMP_PENCIL,
} ImpCursorMode;

typedef struct  {
    int x, y, w, h;
    float dw, dh;
    SDL_Texture *texture;
} ImpCanvas;

typedef struct {
    int x, y;
    ImpCursorMode mode;
    bool pressed;
} ImpCursor;

Imp *create_imp(SDL_Renderer *renderer, SDL_Window *window, SDL_Texture *layer0_texture);
int imp_event(Imp *imp, SDL_Event *e);
void imp_update(Imp *imp, float dt);
void imp_render(Imp *imp, SDL_Window *window);

#endif