#include "tool.h"

uint8_t tool_handle_activate(tool_t* tool) {
    if (tool != NULL && tool->activate_handler != NULL) return tool->activate_handler();
    return 0;
}

uint8_t tool_handle_deactivate(tool_t* tool) {
    if (tool != NULL && tool->deactivate_handler != NULL) return tool->deactivate_handler();
    return 0;
}

uint8_t tool_handle_keydown(tool_t* tool, SDL_KeyboardEvent* evt)  {
    if (tool != NULL && tool->keydown_handler != NULL) return tool->keydown_handler(evt);
    return 0;
}

uint8_t tool_handle_mouse_motion(tool_t* tool, SDL_MouseMotionEvent* evt) {
    if (tool != NULL && tool->mouse_motion_handler != NULL) return tool->mouse_motion_handler(evt);
    return 0;
}

uint8_t tool_handle_mouse_click(tool_t* tool, SDL_MouseButtonEvent* evt) {
    if (tool != NULL && tool->mouse_click_handler != NULL) return tool->mouse_click_handler(evt);
    return 0;
}

char* tool_get_status_bar_text(tool_t* tool) {
    if (tool != NULL && tool->status_bar_text_provider != NULL) return tool->status_bar_text_provider();
    return NULL;
}

void tool_render_ghost(tool_t* tool) {
    if (tool != NULL && tool->ghost_renderer) tool->ghost_renderer();
}
