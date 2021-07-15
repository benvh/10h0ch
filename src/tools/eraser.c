#include "eraser.h"
#include "image.h"
#include "rendering.h"

static char status_bar_buff[32] = "\0";

static uint8_t eraser_size = 8;


uint8_t tools_mode_eraser_handle_keydown(SDL_KeyboardEvent* evt) {
    if (evt->keysym.sym == SDLK_q) return 1; // swallow 'q' keypresses which otherwise would exit 10h0ch

    switch(evt->keysym.sym) {
        case SDLK_LEFTBRACKET:
            // reduce pencil size
            eraser_size = eraser_size == 1 ? 1 : eraser_size - 1;
            return 1;

        case SDLK_RIGHTBRACKET:
            // increase pencil size
            eraser_size = eraser_size == 0xff ? 0xff : eraser_size + 1;
            return 1;
    }

    return 0;
}

uint8_t tools_mode_eraser_handle_mouse_motion(SDL_MouseMotionEvent* evt) {
    // TODO: render the "eraser ghost"

    // do nothing if the left mouse button is not down...
    if ((evt->state & SDL_BUTTON_LMASK) != SDL_BUTTON_LMASK) return 0;

    SDL_Rect img_rect = image_get_offset_rect();
    int32_t edit_x = evt->x - img_rect.x;
    int32_t edit_y = evt->y - img_rect.y;

    if (edit_x < 0 || edit_x > img_rect.w) return 0; // out of bounds, can't draw outside our original image...
    if (edit_y < 0 || edit_y > img_rect.h) return 0; // out of bounds, can't draw outside our original image...

    SDL_SetRenderTarget(rend, img_edit_texture);
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 0);

    SDL_Rect point_rect = { .x = edit_x-(eraser_size>1), .y = edit_y-(eraser_size>1), .w = eraser_size, .h = eraser_size };
    SDL_RenderFillRect(rend, &point_rect);

    SDL_SetRenderTarget(rend, NULL);

    return 1;
}

uint8_t tools_mode_eraser_handle_mouse_click(SDL_MouseButtonEvent* evt) {
    return 0;
}

char* tools_mode_eraser_provide_status_bar_text() {
    sprintf(status_bar_buff, "[eraser] | eraser size = %d", eraser_size);
    return status_bar_buff;
}
