#ifndef IMP_TOOLMENU_H
#define IMP_TOOLMENU_H
#include <SDL2/SDL.h>
#include "canvas.h"

typedef struct ImpToolButton ImpToolButton;
typedef struct ImpToolMenu ImpToolMenu;

typedef enum ImpTool {
    IMP_TOOL_NOTHING,
    IMP_TOOL_CURSOR,
    IMP_TOOL_PENCIL,
    IMP_TOOL_RECTANGLE,
    IMP_TOOL_CIRCLE,
    IMP_TOOL_LINE,
    IMP_TOOL_BUCKET,
    IMP_TOOL_STAMP,
    IMP_TOOL_SAVE,
} ImpTool;

ImpToolMenu *create_imp_toolmenu(SDL_Renderer *renderer, ImpCanvas *canvas, char *bg_path);
ImpTool imp_toolmenu_event(ImpToolMenu *menu, SDL_Event *e, ImpCursor *cursor);
void imp_toolmenu_render(SDL_Renderer *renderer, ImpToolMenu *menu);

#endif