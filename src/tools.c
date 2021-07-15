#include "tools.h"

static tool_t* current_tool;

void tools_init() {
    tools_activate_tool(&tools_tool_none);
}

void tools_deinit() {
}

char* tools_get_status_bar_text() {
    return tool_get_status_bar_text(current_tool);
}

void tools_render_tool_ghost() {
    tool_render_ghost(current_tool);
}

void tools_activate_tool(tool_t* tool) {
    if (current_tool == tool) return;
    tool_handle_deactivate(current_tool);
    current_tool = tool;
    tool_handle_activate(current_tool);
}

uint8_t tools_handle_keydown(SDL_KeyboardEvent* evt) {
    uint8_t tool_handler_result = tool_handle_keydown(current_tool, evt);
    if (tool_handler_result <= 0) {
        switch(evt->keysym.sym) {
            case SDLK_ESCAPE:
                tools_activate_tool(&tools_tool_none);
                return 1;

            case SDLK_p:
                tools_activate_tool(&tools_tool_pencil);
                return 1;

            case SDLK_e:
                tools_activate_tool(&tools_tool_eraser);
                return 1;
        }
    } else {
        return tool_handler_result;
    }

    return 0;
}

uint8_t tools_handle_mouse_motion(SDL_MouseMotionEvent* evt) {
    uint8_t too_handler_result = tool_handle_mouse_motion(current_tool, evt);
    if (too_handler_result <= 0) {

    } else {
        return too_handler_result;
    }

    return 0;
}

uint8_t tools_handle_mouse_click(SDL_MouseButtonEvent* evt) {
    uint8_t tool_handler_result = tool_handle_mouse_click(current_tool, evt);
    if (tool_handler_result <= 0) {

    } else {
        return tool_handler_result;
    }

    return 0;
}
