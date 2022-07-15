#include "canvas.h"
#include <stdlib.h>
#include <SDL2/SDL_image.h>

#define W_CANVAS_RESOLUTION 1080
#define H_CANVAS_RESOLUTION 720
#define rgb_red(rgb) ((rgb & 0xFF0000) >> 16)
#define rgb_green(rgb) ((rgb & 0x00FF00) >> 8)
#define rgb_blue(rgb) (rgb & 0x0000FF)
#define ABS(x) ((x) >= 0 ? (x) : -(x))
#define SIGN(_x) ((_x) < 0 ? -1 : \
		                 ((_x) > 0 ? 1 : 0))

typedef struct ImpCircleGuide {
    unsigned x, y, r;
} ImpCircleGuide;

ImpCanvas *create_imp_canvas(SDL_Window *window, SDL_Renderer *renderer, SDL_Texture *layer0_text) {
    ImpCanvas *canvas = malloc(sizeof(ImpCanvas));
    if (!canvas) {
        return NULL;
    }

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    canvas->window_ref = window;
    canvas->rect.x = w/2 - W_CANVAS_RESOLUTION/2 + 50;
    canvas->rect.y = h/2 - H_CANVAS_RESOLUTION/2 - 50;
    canvas->rect.w = W_CANVAS_RESOLUTION;
    canvas->rect.h = H_CANVAS_RESOLUTION;
    
    canvas->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING, canvas->rect.w, canvas->rect.h);
    void *text_bytes;
    int text_pitch;
    SDL_LockTexture(canvas->texture, NULL, &text_bytes, &text_pitch);
    memset((u32 *)text_bytes, 0xFFFFFF, canvas->rect.w * 4*canvas->rect.h);
    SDL_UnlockTexture(canvas->texture);

    int bgoff = 16;
    SDL_Surface *bg = IMG_Load("../res/png/border.png");
    canvas->bg = SDL_CreateTextureFromSurface(renderer, bg);
    canvas->bg_rect = (SDL_Rect){
        canvas->rect.x-bgoff, canvas->rect.y-bgoff, bg->w+bgoff, bg->h+bgoff
    };

    u32 format = SDL_GetWindowPixelFormat(window);
    canvas->pixel_format = SDL_AllocFormat(format);
    canvas->circle_guide = NULL;
    return canvas;
}

static void print_rect(SDL_Rect *r, char *msg) {
    printf("%s: { x: %d, y: %d, w: %d, h: %d }\n", msg, r->x, r->y, r->w, r->h);
}

static int max(int a, int b) { return a > b ? a : b; }

static int min(int a, int b) { return a < b ? a : b; }

// clamp val into range [lower, upper]
static int clamp(int val, int lower, int upper) { return max(lower, min(upper, val)); }

// TODO: check for endianess
u32 imp_rgba(ImpCanvas *c, u32 color) {
    return SDL_MapRGBA(c->pixel_format, rgb_blue(color), rgb_green(color), rgb_red(color), 0xFF);
}

static void imp_canvas_pencil_draw(ImpCanvas *canvas, ImpCursor *cursor) {
    int xrel = cursor->rect.x - canvas->rect.x;
    int yrel = cursor->rect.y - canvas->rect.y;

    // upper bounds
    if (cursor->rect.x + cursor->w_pencil >= canvas->rect.x + canvas->rect.w) {
        xrel = canvas->rect.x + canvas->rect.w - cursor->w_pencil;
    }
    if (cursor->rect.y + cursor->h_pencil >= canvas->rect.y + canvas->rect.h) {
        yrel = canvas->rect.y + canvas->rect.h - cursor->h_pencil;
    }

    // lower bounds
    if (cursor->rect.x < canvas->rect.x) {
        xrel = canvas->rect.x;
    }
    if (cursor->rect.y < canvas->rect.y) {
        yrel = canvas->rect.y;
    }
    SDL_Rect edit_rect = {xrel,
                          yrel,
                          cursor->w_pencil,
                          cursor->h_pencil};

    
    void *raw_data;
    int pitch;
    SDL_LockTexture(canvas->texture, &edit_rect, &raw_data, &pitch);
    u32 *pixels = (u32 *)raw_data;
    int npixels = (pitch / 4) * edit_rect.h;
    for (int i = 0; i < npixels; ++i) {
        pixels[i] = imp_rgba(canvas, cursor->color);
    }
    SDL_UnlockTexture(canvas->texture);
}

static void set_pixel_color(ImpCanvas *c, u32 *pixels, int x, int y, int width, u32 color) {
    if (x > width) {
        fprintf(stderr, "buffer overflow in x: %d (width: %d)", x, width);
    }
    
    pixels[x + y * width] = imp_rgba(c, color);
}

// unused because slow
static void line_gradient(ImpCanvas *canvas, u32 *pixels, int width, u32 color, int x1, int x2, int y1, int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int inc_x = SIGN(dx);
    int inc_y = SIGN(dy);
    dx = abs(dx);
    dy = abs(dy);

    if (dy == 0) {
        // horizontal line
        for (int x = x1; x != x2 + inc_x; x += inc_x) {
            set_pixel_color(canvas, pixels, x, y1, width, color);
        }
    } else if (dx == 0) {
        // vertical line
        for (int y = y1; y != y2 + inc_y; y += inc_y)
            set_pixel_color(canvas, pixels, x1, y, width, color);
    } else if (dx >= dy) {
        // < 45 degrees
        int slope = 2*dy;
        int error = -dx;
        int error_inc = -2*dx;
        int y = y1;

        for (int x = x1; x != x2 + inc_x; x += inc_x) {
            set_pixel_color(canvas, pixels, x, y, width, color);
            error += slope;
            if (error >= 0) {
                y += inc_y;
                error += error_inc;
            }
        }
    } else {
        // > 45 degrees
        int slope = 2*dx;
        int error = -dy;
        int error_inc = -2*dy;
        int x = x1;

        for (int y = y1; y != y2 + inc_y; y += inc_y) {
            set_pixel_color(canvas, pixels, x, y, width, color);
            error += slope;
            if (error >= 0) {
                x += inc_x;
                error += error_inc;
            }
        }
    }
}

static void imp_canvas_draw_line(ImpCanvas *canvas, ImpCursor *cursor, SDL_Point *start) {
    int x1 = start->x - canvas->rect.x;
    int y1 = start->y - canvas->rect.y;
    int x2 = cursor->rect.x - canvas->rect.x;
    int y2 = cursor->rect.y - canvas->rect.y;

    void *rawdata;
    int pitch;
    SDL_LockTexture(canvas->texture, NULL, &rawdata, &pitch);
    u32 *pixels = (u32 *)rawdata;
    line_gradient(canvas, pixels, canvas->rect.w, cursor->color, x1, x2, y1, y2);
    SDL_UnlockTexture(canvas->texture);
}

// midpoint circle algorithm
static void imp_canvas_render_circle(SDL_Renderer *renderer, int32_t centreX, int32_t centreY,
                                     int32_t radius) {
    const int32_t diameter = (radius * 2);

    int32_t x = (radius - 1);
    int32_t y = 0;
    int32_t tx = 1;
    int32_t ty = 1;
    int32_t error = (tx - diameter);

    while (x >= y) {
        // Each of the following renders an octant of the circle
        SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
        SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
        SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
        SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
        SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
        SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
        SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
        SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);

        if (error <= 0) {
            ++y;
            error += ty;
            ty += 2;
        }

        if (error > 0) {
            --x;
            tx += 2;
            error += (tx - diameter);
        }
    }
}

static void midpoint_draw_circle(ImpCanvas *c, u32 *pixels, u32 color, int width, int32_t centreX,
                                 int32_t centreY, int32_t radius) {
    const int32_t diameter = radius * 2;

    int32_t x = radius - 1;
    int32_t y = 0;
    int32_t tx = 1;
    int32_t ty = 1;
    int32_t error = tx - diameter;

    while (x >= y) {
        // Each of the following renders an octant of the circle
        set_pixel_color(c, pixels, centreX + x, centreY - y, width, color);
        set_pixel_color(c, pixels, centreX + x, centreY + y, width, color);
        set_pixel_color(c, pixels, centreX - x, centreY - y, width, color);
        set_pixel_color(c, pixels, centreX - x, centreY + y, width, color);
        set_pixel_color(c, pixels, centreX + y, centreY - x, width, color);
        set_pixel_color(c, pixels, centreX + y, centreY + x, width, color);
        set_pixel_color(c, pixels, centreX - y, centreY - x, width, color);
        set_pixel_color(c, pixels, centreX - y, centreY + x, width, color);

        if (error <= 0) {
            ++y;
            error += ty;
            ty += 2;
        }

        if (error > 0) {
            --x;
            tx += 2;
            error += (tx - diameter);
        }
    }
}

static ImpCircleGuide* imp_canvas_circle_guide(ImpCursor *cursor) {
    ImpCircleGuide *guide = malloc(sizeof(ImpCircleGuide));
    if (!guide) return NULL;
    guide->x = cursor->fixed.x;
    guide->y = cursor->fixed.y;
    guide->r = abs(cursor->fixed.x - cursor->rect.x);
    return guide;
}

static void imp_canvas_rectangle_guide(ImpCanvas *canvas, ImpCursor *cursor) {
    SDL_Rect guide = {0};
    int dx = cursor->rect.x - cursor->fixed.x;
    int dy = cursor->rect.y - cursor->fixed.y;
    guide.w = abs(dx);
    guide.h = abs(dy);

    if (dy > 0) {
        guide.y = cursor->fixed.y;
    } else if (dy < 0) {
        guide.y = cursor->fixed.y + dy;
    }

    if (dx > 0) {
        guide.x = cursor->fixed.x;
    } else if (dx < 0) {
        guide.x = cursor->fixed.x + dx;
    }
    canvas->rectangle_guide = guide;
}

static void imp_canvas_rectange_guide_draw(ImpCanvas *canvas, ImpCursor *cursor) {
    SDL_Rect relative = { fmax(0, canvas->rectangle_guide.x-canvas->rect.x),
                          fmax(0, canvas->rectangle_guide.y-canvas->rect.y),
                          canvas->rectangle_guide.w,
                          canvas->rectangle_guide.h };
    void *raw_data;
    int pitch;
    SDL_LockTexture(canvas->texture, &relative, &raw_data, &pitch);
    u32 *pixels = (u32 *)raw_data;
    int npixels = (pitch / 4) * canvas->rectangle_guide.h;
    for (int i = 0; i < npixels; ++i) {
        pixels[i] = imp_rgba(canvas, cursor->color);
    }
    SDL_UnlockTexture(canvas->texture);
}

void imp_canvas_event(ImpCanvas *canvas, SDL_Event *e, ImpCursor *cursor) {
    switch (e->type) {
    case SDL_MOUSEBUTTONDOWN: {
        if (cursor->pencil_locked) break;

        if (SDL_HasIntersection(&canvas->rect, &cursor->rect)) {
            if (cursor->mode == IMP_PENCIL) {
                cursor->pencil_locked = true;
                imp_canvas_pencil_draw(canvas, cursor);
            } else if (cursor->mode == IMP_RECTANGLE || cursor->mode == IMP_CIRCLE) {
                cursor->pencil_locked = true;
                int xcur, ycur;
                SDL_GetMouseState(&xcur, &ycur);
                cursor->fixed = (SDL_Point){ xcur, ycur };
            }
        }
    } break;

    case SDL_MOUSEMOTION: {
        if (!cursor->pencil_locked) break;
        if (!SDL_HasIntersection(&canvas->rect, &cursor->rect)) break;

        if (cursor->mode == IMP_PENCIL) {
            imp_canvas_pencil_draw(canvas, cursor);
        } else if (cursor->mode == IMP_RECTANGLE) {
            imp_canvas_rectangle_guide(canvas, cursor);
        } else if (cursor->mode == IMP_CIRCLE) {
            canvas->circle_guide = imp_canvas_circle_guide(cursor);
        }
    } break;

    case SDL_MOUSEBUTTONUP: {
        cursor->pencil_locked = false;

        if (cursor->mode == IMP_RECTANGLE) {
            imp_canvas_rectange_guide_draw(canvas, cursor);
            canvas->rectangle_guide = (SDL_Rect){0};
        } else if (cursor->mode == IMP_CIRCLE) {
            if (canvas->circle_guide) {
                // TODO
                // imp_canvas_circle_guide_draw(canvas, cursor);
                free(canvas->circle_guide);
                canvas->circle_guide = NULL;
            }
        }

    } break;
    }
}

void imp_canvas_render(SDL_Renderer *renderer, ImpCanvas *c) {
    SDL_RenderCopy(renderer, c->bg, NULL, &c->bg_rect);
    SDL_RenderCopy(
        renderer, c->texture, NULL,
        &(SDL_Rect){c->rect.x + 0, c->rect.y + 0, c->rect.w - 0, c->rect.h - 0});
    
    SDL_SetRenderDrawColor(renderer, 0xFF, 0, 0xFF, 255);
    SDL_RenderDrawRect(renderer, &c->rectangle_guide);

    if (c->circle_guide) {
        imp_canvas_render_circle(renderer, c->circle_guide->x, c->circle_guide->y, c->circle_guide->r);
    }
}
