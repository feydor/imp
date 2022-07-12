#include "colormenu.h"
#include <SDL2/SDL_image.h>

#define BG_PATH "../res/png/color-menu-vert-jp-lavender.png"
#define N_COLORS 16
#define SIZE_COLOR_BUTTON 24
#define H_COLOR_DISPLAY SIZE_COLOR_BUTTON
#define W_COLOR_DISPLAY 2*SIZE_COLOR_BUTTON
#define rgb_red(rgb) ((rgb & 0xFF0000) >> 16)
#define rgb_green(rgb) ((rgb & 0x00FF00) >> 8)
#define rgb_blue(rgb) (rgb & 0x0000FF)

typedef uint32_t u32;

typedef enum ImpColor {
    IMP_COLOR_BLACK = 0x000000,
    IMP_COLOR_GRAY = 0x808080,
    IMP_COLOR_RED = 0xFF0000,
    IMP_COLOR_ORANGE = 0xFFA500,
    IMP_COLOR_YELLOW = 0xFFFF00,
    IMP_COLOR_GREEN = 0x32CD32,
    IMP_COLOR_BLUE = 0x0000FF,
    IMP_COLOR_PURPLE = 0x6A0DAD,

    IMP_COLOR_WHITE = 0xFFFFFF,
    IMP_COLOR_LIGHT_GRAY = 0xb4b5b5,
    IMP_COLOR_LIGHT_RED = 0xffb3ba,
    IMP_COLOR_LIGHT_ORANGE = 0xffdfba,
    IMP_COLOR_LIGHT_YELLOW = 0xffffba,
    IMP_COLOR_LIGHT_GREEN = 0xbaffc9,
    IMP_COLOR_LIGHT_BLUE = 0xbae1ff,
    IMP_COLOR_LIGHT_PURPLE = 0x957DAD,
} ImpColor;

typedef struct ImpColorButton {
    u32 color;
    SDL_Rect rect;
} ImpColorButton;

/* shows the currently selected color */
typedef struct ImpColorDisplay {
    SDL_Rect rect;
} ImpColorDisplay;

typedef struct ImpColorMenu {
    SDL_Rect rect;
    SDL_Texture *bg;
    SDL_Rect bg_rect;
    ImpColorDisplay *display;
    ImpColorButton **buttons;
    int n;
    int selected;
} ImpColorMenu;

ImpColorMenu *create_imp_colormenu(SDL_Renderer *renderer, ImpCanvas *canvas) {
    ImpColorMenu *menu = malloc(sizeof(ImpColorMenu));
    if (!menu) {
        return NULL;
    }

    int xmargin = 36, ymargin = 56;
    menu->rect.x = canvas->rect.x - xmargin - 2*SIZE_COLOR_BUTTON;
    menu->rect.y = canvas->rect.y + canvas->rect.h - ymargin;
    menu->rect.w = 2*SIZE_COLOR_BUTTON;
    menu->rect.h =  SIZE_COLOR_BUTTON * (N_COLORS/2) + H_COLOR_DISPLAY;
    menu->selected = 0;
    menu->n = N_COLORS;
    if ((menu->display = malloc(sizeof(ImpColorDisplay))) == NULL) {
        return NULL;
    }
    menu->display->rect = (SDL_Rect){
        menu->rect.x, menu->rect.y,
        W_COLOR_DISPLAY, H_COLOR_DISPLAY
    };

    if ((menu->buttons = malloc(sizeof(ImpColorButton *) * menu->n)) == NULL) {
        return NULL;
    }

    u32 primary_colors[] = {
        IMP_COLOR_BLACK, IMP_COLOR_GRAY, IMP_COLOR_RED, IMP_COLOR_ORANGE, IMP_COLOR_YELLOW,
        IMP_COLOR_GREEN, IMP_COLOR_BLUE, IMP_COLOR_PURPLE
    };

    // TODO: Use real secondary colors
    u32 secondary_colors[] = {
        IMP_COLOR_WHITE, IMP_COLOR_LIGHT_GRAY, IMP_COLOR_LIGHT_RED, IMP_COLOR_LIGHT_ORANGE,
        IMP_COLOR_LIGHT_YELLOW, IMP_COLOR_LIGHT_GREEN, IMP_COLOR_LIGHT_BLUE, IMP_COLOR_LIGHT_PURPLE
    };

    for (int i = 0; i < menu->n; ++i) {
        ImpColorButton *but = malloc(sizeof(ImpColorButton));
        if (!but) return NULL;

        if (i < 8) {
            but->rect = (SDL_Rect){menu->rect.x, H_COLOR_DISPLAY + menu->rect.y + i * SIZE_COLOR_BUTTON,
                               SIZE_COLOR_BUTTON, SIZE_COLOR_BUTTON};
            but->color = primary_colors[i];
        } else {
            but->rect =
                (SDL_Rect){menu->rect.x + SIZE_COLOR_BUTTON, H_COLOR_DISPLAY + menu->rect.y + (i % 8) * SIZE_COLOR_BUTTON,
                           SIZE_COLOR_BUTTON, SIZE_COLOR_BUTTON};
            but->color = secondary_colors[i%8];
        }
        
        menu->buttons[i] = but;
    }

    int xpadding = 12, ypadding = 12;
    SDL_Surface *bg_surf = IMG_Load(BG_PATH);
    menu->bg = SDL_CreateTextureFromSurface(renderer, bg_surf);
    menu->bg_rect = (SDL_Rect){0};
    menu->bg_rect.x = menu->rect.x - xpadding;
    menu->bg_rect.y = menu->rect.y - ypadding;
    if (menu->bg) {
        menu->bg_rect.w = bg_surf->w;
        menu->bg_rect.h = bg_surf->h;
    }

    return menu;
}

void imp_colormenu_event(ImpColorMenu *menu, SDL_Event *e, ImpCursor *cursor) {
    switch (e->type) {
    case SDL_MOUSEBUTTONDOWN: {
        if (SDL_HasIntersection(&menu->rect, &cursor->rect)) {
            for (int i = 0; i < menu->n; ++i) {
                ImpColorButton *but = menu->buttons[i];
                if (SDL_HasIntersection(&but->rect, &cursor->rect)) {
                    menu->selected = i;
                    cursor->color = but->color;
                }
            }
        }
    }
    }
}

void imp_colormenu_render(SDL_Renderer *renderer, ImpColorMenu *menu) {
    SDL_RenderCopy(renderer, menu->bg, NULL, &menu->bg_rect);
    
    u32 selected_color = menu->buttons[menu->selected]->color;
    SDL_SetRenderDrawColor(renderer, rgb_red(selected_color), rgb_green(selected_color), rgb_blue(selected_color), 255);
    SDL_RenderFillRect(renderer, &menu->display->rect);

    for (int i = 0; i < menu->n; ++i) {
        ImpColorButton *but = menu->buttons[i];
        SDL_SetRenderDrawColor(renderer, rgb_red(but->color), rgb_green(but->color), rgb_blue(but->color), 255);
        SDL_RenderFillRect(renderer, &but->rect);
    }
}