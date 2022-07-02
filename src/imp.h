#ifndef IMP_H
#define IMP_H

#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>

typedef uint32_t u32;

typedef struct Imp Imp;

typedef struct  {
    int x, y, w, h;
    float dw, dh;
    SDL_Texture *texture;
} ImpCanvas;

Imp *create_imp(SDL_Renderer *renderer, SDL_Window *window);
int imp_event(Imp *imp, SDL_Event *e);
void imp_update(Imp *imp, float dt);
void imp_render(Imp *imp, SDL_Window *window);

#endif