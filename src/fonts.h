#ifndef _10h0ch_fonts_h
#define _10h0ch_fonts_h

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

/**
 * initialize the fonts module (this will initialize SDL_ttf and fontconfig)
 */
void fonts_init();

/**
 * deinitialize the fonts module
 */
void fonts_deinit();

/**
 * load a font file using font-config to find the actual font.
 * the font must be cleaned up with TTF_CloseFont if it is no longer
 * needed.
 *
 * \param pattern a font-config pattern
 * \return a TTF_Font pointer or NULL if no matching font was found
 */
TTF_Font* fonts_load_font(const char* pattern, int pt_size);

/**
 * render the provided text to an SDL_Texture. The texture must be
 * cleaned up using SDL_DestroyTexture if it is no longer needed.
 *
 * \param text  the text to render
 * \param font  the font to use when rendering the text
 * \param color the text color
 * \return the rendered text as an SDL_Texture*
 */
SDL_Surface* fonts_render_text(const char* text, TTF_Font* font, SDL_Color color);

#endif
