
#include "imp.h"
#include "canvas.h"
#include "buttonpanel.h"
#include <assert.h>
#include <SDL2/SDL_image.h>

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
    SDL_Texture *bg;
    int w_bg, h_bg;
    ImpButtonMenuOrientation orientation;
    ImpButtonMenuDirection direction;
} ImpButtonMenu;

ImpButtonMenu *create_imp_buttonmenu(SDL_Renderer *renderer, SDL_Point loc, int N, int h_button, int w_button,
        char *bg_path, ImpButtonMenuOrientation orientation, ImpButtonMenuDirection direction) {
    
    ImpButtonMenu *menu = malloc(sizeof(ImpButtonMenu));
    if (!menu) {
        return NULL;
    }
    menu->orientation = orientation;
    menu->direction = direction;

    int menu_w = 0, menu_h = 0;
    char filename[32];
    if (orientation == IMP_HORIZ) {
        menu_w = N * w_button;
        menu_h = h_button;
        sprintf(filename, "horiz-button");
    } else {
        menu_w = w_button;
        menu_h = N * h_button;
        sprintf(filename, "vert-button");
    }

    SDL_Surface *bg_surf = IMG_Load(bg_path);
    if (!bg_surf) {
        fprintf(stderr, "failed to load file: %s", bg_path);
        menu->w_bg = 0, menu->h_bg = 0;
    } else {
        menu->w_bg = bg_surf->w;
        menu->h_bg = bg_surf->h;
    }

    menu->bg = SDL_CreateTextureFromSurface(renderer, bg_surf);
    menu->n = N;
    menu->selected = -1;
    menu->rect = (SDL_Rect){ loc.x, loc.y, menu_w, menu_h };
    if ((menu->buttons = malloc(sizeof(ImpButton *) * menu->n)) == NULL) {
        return NULL;
    }

    for (int i = 0; i < menu->n; ++i) {
        char path[255];
        sprintf(path, "../res/icons/%s%d.bmp", filename, i);
        SDL_Surface *surf = SDL_LoadBMP(path);
        SDL_Texture *text = NULL;
        if (!surf) {
            fprintf(stderr, "failed to load texture from file: '%s'\n", path);
            text = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, w_button, h_button);
        } else {
            text = SDL_CreateTextureFromSurface(renderer, surf);
        }
        
        ImpButton *button = malloc(sizeof(ImpButton));
        if (!button) {
            return NULL;
        }
        button->texture = text;
        button->task = IMP_NOTHING;
        button->w = w_button;
        button->h = h_button;
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
    int gap = 5;
    int w_but = menu->buttons[0]->w, h_but = menu->buttons[0]->h;
    if (menu->orientation == IMP_HORIZ) {
        if (menu->direction == IMP_RIGHTWARDS) {
            dx = w_but + gap;
        } else {
            dx = -w_but + gap;
        }
    } else {
        if (menu->direction == IMP_DOWNWARDS) {
            dy = h_but + gap;
        } else {
            dy = -h_but + gap;
        }
    }

    SDL_Rect bg_rect = { menu->rect.x, menu->rect.y, menu->w_bg, menu->h_bg };
    SDL_RenderCopy(renderer, menu->bg, NULL, &bg_rect);

    // FIXME: hardcoding of offsets for specific bg textures
    int xoff = 12, yoff = 20;
    if (menu->orientation == IMP_HORIZ) {
        xoff = 24, yoff = 20;
    }
    
    for (int i = 0; i < menu->n; ++i) {
        ImpButton *button = menu->buttons[i];
        SDL_Rect rect = {
            menu->rect.x + xoff,
            menu->rect.y + yoff,
            button->w,
            button->h };
        SDL_RenderCopy(renderer, button->texture, NULL, &rect);

        if (button->clicked && i != menu->selected) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
            SDL_RenderFillRect(renderer, &rect);
        } else if (i == menu->selected) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
            SDL_RenderFillRect(renderer, &rect);
        }

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
            int w_but = menu->buttons[0]->w, h_but = menu->buttons[0]->h;
            if (menu->orientation == IMP_HORIZ) {
                if (menu->direction == IMP_RIGHTWARDS) {
                    dx = w_but;
                } else {
                    dx = -w_but;
                }
            } else {
                if (menu->direction == IMP_DOWNWARDS) {
                    dy = h_but;
                } else {
                    dy = -h_but;
                }
            }

            // FIXME: hardcoding of offsets for specific bg textures
            // FIXME: refactor into a function (duplicate code with render)
            int xoff = 12, yoff = 20;
            if (menu->orientation == IMP_HORIZ) {
                xoff = 24, yoff = 20;
            }
            for (int i = 0; i < menu->n; ++i) {
                SDL_Rect button_rect = {menu->rect.x + xoff,
                                        menu->rect.y + yoff,
                                        menu->buttons[i]->w,
                                        menu->buttons[i]->h };
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