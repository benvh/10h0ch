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
    SDL_SetRenderDrawColor(rend, 0x44, 0x44, 0x44, 0xff);
    SDL_RenderClear(rend);
}

void rendering_swap_screen() {
    SDL_RenderPresent(rend);
}
