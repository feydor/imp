
#include "imp.h"
#include "button_bar.h"
#include <assert.h>

#define DEFAULT_BUTTON_W 64
#define DEFAULT_BUTTON_H 64

ImpButtonMenu *create_imp_button_menu(SDL_Renderer *renderer, SDL_Point loc, int N,
        ImpButtonMenuOrientation orientation, ImpButtonMenuDirection direction) {

    ImpButtonMenu *menu = malloc(sizeof(ImpButtonMenu));
    if (!menu) {
        return NULL;
    }
    menu->orientation = orientation;
    menu->direction = direction;

    int menu_w = 0, menu_h = 0;
    char filename[32];
    if (orientation == IMP_HORIZ) {
        menu_w = N * DEFAULT_BUTTON_W;
        menu_h = DEFAULT_BUTTON_H;
        sprintf(filename, "horiz-button");
    } else {
        menu_w = DEFAULT_BUTTON_W;
        menu_h = N * DEFAULT_BUTTON_H;
        sprintf(filename, "vert-button");
    }

    menu->n = N;
    menu->rect = (SDL_Rect){ loc.x, loc.y, menu_w, menu_h };
    menu->buttons = malloc(sizeof(ImpButton *) * menu->n);
    if (!menu->buttons) {
        return NULL;
    }

    for (int i = 0; i < menu->n; ++i) {
        char path[255];
        sprintf(path, "../res/icons/%s%d.bmp", filename, i);
        SDL_Surface *surf = SDL_LoadBMP(path);
        if (!surf) {
            fprintf(stderr, "failed to load texture from file: '%s'\n", path);
        }
        SDL_Texture *text = SDL_CreateTextureFromSurface(renderer, surf);
        ImpButton *button = malloc(sizeof(ImpButton));
        if (!button) {
            return NULL;
        }

        button->texture = text;
        button->w = DEFAULT_BUTTON_W;
        button->h = DEFAULT_BUTTON_H;
        menu->buttons[i] = button;

        SDL_FreeSurface(surf);
    }
    
    return menu;
}

void imp_buttonmenu_render(Imp *imp, ImpButtonMenu *menu) {
    int dx = 0, dy = 0;
    int gap = 2;
    if (menu->orientation == IMP_HORIZ) {
        if (menu->direction == IMP_RIGHTWARDS) {
            dx = DEFAULT_BUTTON_W + gap;
        } else {
            dx = -DEFAULT_BUTTON_W + gap;
        }
    } else {
        if (menu->direction == IMP_DOWNWARDS) {
            dy = DEFAULT_BUTTON_H + gap;
        } else {
            dy = -DEFAULT_BUTTON_H + gap;
        }
    }

    int xoff = 0, yoff = 0;
    for (int i = 0; i < menu->n; ++i) {
        ImpButton *button = menu->buttons[i];
        SDL_Rect rect = {
            menu->rect.x + xoff,
            menu->rect.y + yoff,
            button->w,
            button->h };
        SDL_RenderCopy(imp->renderer, button->texture, NULL, &rect);
        xoff += dx;
        yoff += dy;
    }
}
