#include "canvas.h"
#include <stdlib.h>
#include <SDL2/SDL_image.h>

#define W_CANVAS_RESOLUTION 1080
#define H_CANVAS_RESOLUTION 720
#define SIZE_LINE 2
#define rgb_red(rgb) ((rgb & 0xFF0000) >> 16)
#define rgb_green(rgb) ((rgb & 0x00FF00) >> 8)
#define rgb_blue(rgb) (rgb & 0x0000FF)
#define ABS(x) ((x) >= 0 ? (x) : -(x))
#define SIGN(_x) ((_x) < 0 ? -1 : \
		                 ((_x) > 0 ? 1 : 0))

// rgb encoded color
u32 imp_rgba(ImpCanvas *c, u32 color) {
    return SDL_MapRGBA(c->pixel_format, rgb_red(color), rgb_green(color), rgb_blue(color), 255);
}

ImpCanvas *create_imp_canvas(SDL_Window *window, SDL_Renderer *renderer) {
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

    u32 pixel_format;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    pixel_format = SDL_PIXELFORMAT_RGBA32;
    canvas->masks.r = 0x0000FF00;
    canvas->masks.g = 0x00FF0000;
    canvas->masks.b = 0xFF000000;
#else
    pixel_format = SDL_PIXELFORMAT_ABGR32;
    canvas->masks.r = 0xFF000000;
    canvas->masks.g = 0x00FF0000;
    canvas->masks.b = 0x0000FF00;
#endif
    canvas->pixel_format = SDL_AllocFormat(pixel_format);
    canvas->pitch = 4 * canvas->rect.w;
    canvas->depth = 32;
    canvas->surf = SDL_CreateRGBSurface(0, canvas->rect.w, canvas->rect.h, canvas->depth,
                                        canvas->masks.r, canvas->masks.g, canvas->masks.b, 0xFF);
    SDL_FillRect(canvas->surf, NULL, 0xFFFFFFFF);

    int bgoff = 16;
    SDL_Surface *bg = IMG_Load("../res/png/border.png");
    canvas->bg = SDL_CreateTextureFromSurface(renderer, bg);
    canvas->bg_rect = (SDL_Rect){
        canvas->rect.x-bgoff, canvas->rect.y-bgoff, bg->w+bgoff, bg->h+bgoff
    };

    canvas->circle_guide = (ImpCircleGuide){0};
    canvas->line_guide = (ImpLineGuide){0};
    canvas->size_line = SIZE_LINE;
    return canvas;
}

static void print_rect(SDL_Rect *r, char *msg) {
    printf("%s: { x: %d, y: %d, w: %d, h: %d }\n", msg, r->x, r->y, r->w, r->h);
}

static int max(int a, int b) { return a > b ? a : b; }

static int min(int a, int b) { return a < b ? a : b; }

// clamp val into range [lower, upper]
static int clamp(int val, int lower, int upper) { return max(lower, min(upper, val)); }

static void imp_canvas_pencil_draw(ImpCanvas *canvas, ImpCursor *cursor) {
    int xrel = cursor->rect.x - canvas->rect.x;
    int yrel = cursor->rect.y - canvas->rect.y;
    SDL_Rect area = {xrel, yrel, cursor->w_pencil, cursor->h_pencil};
    SDL_FillRect(canvas->surf, &area, imp_rgba(canvas, cursor->color));
}

static void set_pixel_color(ImpCanvas *c, u32 *pixels, int x, int y, int width, u32 color) {
    if (x > width) {
        fprintf(stderr, "buffer overflow in x: %d (width: %d)\n", x, width);
    }
    
    pixels[x + y * width] = imp_rgba(c, color);
}

// unused because slow
static void line_gradient(ImpCanvas *c, SDL_Surface *surf, u32 color, int x1, int x2, int y1, int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int inc_x = SIGN(dx);
    int inc_y = SIGN(dy);
    dx = abs(dx);
    dy = abs(dy);

    if (dy == 0) {
        // horizontal line
        for (int x = x1; x != x2 + inc_x; x += inc_x) {
            SDL_FillRect(surf, &(SDL_Rect){ x, y1, c->size_line, c->size_line }, imp_rgba(c, color));
        }
    } else if (dx == 0) {
        // vertical line
        for (int y = y1; y != y2 + inc_y; y += inc_y) {
            SDL_FillRect(surf, &(SDL_Rect){ x1, y, c->size_line, c->size_line }, imp_rgba(c, color));
        }
    } else if (dx >= dy) {
        // < 45 degrees
        int slope = 2*dy;
        int error = -dx;
        int error_inc = -2*dx;
        int y = y1;

        for (int x = x1; x != x2 + inc_x; x += inc_x) {
            SDL_FillRect(surf, &(SDL_Rect){ x, y, c->size_line, c->size_line }, imp_rgba(c, color));
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
            SDL_FillRect(surf, &(SDL_Rect){ x, y, c->size_line, c->size_line }, imp_rgba(c, color));
            error += slope;
            if (error >= 0) {
                x += inc_x;
                error += error_inc;
            }
        }
    }
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

static void imp_canvas_circle_guide(ImpCanvas *canvas, ImpCursor *cursor) {
    ImpCircleGuide guide = {0};
    guide.x = cursor->fixed.x;
    guide.y = cursor->fixed.y;
    guide.r = abs(cursor->fixed.x - cursor->rect.x);
    canvas->circle_guide = guide;
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

static void imp_canvas_line_guide(ImpCanvas *canvas, ImpCursor *cursor) {
    ImpLineGuide guide = {0};
    guide.x1 = cursor->fixed.x;
    guide.y1 = cursor->fixed.y;
    guide.x2 = cursor->rect.x;
    guide.y2 = cursor->rect.y;
    canvas->line_guide = guide;
}

static void imp_canvas_circle_guide_draw(ImpCanvas *canvas, ImpCursor *cursor) {
    size_t w_rect = 2*canvas->circle_guide.r;
    size_t x_rect = w_rect/2;
    size_t y_rect = w_rect/2;
    uint32_t pixels[w_rect * w_rect];
    for (size_t i = 0; i < w_rect * w_rect; ++i) {
        size_t x = i % w_rect;
        size_t y = i / w_rect;
        if (pow(abs(x - x_rect), 2) + pow(abs(y - y_rect), 2) <= pow(canvas->circle_guide.r, 2)) {
            pixels[i] = imp_rgba(canvas, cursor->color);
        } else {
            // set to black and transparent
            pixels[i] = 0;
        }
    }

    SDL_Surface *circ_surf = SDL_CreateRGBSurfaceFrom(pixels, w_rect, w_rect, 32, 4 * w_rect, canvas->masks.r,
                                                      canvas->masks.g, canvas->masks.b, 0xFF);
    size_t xrel = canvas->circle_guide.x - canvas->rect.x;
    size_t yrel = canvas->circle_guide.y - canvas->rect.y;
    SDL_BlitSurface(circ_surf, NULL, canvas->surf,
                    &(SDL_Rect){xrel - canvas->circle_guide.r,
                                yrel - canvas->circle_guide.r, w_rect, w_rect});
    SDL_FreeSurface(circ_surf);
}

static void imp_canvas_rectange_guide_draw(ImpCanvas *canvas, ImpCursor *cursor) {
    SDL_Rect relative = { fmax(0, canvas->rectangle_guide.x - canvas->rect.x),
                          fmax(0, canvas->rectangle_guide.y - canvas->rect.y),
                          canvas->rectangle_guide.w,
                          canvas->rectangle_guide.h };
    SDL_FillRect(canvas->surf, &relative, imp_rgba(canvas, cursor->color));
}

static void imp_canvas_line_guide_draw(ImpCanvas *canvas, ImpCursor *cursor) {
    size_t w = canvas->rect.w;
    size_t h = canvas->rect.h;

    // literally making a copy of the entire canvas just to render a line lmao
    // if noticable, use smaller buffer and map canvas coordinates into smaller buffer coordinates
    SDL_Surface *line_surf =
        SDL_CreateRGBSurface(0, w, h, 32, canvas->masks.r, canvas->masks.g, canvas->masks.b, 0xFF);
    line_gradient(canvas, line_surf, cursor->color, canvas->line_guide.x1 - canvas->rect.x,
                  canvas->line_guide.x2 - canvas->rect.x, canvas->line_guide.y1 - canvas->rect.y,
                  canvas->line_guide.y2 - canvas->rect.y);
    SDL_BlitSurface(line_surf, NULL, canvas->surf, NULL);
    SDL_FreeSurface(line_surf);
}

void imp_canvas_event(ImpCanvas *canvas, SDL_Event *e, ImpCursor *cursor) {
    switch (e->type) {
    case SDL_MOUSEBUTTONDOWN: {
        if (cursor->pencil_locked) break;

        if (SDL_HasIntersection(&canvas->rect, &cursor->rect)) {
            if (cursor->mode == IMP_PENCIL) {
                cursor->pencil_locked = true;
                imp_canvas_pencil_draw(canvas, cursor);
            } else if (cursor->mode == IMP_RECTANGLE || cursor->mode == IMP_CIRCLE || cursor->mode == IMP_LINE) {
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
            imp_canvas_circle_guide(canvas, cursor);
        } else if (cursor->mode == IMP_LINE) {
            imp_canvas_line_guide(canvas, cursor);
        }
    } break;

    case SDL_MOUSEBUTTONUP: {
        cursor->pencil_locked = false;

        if (cursor->mode == IMP_RECTANGLE) {
            imp_canvas_rectange_guide_draw(canvas, cursor);
            canvas->rectangle_guide = (SDL_Rect){0};
        } else if (cursor->mode == IMP_CIRCLE) {
            imp_canvas_circle_guide_draw(canvas, cursor);
            canvas->circle_guide = (ImpCircleGuide){0};
        } else if (cursor->mode == IMP_LINE) {
            imp_canvas_line_guide_draw(canvas, cursor);
            canvas->line_guide = (ImpLineGuide){0};
        }

    } break;
    }
}

void imp_canvas_render(SDL_Renderer *renderer, ImpCanvas *c) {
    SDL_RenderCopy(renderer, c->bg, NULL, &c->bg_rect);

    SDL_Texture *canvas_texture = SDL_CreateTextureFromSurface(renderer, c->surf);
    SDL_RenderCopy(renderer, canvas_texture, NULL, &c->rect);
    SDL_DestroyTexture(canvas_texture);

    SDL_SetRenderDrawColor(renderer, 0xFF, 0, 0xFF, 255);
    SDL_RenderDrawRect(renderer, &c->rectangle_guide);
    SDL_RenderDrawLine(renderer, c->line_guide.x1, c->line_guide.y1, c->line_guide.x2, c->line_guide.y2);
    imp_canvas_render_circle(renderer, c->circle_guide.x, c->circle_guide.y, c->circle_guide.r);
}
