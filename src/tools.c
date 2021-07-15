#include "tools.h"
#include "tools/all.h"

tools_tool_mode_t   tools_current_mode;
const char* tools_tool_name[TOOL_MODE_MAX] = {
    "none",
    "pencil",
    "eraser",
    "text",
    "bullet",
    "arrow",
};


tools_keydown_handler           tools_mode_keydown_handler;
tools_mouse_motion_handler      tools_mode_mouse_motion_handler;
tools_mouse_click_handler       tools_mode_mouse_click_handler;
tools_status_bar_text_provider  tools_mode_status_bar_text_provider;
tools_ghost_renderer            tools_mode_ghost_renderer;

void tools_ghost_noop_renderer() {
    // noop renderer for tools that dont require a ghost (e.g. the 'none' tool)
}

void tools_init() {
    tools_current_mode = TOOL_MODE_NONE;
    tools_mode_keydown_handler = tools_mode_none_handle_keydown;
    tools_mode_mouse_motion_handler = tools_mode_none_handle_mouse_motion;
    tools_mode_mouse_click_handler = tools_mode_none_handle_mouse_click;
    tools_mode_status_bar_text_provider = tools_mode_none_provide_status_bar_text;
    tools_mode_ghost_renderer = tools_ghost_noop_renderer;
}

void tools_deinit() {
}

char* tools_get_status_bar_text() {
    return tools_mode_status_bar_text_provider();
}

void tools_render_tool_ghost() {
    tools_mode_ghost_renderer();
}

uint8_t tools_handle_keydown(SDL_KeyboardEvent* evt) {
    uint8_t mode_handler_result = tools_mode_keydown_handler(evt);
    if (mode_handler_result <= 0) {
        switch(evt->keysym.sym) {
            case SDLK_ESCAPE:
                SDL_ShowCursor(SDL_ENABLE);
                tools_current_mode = TOOL_MODE_NONE;
                tools_mode_keydown_handler = tools_mode_none_handle_keydown;
                tools_mode_mouse_motion_handler = tools_mode_none_handle_mouse_motion;
                tools_mode_mouse_click_handler = tools_mode_none_handle_mouse_click;
                tools_mode_status_bar_text_provider = tools_mode_none_provide_status_bar_text;
                tools_mode_ghost_renderer = tools_ghost_noop_renderer;
                return 1;

            case SDLK_p:
                SDL_ShowCursor(SDL_DISABLE);
                tools_current_mode = TOOL_MODE_PENCIL;
                tools_mode_keydown_handler = tools_mode_pencil_handle_keydown;
                tools_mode_mouse_motion_handler = tools_mode_pencil_handle_mouse_motion;
                tools_mode_mouse_click_handler = tools_mode_pencil_handle_mouse_click;
                tools_mode_status_bar_text_provider = tools_mode_pencil_provide_status_bar_text;
                tools_mode_ghost_renderer = tools_mode_pencil_render_ghost;
                return 1;

            case SDLK_e:
                SDL_ShowCursor(SDL_DISABLE);
                tools_current_mode = TOOL_MODE_ERASER;
                tools_mode_keydown_handler = tools_mode_eraser_handle_keydown;
                tools_mode_mouse_motion_handler = tools_mode_eraser_handle_mouse_motion;
                tools_mode_mouse_click_handler = tools_mode_eraser_handle_mouse_click;
                tools_mode_status_bar_text_provider = tools_mode_eraser_provide_status_bar_text;
                tools_mode_ghost_renderer = tools_mode_eraser_render_ghost;
                return 1;
        }
    } else {
        return mode_handler_result;
    }

    return 0;
}

uint8_t tools_handle_mouse_motion(SDL_MouseMotionEvent* evt) {
    uint8_t mode_handler_result = tools_mode_mouse_motion_handler(evt);
    if (mode_handler_result <= 0) {

    } else {
        return mode_handler_result;
    }

    return 0;
}

uint8_t tools_handle_mouse_click(SDL_MouseButtonEvent* evt) {
    uint8_t mode_handler_result = tools_mode_mouse_click_handler(evt);
    if (mode_handler_result <= 0) {

    } else {
        return mode_handler_result;
    }

    return 0;
}
