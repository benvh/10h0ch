#ifndef _10h0ch_tools_bullet_h
#define _10h0ch_tools_bullet_h

#include <SDL2/SDL_events.h>

void tools_mode_bullet_handle_keydown(SDL_KeyboardEvent* evt);

void tools_mode_bullet_handle_mouse_motion(SDL_MouseMotionEvent* evt);

void tools_mode_bullet_handle_mouse_click(SDL_MouseButtonEvent* evt);

#endif