#include "input_util.h"
#include <string.h>

void input_util_reset(input_util_t* iu, uint8_t max_length, input_util_key_filter_t key_filter) {
    iu->max_length = max_length;
    iu->key_filter = key_filter;
    input_util_wipe(iu);
}

void input_util_wipe(input_util_t* iu) {
    memset(iu->buff, '\0', INPUT_UTIL_BUFF_LENGTH);
    iu->buff_ptr = iu->buff;
}

uint8_t input_util_handle_keydown(input_util_t* iu, SDL_KeyboardEvent* evt) {
    if (iu->key_filter(evt->keysym.sym) > 0) {
        INPUT_UTIL_ADD(iu, evt->keysym.sym);
        return 1;

    } else if (evt->keysym.sym == SDLK_BACKSPACE) {
        INPUT_UTIL_DEL(iu);
        return 1;
    } 

    return 0;
}

uint8_t input_util_hex_key_filter(char c) {
    return (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || (c >= '0' && c <= '9');
}

uint8_t input_util_alphanumeric_key_filter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}
