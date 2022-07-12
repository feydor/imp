#include "toolmenu.h"
#include <SDL2/SDL_image.h>

#define N_TOOL_BUTTONS 12
#define W_TOOL_BUTTON 48
#define H_TOOL_BUTTON 48

typedef enum ImpTool {
    IMP_TOOL_NOTHING,
    IMP_TOOL_CURSOR,
    IMP_TOOL_PENCIL,
} ImpTool;

#define BUTTON_0_TOOL IMP_TOOL_CURSOR
#define BUTTON_1_TOOL IMP_TOOL_PENCIL

typedef struct ImpToolButton {
    SDL_Texture *texture;
    SDL_Rect rect;
    ImpTool tool;
} ImpToolButton;

typedef struct ImpToolMenu {
    ImpToolButton **buttons;
    int w_button, h_button;
    int n;
    int selected;
    SDL_Rect rect;
    SDL_Texture *bg;
    SDL_Rect bg_rect;
} ImpToolMenu;

ImpToolMenu *create_imp_toolmenu(SDL_Renderer *renderer, ImpCanvas *canvas, char *bg_path) {
    ImpToolMenu *menu = malloc(sizeof(ImpToolMenu));
    if (!menu) {
        return NULL;
    }

    // margin == outside background, padding == inside background
    int xpadding = 12, ypadding = 20;
    int margin = 24;
    menu->n = N_TOOL_BUTTONS;
    menu->w_button = W_TOOL_BUTTON;
    menu->h_button = H_TOOL_BUTTON;
    menu->rect.x = canvas->rect.x - margin - xpadding - menu->w_button;
    menu->rect.y = canvas->rect.y;
    menu->rect.w = menu->w_button;
    menu->rect.h = menu->n * menu->h_button;

    SDL_Surface *bg_surf = IMG_Load(bg_path);
    menu->bg_rect = (SDL_Rect){0};
    menu->bg_rect.x = menu->rect.x - xpadding;
    menu->bg_rect.y = menu->rect.y - ypadding;
    if (bg_surf) {
        menu->bg_rect.w = bg_surf->w;
        menu->bg_rect.h = bg_surf->h;
    }
    menu->bg = SDL_CreateTextureFromSurface(renderer, bg_surf);

    if ((menu->buttons = malloc(sizeof(ImpToolButton *) * menu->n)) == NULL) {
        return NULL;
    }

    int ypadding_button = 10;
    for (int i = 0; i < menu->n; ++i) {
        char path[255];
        sprintf(path, "../res/icons/tool-button%d.bmp", i);
        SDL_Surface *surf = SDL_LoadBMP(path);
        SDL_Texture *text = NULL;
        if (!surf) {
            fprintf(stderr, "failed to load texture from file: '%s'\n", path);
            text = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, menu->w_button, menu->h_button);
        } else {
            text = SDL_CreateTextureFromSurface(renderer, surf);
        }
        
        ImpToolButton *button = malloc(sizeof(ImpToolButton));
        if (!button) {
            return NULL;
        }
        button->texture = text;
        button->tool = IMP_TOOL_NOTHING;
        button->rect.x = menu->rect.x;
        button->rect.y = menu->rect.y + i*menu->h_button + i*ypadding_button;
        button->rect.w = menu->w_button;
        button->rect.h = menu->h_button;

        menu->buttons[i] = button;
        SDL_FreeSurface(surf);
    }

    // set hardcoded tools/functions
    menu->selected = 0;
    menu->buttons[0]->tool = BUTTON_0_TOOL;
    menu->buttons[1]->tool = BUTTON_1_TOOL;
    
    return menu;
}

static void imp_toolmenu_dispatch(ImpToolMenu *menu, ImpToolButton *b, int i, ImpCursor *cursor) {
    switch(b->tool) {
    case IMP_TOOL_CURSOR: {
        cursor->mode = IMP_CURSOR;
        menu->selected = i;
    } break;
    case IMP_TOOL_PENCIL: {
        cursor->mode = IMP_PENCIL;
        menu->selected = i;
    } break;
    default: printf("button task not implemented.\n"); break;
    }
}

void imp_toolmenu_event(ImpToolMenu *menu, SDL_Event *e, ImpCursor *cursor) {
    switch (e->type) {
    case SDL_MOUSEBUTTONDOWN: {
        if (SDL_HasIntersection(&menu->rect, &cursor->rect)) {
            for (int i = 0; i < menu->n; ++i) {
                ImpToolButton *b = menu->buttons[i];
                if (SDL_HasIntersection(&b->rect, &cursor->rect)) {
                    imp_toolmenu_dispatch(menu, menu->buttons[i], i, cursor);
                }
            }
        }
    } break;
    }
}

void imp_toolmenu_render(SDL_Renderer *renderer, ImpToolMenu *menu) {
    SDL_RenderCopy(renderer, menu->bg, NULL, &menu->bg_rect);

    for (int i = 0; i < menu->n; ++i) {
        ImpToolButton *button = menu->buttons[i];
        SDL_RenderCopy(renderer, button->texture, NULL, &button->rect);
        if (i == menu->selected) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
            SDL_RenderFillRect(renderer, &button->rect);
        }
    }
}
