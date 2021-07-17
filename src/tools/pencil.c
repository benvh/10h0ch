#include "tool.h"
#include "rendering.h"
#include "image.h"
#include "hex.h"

static char status_bar_buff[128] = "\0";

static uint8_t pencil_size = 8;

static uint32_t pencil_color = 0xffffffff;

static SDL_Texture* pencil_edit_texture = NULL;


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

// --

static void wipe_pencil_edit_texture() {
    SDL_SetRenderTarget(rend, pencil_edit_texture);
    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 0x00);
    SDL_RenderClear(rend);
    SDL_SetRenderTarget(rend, NULL);
}

static uint8_t tools_tool_pencil_handle_activate() {
    SDL_ShowCursor(SDL_DISABLE);

    // lazy initialize internal pencil_edit_texture
    if (pencil_edit_texture == NULL) {
        pencil_edit_texture = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, img_orig_surface->w, img_orig_surface->h);
        SDL_SetTextureBlendMode(pencil_edit_texture, SDL_BLENDMODE_BLEND);
    }

    wipe_pencil_edit_texture();

    return 1;
}

static uint8_t tools_tool_pencil_handle_deactivate() {
    SDL_ShowCursor(SDL_ENABLE);
    return 1;
}

static uint8_t tools_tool_pencil_handle_keydown(SDL_KeyboardEvent* evt) {
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

static uint8_t tools_tool_pencil_handle_mouse_motion(SDL_MouseMotionEvent* evt) {
    // TODO: render the "pencil ghost"

    // do nothing if the left mouse button is not down...
    if ((evt->state & SDL_BUTTON_LMASK) != SDL_BUTTON_LMASK) return 0;

    SDL_Rect img_rect = image_get_offset_rect();
    int32_t edit_x = evt->x - img_rect.x;
    int32_t edit_y = evt->y - img_rect.y;

    if (edit_x < 0 || edit_x > img_rect.w) return 0; // out of bounds, can't draw outside our original image...
    if (edit_y < 0 || edit_y > img_rect.h) return 0; // out of bounds, can't draw outside our original image...

    // the "point" we're about to draw
    SDL_Rect point_rect = { .x = edit_x-(pencil_size>>1), .y = edit_y-(pencil_size>>1), .w = pencil_size, .h = pencil_size };

    // rendering our stroke to pencil_edit_texture without any blending
    SDL_SetRenderTarget(rend, pencil_edit_texture);
    rendering_fill_circle(point_rect.x, point_rect.y, pencil_size, pencil_color>>24, pencil_color>>16, pencil_color>>8, pencil_color&0xff, SDL_BLENDMODE_NONE);

    // reset render target...
    SDL_SetRenderTarget(rend, NULL);

    return 1;
}

static uint8_t tools_tool_pencil_handle_mouse_click(SDL_MouseButtonEvent* evt) {
    if (evt->button == SDL_BUTTON_LEFT && evt->state == SDL_PRESSED) {

    } else if (evt->button == SDL_BUTTON_LEFT && evt->state == SDL_RELEASED) {
        // done drawing, copy our edit texture onto img_edit_texture
        SDL_SetRenderTarget(rend, img_edit_texture);
        SDL_RenderCopy(rend, pencil_edit_texture, NULL, NULL);
        SDL_SetRenderTarget(rend, NULL);
        
        // wipe our edit texture so our ghost doesn't keep rendering our previous stroke
        wipe_pencil_edit_texture();

        // be nice and reset the render target we changed
        SDL_SetRenderTarget(rend, NULL);
    }

    return 0;
}

static char* tools_tool_pencil_provide_status_bar_text() {
    if (input_mode == INPUT_MODE_NONE) {
        sprintf(status_bar_buff, "[pencil] | color = #%06x, alpha = 0x%02x | pencil size = %d", pencil_color>>8, pencil_color & 0xff, pencil_size);
    } else if (input_mode == INPUT_MODE_INPUT_COLOR) {
        sprintf(status_bar_buff, "[pencil] | set color: #%s_", input_buff);
    }

    return status_bar_buff;
}

static void tools_tool_pencil_render_ghost() {
    int32_t mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    // make sure we're rending to our window...
    SDL_SetRenderTarget(rend, NULL);

    // "temporarily" render the stroke we're drawing
    SDL_Rect img_offset_rect = image_get_offset_rect();
    SDL_RenderCopy(rend, pencil_edit_texture, NULL, &img_offset_rect);

    // render the actual pencil ghost so you can see where you'll be drawing
    SDL_Rect point_rect = { .x = mouse_x-(pencil_size>>1), .y = mouse_y-(pencil_size>>1), .w = pencil_size, .h = pencil_size };
    rendering_draw_circle(point_rect.x, point_rect.y, pencil_size, 0, 0, 0, 0xa0, SDL_BLENDMODE_BLEND);
}


// --------

tool_t tools_tool_pencil = {
    .activate_handler = tools_tool_pencil_handle_activate,
    .deactivate_handler = tools_tool_pencil_handle_deactivate,
    .keydown_handler = tools_tool_pencil_handle_keydown,
    .mouse_motion_handler = tools_tool_pencil_handle_mouse_motion,
    .mouse_click_handler = tools_tool_pencil_handle_mouse_click,
    .status_bar_text_provider = tools_tool_pencil_provide_status_bar_text,
    .ghost_renderer = tools_tool_pencil_render_ghost
};

