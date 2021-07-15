#ifndef _10h0ch_status_bar_h
#define _10h0ch_status_bar_h

#include "fonts.h"
#include <SDL2/SDL_render.h>

#define STATUS_BAR_HEIGHT 23

/**
 * initialize the status bar module
 */
void status_bar_init();

/**
 * deinitialize the status bar module
 */
void status_bar_deinit();

/**
 */
void status_bar_render_bar();


#endif
