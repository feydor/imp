#ifndef BUTTON_BAR_H
#define BUTTON_BAR_H

#include <SDL2/SDL.h>

typedef struct ImpButton ImpButton;
typedef struct ImpButtonMenu ImpButtonMenu;
typedef struct ImpCursor ImpCursor;

typedef enum {
    IMP_NOTHING,
    IMP_SELECT_CURSOR,
    IMP_SELECT_PENCIL,
    IMP_SAVE,
} ImpButtonTask;

typedef enum {
    IMP_HORIZ,
    IMP_VERT
} ImpButtonMenuOrientation;

typedef enum {
    IMP_DOWNWARDS,
    IMP_UPWARDS,
    IMP_RIGHTWARDS,
    IMP_LEFTWARDS,
} ImpButtonMenuDirection;

ImpButtonMenu *create_imp_buttonmenu(SDL_Renderer *renderer, SDL_Point loc, int N, int h_button, int w_button,
        char *bg_path, ImpButtonMenuOrientation orientation, ImpButtonMenuDirection direction);
void imp_buttonmenu_render(SDL_Renderer *renderer, ImpButtonMenu *menu);
void imp_buttonmenu_select(ImpButtonMenu *menu, int i);
void imp_buttonmenu_settask(ImpButtonMenu *menu, int i, ImpButtonTask task);
void imp_buttonmenu_event(ImpButtonMenu *menu, SDL_Event *e, ImpCursor *cursor);
#endif