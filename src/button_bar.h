#ifndef BUTTON_BAR_H
#define BUTTON_BAR_H

#include <SDL2/SDL.h>
typedef struct Imp Imp;

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

typedef struct {
    SDL_Texture *texture;
    int w, h;
} ImpButton;

typedef struct {
    ImpButton **buttons;
    int n;
    SDL_Rect rect;
    ImpButtonMenuOrientation orientation;
    ImpButtonMenuDirection direction;
} ImpButtonMenu;

ImpButtonMenu *create_imp_button_menu(SDL_Renderer *renderer, SDL_Point loc, int N,
    ImpButtonMenuOrientation orientation, ImpButtonMenuDirection direction);
void imp_buttonmenu_render(Imp *imp, ImpButtonMenu *menu);
#endif