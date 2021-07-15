#ifndef _10h0ch_tools_arrow_h
#define _10h0ch_tools_arrow_h

#include <SDL2/SDL_events.h>

void tools_mode_arrow_handle_keydown(SDL_KeyboardEvent* evt);

void tools_mode_arrow_handle_mouse_motion(SDL_MouseMotionEvent* evt);

void tools_mode_arrow_handle_mouse_click(SDL_MouseButtonEvent* evt);

#endif
