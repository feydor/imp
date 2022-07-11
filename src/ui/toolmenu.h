#ifndef IMP_TOOLMENU_H
#define IMP_TOOLMENU_H
#include <SDL2/SDL.h>
#include "canvas.h"

typedef enum ImpTool ImpTool;
typedef struct ImpToolButton ImpToolButton;
typedef struct ImpToolMenu ImpToolMenu;

ImpToolMenu *create_imp_toolmenu(SDL_Renderer *renderer, ImpCanvas *canvas, char *bg_path);
void imp_toolmenu_event(ImpToolMenu *menu, SDL_Event *e, ImpCursor *cursor);
void imp_toolmenu_render(SDL_Renderer *renderer, ImpToolMenu *menu);

#endif