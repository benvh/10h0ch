#include "tool.h"
#include "image.h"
#include "rendering.h"

static char status_bar_buff[32] = "\0";

static uint8_t eraser_size = 8;

static SDL_Texture* eraser_edit_texture = NULL;

static void wipe_eraser_edit_texture() {
    SDL_SetRenderTarget(rend, eraser_edit_texture);
    SDL_RenderCopy(rend, img_edit_texture, NULL, NULL);
    SDL_SetRenderTarget(rend, NULL);
}

uint8_t tools_tool_eraser_handle_activate() {
    SDL_ShowCursor(SDL_DISABLE);

    // lazyily initialize our edit texture
    if (eraser_edit_texture == NULL) {
        eraser_edit_texture = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, img_orig_surface->w, img_orig_surface->h);
        SDL_SetTextureBlendMode(eraser_edit_texture, SDL_BLENDMODE_BLEND);
    }

    wipe_eraser_edit_texture();

    return 1;
}

uint8_t tools_tool_eraser_handle_deactivate() {
    SDL_ShowCursor(SDL_ENABLE);
    return 1;
}

uint8_t tools_tool_eraser_handle_keydown(SDL_KeyboardEvent* evt) {
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

uint8_t tools_tool_eraser_handle_mouse_motion(SDL_MouseMotionEvent* evt) {
    // do nothing if the left mouse button is not down...
    if ((evt->state & SDL_BUTTON_LMASK) != SDL_BUTTON_LMASK) return 0;

    SDL_Rect img_rect = image_get_offset_rect();
    int32_t edit_x = evt->x - img_rect.x;
    int32_t edit_y = evt->y - img_rect.y;

    if (edit_x < 0 || edit_x > img_rect.w) return 0; // out of bounds, can't draw outside our original image...
    if (edit_y < 0 || edit_y > img_rect.h) return 0; // out of bounds, can't draw outside our original image...

    SDL_Rect point_rect = { .x = edit_x-(eraser_size>>1), .y = edit_y-(eraser_size>>1), .w = eraser_size, .h = eraser_size };

    SDL_SetRenderTarget(rend, eraser_edit_texture);
    rendering_fill_circle(point_rect.x, point_rect.y, eraser_size, 0x00, 0x00, 0x00, 0x00, SDL_BLENDMODE_NONE);
    SDL_SetRenderTarget(rend, NULL);

    return 1;
}

uint8_t tools_tool_eraser_handle_mouse_click(SDL_MouseButtonEvent* evt) {

    if (evt->button == SDL_BUTTON_LEFT && evt->state == SDL_RELEASED) {
        // commit our erase stroke to img_edit
        SDL_SetRenderTarget(rend, img_edit_texture);
        SDL_RenderCopy(rend, img_orig_texture, NULL, NULL);
        SDL_RenderCopy(rend, eraser_edit_texture, NULL, NULL);

        // also wipe our edit texture so we can start erasing again
        wipe_eraser_edit_texture();

        // reset render which we changed...
        SDL_SetRenderTarget(rend, NULL);
    }

    return 0;
}

char* tools_tool_eraser_provide_status_bar_text() {
    sprintf(status_bar_buff, "[eraser] | eraser size = %d", eraser_size);
    return status_bar_buff;
}

void tools_tool_eraser_render_ghost() {
    int32_t mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    // make sure we're rendering to our window
    SDL_SetRenderTarget(rend, NULL);
    SDL_Rect img_offset_rect = image_get_offset_rect();

    // render the original image and on top of that our "eraser_edit_texture"
    // which contains edit_img with "holes" wherever stuff got erased causing
    // the original image we rendered beneath it to show up
    SDL_RenderCopy(rend, img_orig_texture, NULL, &img_offset_rect);
    SDL_RenderCopy(rend, eraser_edit_texture, NULL, &img_offset_rect);

    // drawer the actual eraser tool ghost so you can see what you're doing
    SDL_Rect point_rect = { .x = mouse_x-(eraser_size>>1), .y = mouse_y-(eraser_size>>1), .w = eraser_size, .h = eraser_size };
    rendering_draw_circle(point_rect.x, point_rect.y, eraser_size, 0, 0, 0, 0xa0, SDL_BLENDMODE_BLEND);
}


// --------

tool_t tools_tool_eraser = {
    .activate_handler = tools_tool_eraser_handle_activate,
    .deactivate_handler = tools_tool_eraser_handle_deactivate,
    .keydown_handler = tools_tool_eraser_handle_keydown,
    .mouse_motion_handler = tools_tool_eraser_handle_mouse_motion,
    .mouse_click_handler = tools_tool_eraser_handle_mouse_click,
    .status_bar_text_provider = tools_tool_eraser_provide_status_bar_text,
    .ghost_renderer = tools_tool_eraser_render_ghost,
};
