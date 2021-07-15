#ifndef _10h0ch_tool_h
#define _10h0ch_tool_h

#include <SDL2/SDL_events.h>

typedef struct TOOL tool_t;

typedef uint8_t (*tool_keydown_handler)(SDL_KeyboardEvent* evt);

typedef uint8_t (*tool_mouse_motion_handler)(SDL_MouseMotionEvent* evt);

typedef uint8_t (*tool_mouse_click_handler)(SDL_MouseButtonEvent* evt);

typedef uint8_t (*tool_func_with_status)();

typedef void (*tool_func)();

typedef char* (*tool_status_bar_text_provider)();


/**
 * container for all the different tool "lifecycle" handlers.
 * make sure to initialize this with 0's so the NULL checks the
 * tool_... flavored functions perform actually make sense.
 */
struct TOOL {
   tool_keydown_handler keydown_handler; 
   tool_mouse_motion_handler mouse_motion_handler;
   tool_mouse_click_handler mouse_click_handler;
   tool_status_bar_text_provider status_bar_text_provider;
   tool_func ghost_renderer;
   tool_func_with_status activate_handler;
   tool_func_with_status deactivate_handler;

};

uint8_t tool_handle_activate(tool_t* tool);

uint8_t tool_handle_deactivate(tool_t* tool);

uint8_t tool_handle_keydown(tool_t* tool, SDL_KeyboardEvent* evt);

uint8_t tool_handle_mouse_motion(tool_t* tool, SDL_MouseMotionEvent* evt);

uint8_t tool_handle_mouse_click(tool_t* tool, SDL_MouseButtonEvent* evt);

char* tool_get_status_bar_text(tool_t* tool);

void tool_render_ghost(tool_t* tool);


#endif
