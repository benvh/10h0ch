#ifndef _10h0ch_input_util_h
#define _10h0ch_input_util_h

#include <stdint.h>
#include <SDL2/SDL_events.h>

#define INPUT_UTIL_BUFF_LENGTH 128
#define INPUT_UTIL_ADD(iu, c) if (iu->buff_ptr != &iu->buff[iu->max_length]) *(iu->buff_ptr++) = c
#define INPUT_UTIL_DEL(iu) if (iu->buff_ptr != iu->buff) *(--iu->buff_ptr) = '\0'

typedef uint8_t (*input_util_key_filter_t)(char c);

typedef struct INPUT_UTIL {
    char buff[INPUT_UTIL_BUFF_LENGTH];
    char* buff_ptr;
    uint8_t max_length;
    input_util_key_filter_t key_filter;

} input_util_t;


void input_util_reset(input_util_t* iu, uint8_t max_length, input_util_key_filter_t key_filter);

void input_util_wipe(input_util_t* iu);

uint8_t input_util_handle_keydown(input_util_t* iu, SDL_KeyboardEvent* evt);


uint8_t input_util_hex_key_filter(char c);

uint8_t input_util_alphanumeric_key_filter(char c);


#endif
