#include "pencil.h"
#include "rendering.h"
#include "image.h"
#include "hex.h"

static char status_bar_buff[128] = "\0";

static uint8_t pencil_size = 8;

static uint32_t pencil_color = 0xffffffff;


// text input related stuff specifically used for changing pencil
// color as  far as the pencil tool goes...

typedef enum {
    INPUT_MODE_NONE,
    INPUT_MODE_INPUT_COLOR
} input_mode_t;

static input_mode_t input_mode = 0;

static char input_buff[16] = "\0";

static char* input_buff_ptr = input_buff;

static void clear_input_buffer() {
    memset(input_buff, '\0', 16);
    input_buff_ptr = input_buff;
}



uint8_t tools_mode_pencil_handle_keydown(SDL_KeyboardEvent* evt) {
    if (evt->keysym.sym == SDLK_q) return 1; // swallow 'q' keypresses which otherwise would exit 10h0ch

    // if we're in "color input mode" we'll basically swallow all keypresses as well
    if (input_mode == INPUT_MODE_INPUT_COLOR) {
        if (evt->keysym.sym == SDLK_ESCAPE) {
            // escape cancel color input
            input_mode = INPUT_MODE_NONE;

        } else if ((evt->keysym.sym >= '0' && evt->keysym.sym <= '9') || (evt->keysym.sym >= 'a' && evt->keysym.sym <= 'f') || (evt->keysym.sym >= 'A' && evt->keysym.sym <= 'F')) {
            // valid hex chars will be appended to our input buffer
            if (input_buff_ptr != (input_buff + 8)) *(input_buff_ptr++) = evt->keysym.sym;

        } else if (evt->keysym.sym == SDLK_BACKSPACE) {
            // backspace removes chars from our input buffer...
            if (input_buff_ptr != input_buff) *(--input_buff_ptr) = '\0';

        } else if (evt->keysym.sym == SDLK_RETURN || evt->keysym.sym == SDLK_RETURN2) {
            // commit color change

            // we'll check the length of the color that was entered
            // if it's only 3 bytes we'll assume those are our RGB colors
            // and we'll set our A(lpha) channel to 0xFF. If the entered
            // color is 4 bytes long the full RGBA value was entered which
            // we'll use as-is

            // NOTE: what about "shorter" color values? e.g. only a single
            // byte was provided
            pencil_color = hex_parse_hex_str(input_buff);
            if (strlen(input_buff) == 6) {
                pencil_color = (pencil_color << 8) | 0xff;
            }
            input_mode = INPUT_MODE_NONE;
        }

        return 1;
    }

    switch(evt->keysym.sym) {
        case SDLK_LEFTBRACKET:
            // reduce pencil size
            pencil_size = pencil_size == 1 ? 1 : pencil_size - 1;
            return 1;

        case SDLK_RIGHTBRACKET:
            // increase pencil size
            pencil_size = pencil_size == 0xff ? 0xff : pencil_size + 1;
            return 1;

        case SDLK_c:
            // change pencil color
            clear_input_buffer();
            input_mode = INPUT_MODE_INPUT_COLOR;
            return 1;
    }

    return 0;
}

uint8_t tools_mode_pencil_handle_mouse_motion(SDL_MouseMotionEvent* evt) {
    // TODO: render the "pencil ghost"

    // do nothing if the left mouse button is not down...
    if ((evt->state & SDL_BUTTON_LMASK) != SDL_BUTTON_LMASK) return 0;

    SDL_Rect img_rect = image_get_offset_rect();
    int32_t edit_x = evt->x - img_rect.x;
    int32_t edit_y = evt->y - img_rect.y;

    if (edit_x < 0 || edit_x > img_rect.w) return 0; // out of bounds, can't draw outside our original image...
    if (edit_y < 0 || edit_y > img_rect.h) return 0; // out of bounds, can't draw outside our original image...

    SDL_SetRenderTarget(rend, img_edit_texture);
    SDL_SetRenderDrawColor(rend, pencil_color >> 24, pencil_color >> 16, pencil_color >> 8, pencil_color & 0xff);

    SDL_Rect point_rect = { .x = edit_x-(pencil_size>>1), .y = edit_y-(pencil_size>>1), .w = pencil_size, .h = pencil_size };
    SDL_RenderFillRect(rend, &point_rect);

    SDL_SetRenderTarget(rend, NULL);

    return 1;
}

uint8_t tools_mode_pencil_handle_mouse_click(SDL_MouseButtonEvent* evt) {


    return 0;
}

char* tools_mode_pencil_provide_status_bar_text() {
    if (input_mode == INPUT_MODE_NONE) {
        sprintf(status_bar_buff, "[pencil] | fg = #%06x, alpha = 0x%02x | pencil size = %d", pencil_color>>8, pencil_color & 0xff, pencil_size);
    } else if (input_mode == INPUT_MODE_INPUT_COLOR) {
        sprintf(status_bar_buff, "[pencil] | set color: #%s_", input_buff);
    }

    return status_bar_buff;
}

void tools_mode_pencil_render_ghost() {
    int32_t mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    SDL_Rect point_rect = { .x = mouse_x-(pencil_size>>1), .y = mouse_y-(pencil_size>>1), .w = pencil_size, .h = pencil_size };
    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 0xa0);
    SDL_RenderDrawRect(rend, &point_rect);
    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_NONE);
}
