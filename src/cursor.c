#include "cursor.h"

#define W_CURSOR 1
#define H_CURSOR 1
#define DEFAULT_PENCIL_SIZE 20
#define DEFAULT_PENCIL_COLOR 0x000000

ImpCursor *create_imp_cursor() {
    // TODO: pass in a texture/path to create a custom cursor texture

    ImpCursor *cursor = malloc(sizeof(ImpCursor));
    if (!cursor) {
        return NULL;
    }

    cursor->rect = (SDL_Rect){ 0, 0, W_CURSOR, H_CURSOR };
    cursor->fixed = (SDL_Point){0};
    cursor->mode = IMP_PENCIL;
    cursor->scroll_locked = false;
    cursor->pencil_locked = false;
    cursor->w_pencil = cursor->h_pencil = DEFAULT_PENCIL_SIZE;
    cursor->color = DEFAULT_PENCIL_COLOR;

    return cursor;
}