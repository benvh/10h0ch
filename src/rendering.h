#ifndef _10h0ch_rendering_h
#define _10h0ch_rendering_h

#include <SDL2/SDL.h>

#define RENDERING_DEFAULT_WINDOW_WIDTH 1024
#define RENDERING_DEFAULT_WINDOW_HEIGHT 768
#define RENDERING_DEFAULT_WINDOW_TITLE "10h0ch"

extern SDL_Window* wnd;
extern SDL_Rect wnd_rect;
extern SDL_Renderer* rend;

void rendering_init();

void rendering_deinit();

void rendering_handle_window_resized();

void rendering_wipe_screen();

void rendering_swap_screen();

#endif
