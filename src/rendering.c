#include "rendering.h"

SDL_Window* wnd = NULL;
SDL_Rect wnd_rect = { .w = RENDERING_DEFAULT_WINDOW_WIDTH, .h = RENDERING_DEFAULT_WINDOW_HEIGHT, .x = SDL_WINDOWPOS_UNDEFINED, .y = SDL_WINDOWPOS_UNDEFINED };
SDL_Renderer* rend = NULL;

void rendering_init() {
    // init core sdl lib (we only care about video and events)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "could not initialize SDL:%s\n", SDL_GetError());
        exit(-1);
    }

    // create the sdl window
    uint32_t wnd_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN; 
    wnd = SDL_CreateWindow("10h0ch", wnd_rect.x, wnd_rect.y, wnd_rect.w, wnd_rect.h, wnd_flags);
    if (wnd == NULL) {
        fprintf(stderr, "could not create SDL window: %s\n", SDL_GetError());
        SDL_Quit();
        exit(-1);
    }

    // create our sdl renderer
    rend = SDL_CreateRenderer(wnd, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    if (rend == NULL) {
        fprintf(stderr, "could not create SDL renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(wnd);
        SDL_Quit();
        exit(-1);
    }

}

void rendering_deinit() {
    if (wnd == NULL) return; // we probably didn't even initialize?
    SDL_DestroyWindow(wnd);
    SDL_Quit();
}

void rendering_handle_window_resized() {
     SDL_GetWindowSize(wnd, &wnd_rect.w, &wnd_rect.h);
}

void rendering_wipe_screen() {
    SDL_SetRenderTarget(rend, NULL);
    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(rend, 0x44, 0x44, 0x44, 0xff);
    SDL_RenderClear(rend);
}

void rendering_swap_screen() {
    SDL_RenderPresent(rend);
}

void rendering_draw_circle(uint32_t circle_x, uint32_t circle_y, uint32_t radius, uint8_t r, uint8_t g, uint8_t b, uint8_t a, SDL_BlendMode blend_mode) {
    SDL_SetRenderDrawColor(rend, r, g, b, a);
    SDL_SetRenderDrawBlendMode(rend, blend_mode);

    int32_t center_x = circle_x + radius;
    int32_t center_y = circle_y + radius;

    int32_t x = 0;
    int32_t y = radius;
    int32_t d = 3 - 2 * radius;

    SDL_RenderDrawPoint(rend, center_x + x,  center_y + y);
    SDL_RenderDrawPoint(rend, center_x + y,  center_y + x);
    SDL_RenderDrawPoint(rend, center_x + -x, center_y + -y);
    SDL_RenderDrawPoint(rend, center_x + -y, center_y + -x);

    while (y >= x) {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }

        SDL_RenderDrawPoint(rend, center_x + x,  center_y + -y);
        SDL_RenderDrawPoint(rend, center_x + x,  center_y + y);
        SDL_RenderDrawPoint(rend, center_x + -x, center_y + -y);
        SDL_RenderDrawPoint(rend, center_x + -x, center_y + y);
        SDL_RenderDrawPoint(rend, center_x + y,  center_y + -x);
        SDL_RenderDrawPoint(rend, center_x + y,  center_y + x);
        SDL_RenderDrawPoint(rend, center_x + -y, center_y + -x);
        SDL_RenderDrawPoint(rend, center_x + -y, center_y + x);
    }

    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_NONE);
}

void rendering_fill_circle(uint32_t circle_x, uint32_t circle_y, uint32_t radius, uint8_t r, uint8_t g, uint8_t b, uint8_t a, SDL_BlendMode blend_mode) {
    SDL_SetRenderDrawColor(rend, r, g, b, a);
    SDL_SetRenderDrawBlendMode(rend, blend_mode);

    int32_t center_x = circle_x + radius;
    int32_t center_y = circle_y + radius;

    int32_t x = 0;
    int32_t y = radius;
    int32_t d = 3 - 2 * radius;

    SDL_RenderDrawLine(rend, center_x, center_y, center_x, center_y - radius);
    SDL_RenderDrawLine(rend, center_x, center_y, center_x, center_y + radius);
    SDL_RenderDrawLine(rend, center_x, center_y, center_x - radius, center_y);
    SDL_RenderDrawLine(rend, center_x, center_y, center_x + radius, center_y);
    
    while (y >= x) {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }

        // dont draw 0-length lines which cause artifacts
        if (-y + x < 0) {
            // top-right quadrant
            SDL_RenderDrawLine(rend, center_x + x,  center_y + -y, center_x + x, center_y + -x);
            SDL_RenderDrawLine(rend, center_x + y,  center_y + -x, center_x + x + 1, center_y + -x);

            // bottom-right quadrant
            SDL_RenderDrawLine(rend, center_x + x,  center_y + y, center_x + x, center_y + x);
            SDL_RenderDrawLine(rend, center_x + y,  center_y + x, center_x + x + 1, center_y + x);


            // bottom-left quadrant
            SDL_RenderDrawLine(rend, center_x + -x, center_y + y, center_x + -x, center_y + x);
            SDL_RenderDrawLine(rend, center_x + -y, center_y + x, center_x + -x - 1, center_y + x);;

            // top-left quadrant
            SDL_RenderDrawLine(rend, center_x + -x, center_y + -y, center_x + -x, center_y + -x);
            SDL_RenderDrawLine(rend, center_x + -y, center_y + -x, center_x + -x - 1, center_y + -x);
        }
    }
    
    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_NONE);
}
