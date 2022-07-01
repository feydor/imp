#ifndef IMP_H
#define IMP_H

#include <stdint.h>
#include <SDL2/SDL.h>

typedef uint32_t u32;

typedef struct Imp Imp;

Imp *create_imp(SDL_Renderer *renderer);
int imp_event(Imp *imp, SDL_Event *e);
void imp_update(Imp *imp, float dt);
void imp_render(Imp *imp, SDL_Window *window);

#endif