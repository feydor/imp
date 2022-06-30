#include "imp_button.h"
#include <assert.h>
#include <stdlib.h>

ImpButton *ImpButton_create(int x, int y, int w, int h) {
    assert(w > 0 && h > 0);
    
    ImpButton *button = calloc(1, sizeof(ImpButton));
    if (!button) {
        fprintf(stderr, "Memory allocation failed in Imp_Button_malloc\n");
        return NULL;
    }

    button->surface = NULL;
    button->clicked = NULL;
    button->task = IMP_DISABLED;
    button->pressed = false;
    button->x = x;
    button->y = y;
    button->h = h;
    button->w = w;
    return button;
}


bool ImpButton_mouseover(ImpButton *button, int mouse_x, int mouse_y) {
    return mouse_x > button->x &&
           mouse_x < button->x + button->w &&
           mouse_y > button->y &&
           mouse_y < button->y + button->h;
}


void ImpButton_free(ImpButton *button) {
    assert(button && button->surface);
    free(button->surface);
    free(button);
    button->surface = NULL;
    button = NULL;
}
