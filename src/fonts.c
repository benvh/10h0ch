#include "fonts.h"

#include <fontconfig/fontconfig.h>

static FcConfig* fc = NULL;

/**
 * try to find a font file that matches the provided font-config pattern
 *
 * \param pattern a font-config pattern to find a font...
 */
char* find_font_file(const char* pattern);



void fonts_init() {
    if (TTF_Init() != 0) {
        fprintf(stderr, "could not initialize SDL ttf: %s\n", TTF_GetError());
        exit(-1);
    }

    FcInit();
    fc = FcInitLoadConfigAndFonts();
}

void fonts_deinit() {
    FcConfigDestroy(fc);
    FcFini();
    TTF_Quit();
}


TTF_Font* fonts_load_font(const char* pattern) {
    char* font_path = find_font_file(pattern);
    TTF_Font* font = TTF_OpenFont(font_path, 11);
    if (font == NULL) {
        fprintf(stderr, "could not load font '%s': %s\n", pattern, TTF_GetError());
        exit(-1);
    }
    free(font_path);
    return font;
}

SDL_Surface* fonts_render_text(const char* text, TTF_Font* font, SDL_Color color) {
    SDL_Surface* text_surface = TTF_RenderText_Blended(font, text, color);
    if (text_surface == NULL) {
        fprintf(stderr, "could not render text: %s\n", TTF_GetError());
    }
    return text_surface;
}

/**
 * try to find a font file that matches the provided font-config pattern
 *
 * \param pattern a font-config pattern to find a font...
 */
char* find_font_file(const char* pattern) {
    FcPattern* fc_pat = FcNameParse((const FcChar8*)pattern);
    FcConfigSubstitute(fc, fc_pat, FcMatchPattern);
    FcDefaultSubstitute(fc_pat);

    FcResult font_res;
    FcPattern* fc_font = FcFontMatch(fc, fc_pat, &font_res);
    if (fc_font) {
        FcChar8* fc_font_path;
        FcPatternGetString(fc_font, FC_FILE, 0, &fc_font_path);

        // copy fc_font_path as it is part of the fc_font struct which we'll free in a moment...
        uint64_t fc_font_path_len = strlen((const char*)fc_font_path);
        char* font_path = (char*)malloc(sizeof(char) * fc_font_path_len + 1);
        strcpy(font_path, (const char*)fc_font_path);

        FcPatternDestroy(fc_font);
        FcPatternDestroy(fc_pat);

        return font_path;
    }

    return NULL;
}
