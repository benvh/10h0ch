#include "tool.h"
#include "rendering.h"
#include "image.h"
#include "hex.h"
#include "fonts.h"
#include "input_util.h"

#define STR_ON "on"
#define STR_OFF "off"

#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a < b ? a : b)
#define NEXT_MARKER_NUMBER MIN(999, marker_number + 1)
#define PREV_MARKER_NUMBER MAX(1, marker_number - 1)

typedef enum {
    MARKER_MODE_PLACE,
    MARKER_MODE_INPUT_COLOR,
    MARKER_MODE_INPUT_ALPHA,
} marker_mode_t;


static marker_mode_t marker_mode = MARKER_MODE_PLACE;

static char marker_status_bar_buff[128] = "\0";

static uint8_t marker_auto_increment = ~0;

static uint32_t marker_color = 0x000000ff;

static uint32_t marker_number = 1;

static uint32_t marker_size = 12;

static input_util_t marker_input_util;



static uint8_t marker_handle_activate() {
    SDL_ShowCursor(SDL_DISABLE);
    return 1;
}

static uint8_t marker_handle_deactivate() {
    SDL_ShowCursor(SDL_ENABLE);
    return 1;
}

static uint8_t marker_handle_keydown(SDL_KeyboardEvent* evt) {
    if (marker_mode == MARKER_MODE_PLACE) {
        switch(evt->keysym.sym) {
            case SDLK_LEFTBRACKET:
                marker_size = MAX(1, marker_size-1);
                return 1;
            case SDLK_RIGHTBRACKET:
                marker_size = MIN(0xff, marker_size+1);
                return 1;
            case SDLK_SEMICOLON:
                marker_number = PREV_MARKER_NUMBER;
                return 1;
            case SDLK_QUOTE:
                marker_number = NEXT_MARKER_NUMBER;
                return 1;
            case SDLK_PERIOD:
                marker_number = 1;
                return 1;
            case SDLK_a:
                input_util_reset(&marker_input_util, 2, input_util_hex_key_filter);
                marker_mode = MARKER_MODE_INPUT_ALPHA;
                return 1;
            case SDLK_c:
                input_util_reset(&marker_input_util, 6, input_util_hex_key_filter);
                marker_mode = MARKER_MODE_INPUT_COLOR;
                return 1;
            case SDLK_i:
                marker_auto_increment = ~marker_auto_increment;
                return 1;
        }

    } else if (marker_mode == MARKER_MODE_INPUT_COLOR || marker_mode == MARKER_MODE_INPUT_ALPHA) {
        if (evt->keysym.sym == SDLK_ESCAPE) {
            // escape cancel color input
            marker_mode = MARKER_MODE_PLACE;

        }  else if (evt->keysym.sym == SDLK_RETURN || evt->keysym.sym == SDLK_RETURN2) { 
            // commit color change
            if (marker_mode == MARKER_MODE_INPUT_COLOR) marker_color = (hex_parse_hex_str(marker_input_util.buff)<<8) | (marker_color&0xff);
            else if (marker_mode == MARKER_MODE_INPUT_ALPHA) marker_color = (marker_color&(~0xff)) | hex_parse_hex_str(marker_input_util.buff);
            marker_mode = MARKER_MODE_PLACE;
        } else {
            input_util_handle_keydown(&marker_input_util, evt);
        }
        return 1;
    }

    return 0;
}

static uint8_t marker_handle_mouse_motion(SDL_MouseMotionEvent* evt) {
    return 0;
}

static uint8_t marker_handle_mouse_click(SDL_MouseButtonEvent* evt) {

    SDL_Rect img_offset_rect = image_get_offset_rect();

    if (marker_mode == MARKER_MODE_PLACE && evt->button == SDL_BUTTON_LEFT && evt->state == SDL_PRESSED) {
        // get our position in "image space"
        SDL_Rect marker_edit_rect = {0};
        marker_edit_rect.x = evt->x - img_offset_rect.x;
        marker_edit_rect.y = evt->y - img_offset_rect.y;

        // only place a marker if we're inside our image
        if (marker_edit_rect.x >= 0 && marker_edit_rect.x < img_offset_rect.w &&  marker_edit_rect.y >= 0 && marker_edit_rect.y < img_offset_rect.h) {
            SDL_SetRenderTarget(rend, img_edit_texture);
            
            // draw the marker circle
            SDL_Rect marker_circle_rect = { 
                .x = marker_edit_rect.x - marker_size,
                .y = marker_edit_rect.y - marker_size
            };
            rendering_fill_circle(
                    marker_circle_rect.x,
                    marker_circle_rect.y,
                    marker_size,
                    marker_color>>24, marker_color>>16, marker_color>>8, marker_color&0xff,
                    SDL_BLENDMODE_BLEND);

            // render the number string
            SDL_Color color_white = { .r = 0xff, .g = 0xff, .b = 0xff, .a = marker_color&0xff };
            char marker_number_text[4];
            sprintf(marker_number_text, "%d", marker_number);
            TTF_Font* marker_number_font = fonts_load_font("monospace", marker_size*0.85);
            SDL_Surface* marker_number_surface = fonts_render_text(marker_number_text, marker_number_font, color_white);
            SDL_Texture* marker_number_texture = SDL_CreateTextureFromSurface(rend, marker_number_surface);

            // now copy the number string texture we've rendered onto our img_edit_texture
            SDL_Rect marker_number_rect = {
                .x = marker_edit_rect.x - (marker_number_surface->w>>1),
                .y = marker_edit_rect.y - (marker_number_surface->h>>1),
                .w = marker_number_surface->w,
                .h = marker_number_surface->h,
            };
            SDL_RenderCopy(rend, marker_number_texture, NULL, &marker_number_rect);

            // clean up
            SDL_DestroyTexture(marker_number_texture);
            SDL_FreeSurface(marker_number_surface);
            TTF_CloseFont(marker_number_font);

            // reset render target
            SDL_SetRenderTarget(rend, NULL);

            // auto-increment our number if that's enabled...
            if (marker_auto_increment > 0) marker_number = NEXT_MARKER_NUMBER;
        }

        return 1;
    }

    return 0;
}

static void marker_render_ghost() {
    int32_t mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    SDL_SetRenderTarget(rend, NULL);

    // render the actual pencil ghost so you can see where you'll be placing a marker
    SDL_Rect point_rect = { .x = mouse_x-marker_size, .y = mouse_y-marker_size, .w = marker_size, .h = marker_size };
    rendering_draw_circle(point_rect.x, point_rect.y, marker_size, 0, 0, 0, 0xa0, SDL_BLENDMODE_BLEND);
}

static char* marker_provide_status_bar_text() {
    switch(marker_mode) {
        case MARKER_MODE_PLACE:
            sprintf(marker_status_bar_buff, "[marker] | color = #%06x | alpha = #%02x | number = %d | size = %d | auto-increment = %s", 
                    marker_color>>8, marker_color & 0xff,
                    marker_number,
                    marker_size,
                    (marker_auto_increment > 0 ? STR_ON : STR_OFF));
            break;
        case MARKER_MODE_INPUT_COLOR:
            sprintf(marker_status_bar_buff, "[marker] | input color: #%s_", marker_input_util.buff);
            break;
        case MARKER_MODE_INPUT_ALPHA:
            sprintf(marker_status_bar_buff, "[marker] | input alpha: #%s_", marker_input_util.buff);
            break;
    }

    return marker_status_bar_buff;
}




// -------------

tool_t tools_tool_marker = {
    .activate_handler = marker_handle_activate,
    .deactivate_handler = marker_handle_deactivate,
    .keydown_handler = marker_handle_keydown,
    .mouse_click_handler = marker_handle_mouse_click,
    .mouse_motion_handler = marker_handle_mouse_motion,
    .ghost_renderer = marker_render_ghost,
    .status_bar_text_provider = marker_provide_status_bar_text,
};
