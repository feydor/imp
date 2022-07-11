#ifndef IMP_ACTIONMENU_H
#define IMP_ACTIONMENU_H

#include <SDL2/SDL.h>
#include "canvas.h"

typedef enum ImpAction ImpAction;
typedef struct ImpActionButton ImpActionButton;
typedef struct ImpActionMenu ImpActionMenu;

ImpActionMenu *create_imp_actionmenu(SDL_Renderer *renderer, ImpCanvas *canvas, char *bg_path);
void imp_actionmenu_event(ImpActionMenu *menu, SDL_Event *e, ImpCursor *cursor);
void imp_actionmenu_render(SDL_Renderer *renderer, ImpActionMenu *menu);

#endif