#ifndef IMP_COLORMENU_H
#define IMP_COLORMENU_H

#include "canvas.h"
#include <SDL2/SDL.h>

typedef struct ImpColorButton ImpColorButton;
typedef struct ImpColorDisplay ImpColorDisplay;
typedef struct ImpColorMenu ImpColorMenu;

ImpColorMenu *create_imp_colormenu(SDL_Renderer *renderer, ImpCanvas *canvas);
void imp_colormenu_event(ImpColorMenu *menu, SDL_Event *e, ImpCursor *cursor);
void imp_colormenu_render(SDL_Renderer *renderer, ImpColorMenu *menu);

#endif