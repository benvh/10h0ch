#include "none.h"

static char* status_bar_text = "[normal]";

uint8_t tools_mode_none_handle_keydown(SDL_KeyboardEvent* evt) {
    // noop
    return 0;
}

uint8_t tools_mode_none_handle_mouse_motion(SDL_MouseMotionEvent* evt) {
    // noop
    return 0;
}

uint8_t tools_mode_none_handle_mouse_click(SDL_MouseButtonEvent* evt) {
    // noop
    return 0;
}

char* tools_mode_none_provide_status_bar_text() {
    return status_bar_text;
}
