#ifndef BUTTON_BAR_H
#define BUTTON_BAR_H

#include <SDL2/SDL.h>

typedef struct ImpButton ImpButton;
typedef struct ImpButtonMenu ImpButtonMenu;

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

ImpButtonMenu *create_imp_button_menu(SDL_Renderer *renderer, SDL_Point loc, int N,
    ImpButtonMenuOrientation orientation, ImpButtonMenuDirection direction);
void imp_buttonmenu_render(SDL_Renderer *renderer, ImpButtonMenu *menu);
#endif