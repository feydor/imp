
#include "imp.h"
#include "button_bar.h"
#include <assert.h>

#define DEFAULT_BUTTON_W 64
#define DEFAULT_BUTTON_H 64
typedef uint8_t u8;

typedef struct ImpButton {
    SDL_Texture *texture;
    int w, h;
    bool clicked;
    ImpButtonTask task;
} ImpButton;

typedef struct ImpButtonMenu {
    ImpButton **buttons;
    int n;
    int selected;
    SDL_Rect rect;
    ImpButtonMenuOrientation orientation;
    ImpButtonMenuDirection direction;
} ImpButtonMenu;

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
    menu->selected = -1;
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
        button->task = IMP_NOTHING;
        button->w = DEFAULT_BUTTON_W;
        button->h = DEFAULT_BUTTON_H;
        button->clicked = false;
        menu->buttons[i] = button;

        SDL_FreeSurface(surf);
    }
    
    return menu;
}

void imp_buttonmenu_select(ImpButtonMenu *menu, int i) {
    menu->selected = i;
}

void imp_buttonmenu_settask(ImpButtonMenu *menu, int i, ImpButtonTask task) {
    menu->buttons[i]->task = task;
}

void imp_buttonmenu_render(SDL_Renderer *renderer, ImpButtonMenu *menu) {
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
        SDL_RenderCopy(renderer, button->texture, NULL, &rect);

        u8 r, g, b, a;
        SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
        if (button->clicked && i != menu->selected) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
            SDL_RenderFillRect(renderer, &rect);
        } else if (i == menu->selected) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
            SDL_RenderFillRect(renderer, &rect);
        }
        SDL_SetRenderDrawColor(renderer, r, g, b, a);

        xoff += dx;
        yoff += dy;
    }
}

static bool imp_cursor_over_buttonmenu(ImpButtonMenu *menu, ImpCursor *cursor) {
    return SDL_HasIntersection(&menu->rect, &(SDL_Rect){cursor->x, cursor->y, 1, 1});
}

static bool imp_cursor_over_button(SDL_Rect *button_rect, ImpCursor *cursor) {
    return SDL_HasIntersection(button_rect, &(SDL_Rect){cursor->x, cursor->y, 1, 1});
}

static void imp_buttonmenu_dispatchtask(ImpButtonMenu *menu, ImpButton *b, int i, ImpCursor *cursor) {
    switch(b->task) {
    case IMP_SELECT_CURSOR: {
        cursor->mode = IMP_CURSOR;
        menu->selected = i;
    } break;
    case IMP_SELECT_PENCIL: {
        cursor->mode = IMP_PENCIL;
        menu->selected = i;
    } break;
    default: printf("button task not implemented.\n"); break;
    }
}

void imp_buttonmenu_event(ImpButtonMenu *menu, SDL_Event *e, ImpCursor *cursor) {
    switch (e->type) {
    case SDL_MOUSEBUTTONDOWN: {
        if (imp_cursor_over_buttonmenu(menu, cursor)) {
            int dx = 0, dy = 0;
            if (menu->orientation == IMP_HORIZ) {
                if (menu->direction == IMP_RIGHTWARDS) {
                    dx = DEFAULT_BUTTON_W;
                } else {
                    dx = -DEFAULT_BUTTON_W;
                }
            } else {
                if (menu->direction == IMP_DOWNWARDS) {
                    dy = DEFAULT_BUTTON_H;
                } else {
                    dy = -DEFAULT_BUTTON_H;
                }
            }

            int xoff = 0, yoff = 0;
            for (int i = 0; i < menu->n; ++i) {
                SDL_Rect button_rect = {menu->rect.x + xoff,
                                        menu->rect.y + yoff,
                                        DEFAULT_BUTTON_W,
                                        DEFAULT_BUTTON_H };
                if (imp_cursor_over_button(&button_rect, cursor)) {
                    menu->buttons[i]->clicked = true;
                    imp_buttonmenu_dispatchtask(menu, menu->buttons[i], i, cursor);
                }

                xoff += dx;
                yoff += dy;
            }
        }
    } break;

    case SDL_MOUSEBUTTONUP: {
        for (int i = 0; i < menu->n; ++i) {
            menu->buttons[i]->clicked = false;
        }
    } break;
    }
}