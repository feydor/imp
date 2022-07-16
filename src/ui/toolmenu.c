#include "toolmenu.h"
#include <SDL2/SDL_image.h>

#define N_TOOL_BUTTONS 11
#define W_TOOL_BUTTON 48
#define H_TOOL_BUTTON 48
#define BUTTON_0_TOOL IMP_TOOL_PENCIL
#define BUTTON_1_TOOL IMP_TOOL_RECTANGLE
#define BUTTON_2_TOOL IMP_TOOL_CIRCLE
#define BUTTON_3_TOOL IMP_TOOL_LINE
#define BUTTON_4_TOOL IMP_TOOL_BUCKET

typedef struct ImpToolButton {
    SDL_Texture *texture;
    SDL_Rect rect;
    ImpTool tool;
} ImpToolButton;

typedef struct ImpToolMenu {
    ImpToolButton **buttons;
    ImpTool current_tool;
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
    int margin = 40;
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
        sprintf(path, "../res/icons/tool-button%d.webp", i);
        SDL_Surface *surf = IMG_Load(path);
        SDL_Texture *text = NULL;
        if (!surf) {
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
    menu->current_tool = BUTTON_0_TOOL;
    menu->buttons[0]->tool = menu->current_tool;
    menu->buttons[1]->tool = BUTTON_1_TOOL;
    menu->buttons[2]->tool = BUTTON_2_TOOL;
    menu->buttons[3]->tool = BUTTON_3_TOOL;
    menu->buttons[4]->tool = BUTTON_4_TOOL;
    
    return menu;
}

static void imp_toolmenu_dispatch(ImpToolMenu *menu, ImpToolButton *b, int i, ImpCursor *cursor) {
    menu->selected = i;
    menu->current_tool = b->tool;

    switch(b->tool) {
    case IMP_TOOL_CURSOR: cursor->mode = IMP_CURSOR; break;
    case IMP_TOOL_PENCIL: cursor->mode = IMP_PENCIL; break;
    case IMP_TOOL_RECTANGLE: cursor->mode = IMP_RECTANGLE; break;
    case IMP_TOOL_CIRCLE: cursor->mode = IMP_CIRCLE; break;
    case IMP_TOOL_LINE: cursor->mode = IMP_LINE; break;
    case IMP_TOOL_BUCKET: cursor->mode = IMP_BUCKET; break;
    default: printf("button task not implemented.\n"); break;
    }
}

// returns non-zero when a tool change occurs
ImpTool imp_toolmenu_event(ImpToolMenu *menu, SDL_Event *e, ImpCursor *cursor) {
    ImpTool start_tool = menu->current_tool;

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

    if (menu->current_tool != start_tool) {
        return menu->current_tool;
    }
    return 0;
}

void imp_toolmenu_render(SDL_Renderer *renderer, ImpToolMenu *menu) {
    SDL_RenderCopy(renderer, menu->bg, NULL, &menu->bg_rect);

    for (int i = 0; i < menu->n; ++i) {
        ImpToolButton *button = menu->buttons[i];
        SDL_RenderCopy(renderer, button->texture, NULL, &button->rect);
        if (i == menu->selected) {
            // TODO: render a slightly larger pre-loaded verison, about 10 px
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 50);
            SDL_RenderFillRect(renderer, &button->rect);
        }
    }
}
