#ifndef IMP_BUTTON_H
#define IMP_BUTTON_H

#include <SDL2/SDL.h>
#include <stdbool.h>

typedef enum {
    IMP_DISABLED,
    IMP_SAVE,
    IMP_ORDERED_DITHERING,
    IMP_PALETTE_QUANTIZATION,
    IMP_INVERT,
    IMP_GRAYSCALE,
    IMP_UNIFORM_NOISE,
    IMP_HORIZ_FLIP,
} ImpButtonTask;

/**
 * @brief
 * must pass to Imp_Button_free() to return memory
 */
typedef struct {
    ImpButtonTask task;
    SDL_Surface *surface; // TODO: store SDL_Texture instead ???
    SDL_Surface *clicked;
    int x;
    int y;
    int w;
    int h;
    bool pressed;
} ImpButton;

/** NULL on failure */
ImpButton *ImpButton_create(int x, int y, int w, int h);
bool ImpButton_mouseover(ImpButton *button, int mouse_x, int mouse_y);
void ImpButton_free(ImpButton *button);

#endif