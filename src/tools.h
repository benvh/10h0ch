#ifndef _10h0ch_tools_h
#define _10h0ch_tools_h

#include "tool.h"

#include <stdint.h>
#include <SDL2/SDL_events.h>

extern tool_t tools_tool_none;
extern tool_t tools_tool_pencil;
extern tool_t tools_tool_eraser;
extern tool_t tools_tool_marker;


/**
 * initialize the tools module
 */
void tools_init();


/**
 * deinitialize the tools module
 */
void tools_deinit();

/**
 * generates status bar text content for the current mode
 * the caller must free the returned char*
 */
char* tools_get_status_bar_text();

/**
 * renders the active tool's ghost to the window. keep in mind some tools
 * have no ghost and in  that case nothing will be rendered...
 */
void tools_render_tool_ghost();

/**
 * make the provided tool the active tool. NULL values are ignored.
 */
void tools_activate_tool(tool_t* tool);

/**
 * main tools keydown event handler
 */
uint8_t tools_handle_keydown(SDL_KeyboardEvent* evt);

/**
 * main tools mouse motion event handler
 */
uint8_t tools_handle_mouse_motion(SDL_MouseMotionEvent* evt);

/**
 * main tools mouse button event handler
 */
uint8_t tools_handle_mouse_click(SDL_MouseButtonEvent* evt);


#endif
