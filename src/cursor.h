#ifndef IMP_CURSOR_H
#define IMP_CURSOR_H

#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint32_t u32;

typedef enum ImpCursorMode {
    IMP_CURSOR,
    IMP_PENCIL,
} ImpCursorMode;
                                                                         
typedef struct ImpCursor {
    SDL_Rect rect;
    ImpCursorMode mode;
    u32 color;
    int w_pencil, h_pencil;
    bool scroll_locked;
    bool pencil_locked;
} ImpCursor;

ImpCursor *create_imp_cursor();

#endif