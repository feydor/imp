#include "actionmenu.h"
#include <SDL2/SDL_image.h>

#define N_TOOL_BUTTONS 14
#define W_TOOL_BUTTON 64
#define H_TOOL_BUTTON 64

typedef enum ImpAction {
    IMP_ACTION_NOTHING,
} ImpTool;

#define BUTTON_0_TOOL IMP_TOOL_CURSOR
#define BUTTON_1_TOOL IMP_TOOL_PENCIL

typedef struct ImpActionButton {
    SDL_Texture *texture;
    SDL_Rect rect;
    bool clicked;
    ImpTool tool;
} ImpActionButton;

typedef struct ImpActionMenu {
    ImpActionButton **buttons;
    int w_button, h_button;
    int n;
    SDL_Rect rect;
    SDL_Texture *bg;
    SDL_Rect bg_rect;
} ImpActionMenu;

ImpActionMenu *create_imp_actionmenu(SDL_Renderer *renderer, ImpCanvas *canvas, char *bg_path) {
    ImpActionMenu *menu = malloc(sizeof(ImpActionMenu));
    if (!menu) {
        return NULL;
    }

    // margin == outside background, padding == inside background
    int xpadding = 20, ypadding = 20;
    int margin = 48;
    menu->n = N_TOOL_BUTTONS;
    menu->w_button = W_TOOL_BUTTON;
    menu->h_button = H_TOOL_BUTTON;
    menu->rect.x = canvas->x;
    menu->rect.y = canvas->y + canvas->h + margin + ypadding;
    menu->rect.w = menu->n * menu->w_button;
    menu->rect.h = menu->h_button;

    SDL_Surface *bg_surf = IMG_Load(bg_path);
    menu->bg_rect = (SDL_Rect){0};
    menu->bg_rect.x = menu->rect.x - xpadding;
    menu->bg_rect.y = menu->rect.y - ypadding;
    if (bg_surf) {
        menu->bg_rect.w = bg_surf->w;
        menu->bg_rect.h = bg_surf->h;
    }
    menu->bg = SDL_CreateTextureFromSurface(renderer, bg_surf);

    if ((menu->buttons = malloc(sizeof(ImpActionButton *) * menu->n)) == NULL) {
        return NULL;
    }

    int xpadding_button = 6;
    for (int i = 0; i < menu->n; ++i) {
        char path[255];
        sprintf(path, "../res/icons/action-button%d.bmp", i);
        SDL_Surface *surf = SDL_LoadBMP(path);
        SDL_Texture *text = NULL;
        if (!surf) {
            fprintf(stderr, "failed to load texture from file: '%s'\n", path);
            text = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, menu->w_button, menu->h_button);
        } else {
            text = SDL_CreateTextureFromSurface(renderer, surf);
        }
        
        ImpActionButton *button = malloc(sizeof(ImpActionButton));
        if (!button) {
            return NULL;
        }
        button->texture = text;
        button->tool = IMP_ACTION_NOTHING;
        button->clicked = false;
        button->rect.x = menu->rect.x + i*menu->w_button + i*xpadding_button;
        button->rect.y = menu->rect.y;
        button->rect.w = menu->w_button;
        button->rect.h = menu->h_button;

        menu->buttons[i] = button;
        SDL_FreeSurface(surf);
    }

    return menu;
}

void imp_actionmenu_event(ImpActionMenu *menu, SDL_Event *e, ImpCursor *cursor) {
    SDL_Rect cursor_rect = { cursor->x, cursor->y, 1, 1 };

    switch (e->type) {
    case SDL_MOUSEBUTTONDOWN: {
        if (SDL_HasIntersection(&menu->rect, &cursor_rect)) {
            for (int i = 0; i < menu->n; ++i) {
                ImpActionButton *b = menu->buttons[i];
                if (SDL_HasIntersection(&b->rect, &cursor_rect)) {
                    menu->buttons[i]->clicked = true;
                }
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

void imp_actionmenu_render(SDL_Renderer *renderer, ImpActionMenu *menu) {
    SDL_RenderCopy(renderer, menu->bg, NULL, &menu->bg_rect);

    for (int i = 0; i < menu->n; ++i) {
        ImpActionButton *button = menu->buttons[i];
        SDL_RenderCopy(renderer, button->texture, NULL, &button->rect);
        if (button->clicked) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
            SDL_RenderFillRect(renderer, &button->rect);
        }
    }
}
