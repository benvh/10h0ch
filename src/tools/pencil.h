#ifndef _10h0ch_tools_pencil_h
#define _10h0ch_tools_pencil_h

#include <SDL2/SDL_events.h>


uint8_t tools_mode_pencil_handle_keydown(SDL_KeyboardEvent* evt);

uint8_t tools_mode_pencil_handle_mouse_motion(SDL_MouseMotionEvent* evt);

uint8_t tools_mode_pencil_handle_mouse_click(SDL_MouseButtonEvent* evt);

char* tools_mode_pencil_provide_status_bar_text();


#endif
