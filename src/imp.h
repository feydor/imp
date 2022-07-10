#ifndef IMP_H
#define IMP_H

#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>

typedef uint32_t u32;

typedef struct Imp Imp;
typedef enum ImpCursorMode ImpCursorMode;

Imp *create_imp(SDL_Renderer *renderer, SDL_Window *window, SDL_Texture *layer0_texture);
int imp_event(Imp *imp, SDL_Event *e);
void imp_update(Imp *imp, float dt);
void imp_render(Imp *imp, SDL_Window *window);

int fill_rounded_box_b(SDL_Surface *dst, int xo, int yo, int w, int h, int r, Uint32 color);

#endif