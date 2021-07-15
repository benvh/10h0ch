#include "rendering.h"
#include "status_bar.h"
#include "tools.h"
#include <SDL2/SDL_ttf.h>

static TTF_Font* status_bar_font = NULL;
static SDL_Color status_bar_color = { .r = 0xff, .g = 0x0ff, .b = 0xff, .a = 0xff };

void status_bar_init() {
    status_bar_font = fonts_load_font("monospace");
}

void status_bar_deinit() {
    TTF_CloseFont(status_bar_font);
    status_bar_font = NULL;
}

void status_bar_render_bar() {
    // render our status bar background
    SDL_Rect status_bar_rect = {
        .x = 0,
        .y = wnd_rect.h - STATUS_BAR_HEIGHT,
        .w = wnd_rect.w,
        .h = STATUS_BAR_HEIGHT,
    };
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 0xff);
    SDL_RenderFillRect(rend, &status_bar_rect);

    // generate status bar text and if there is any render it on the status bar...
    char* status_bar_tool_text = tools_get_status_bar_text();
    if (status_bar_tool_text != NULL) {
        SDL_Surface* status_bar_text_surface = fonts_render_text(status_bar_tool_text, status_bar_font, status_bar_color);
        SDL_Texture* status_bar_text_texture = SDL_CreateTextureFromSurface(rend, status_bar_text_surface);

        SDL_Rect status_bar_text_rect = { 
            .x = status_bar_rect.x + 5,
            .y = status_bar_rect.y + (status_bar_rect.h>>1) - (status_bar_text_surface->clip_rect.h>>1),
            .w = status_bar_text_surface->clip_rect.w,
            .h = status_bar_text_surface->clip_rect.h,
        };
        SDL_RenderCopy(rend, status_bar_text_texture, &(status_bar_text_surface->clip_rect), &status_bar_text_rect);

        SDL_DestroyTexture(status_bar_text_texture);
        SDL_FreeSurface(status_bar_text_surface);
    }
}
