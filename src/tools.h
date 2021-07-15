#ifndef _10h0ch_tools_h
#define _10h0ch_tools_h

#include <stdint.h>
#include <SDL2/SDL_events.h>

/**
 * a tool based "key down" event handler
 * the handler returns a positive number if it has handled the
 * event and the event should be ignored further down the line
 */
typedef uint8_t (*tools_keydown_handler)(SDL_KeyboardEvent* evt);

/**
 * a tool based "mouse motion" event handler
 * the handler returns a positive number if it has handled the
 * event and the event should be ignored further down the line
 */
typedef uint8_t (*tools_mouse_motion_handler)(SDL_MouseMotionEvent* evt);

/**
 * a tool based "mouse click" event handler
 * the handler returns a positive number if it has handled the
 * event and the event should be ignored further down the line
 */
typedef uint8_t (*tools_mouse_click_handler)(SDL_MouseButtonEvent* evt);

/**
 * a tool based status bar text provider. the returned char* must be owned by the
 * tool that generated it and the caller should *never* free it!
 */
typedef char* (*tools_status_bar_text_provider)();

/**
 * a tool based "ghost renderer" function which will try to render
 * a "ghost" of the active tool so that users can, for example, visually
 * assess this size of the pen/eraser
 */
typedef void (*tools_ghost_renderer)();


typedef enum {
    TOOL_MODE_NONE,
    TOOL_MODE_PENCIL,
    TOOL_MODE_ERASER,
    TOOL_MODE_TEXT,
    TOOL_MODE_BULLET,
    TOOL_MODE_ARROW,

    TOOL_MODE_MAX // 
} tools_tool_mode_t;


extern tools_tool_mode_t tools_current_mode;

extern const char* tools_tool_names[TOOL_MODE_MAX];

extern tools_keydown_handler      tools_mode_keydown_handler;

extern tools_mouse_motion_handler tools_mode_mouse_motion_handler;

extern tools_mouse_click_handler  tools_mode_mouse_click_handler;

extern tools_status_bar_text_provider tools_mode_status_bar_text_provider;

extern tools_ghost_renderer tools_mode_ghost_renderer;


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
