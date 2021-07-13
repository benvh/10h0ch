#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <fontconfig/fontconfig.h>

#define DEFAULT_WINDOW_WIDTH 1280
#define DEFAULT_WINDOW_HEIGTH 720
#define STATUS_BAR_HEIGHT 25

#define EXIT_INIT_FAILED 1
#define EXIT_READ_IMAGE_FAILED 2
// TODO: define more exit codes... or drop them altogether?

#define INPUT_BUFFER_SIZE 512

// --

typedef enum {
    TOOL_MODE_NORMAL,
    TOOL_MODE_PENCIL,
    TOOL_MODE_TEXT_PLACE,
    TOOL_MODE_TEXT_EDIT,
    TOOL_MODE_ERASER,
    TOOL_MODE_INPUT_COLOR,
} tool_mode_t;

static const char* tool_names[] = {
    "normal",
    "pencil",
    "text",
    "text-edit",
    "eraser",
    "color-edit"
};


static uint8_t should_be_running = 0;
static tool_mode_t tool_mode = TOOL_MODE_NORMAL;
static uint8_t pencil_width = 1;
static uint32_t pencil_color = 0xffffffff;

static char input_buffer[INPUT_BUFFER_SIZE];
static char* input_buffer_offset = input_buffer;

// some misc internal SDL values
static SDL_Rect wnd_rect = { .x = 0, .y = 0, .w = DEFAULT_WINDOW_WIDTH, .h = DEFAULT_WINDOW_HEIGTH };
static SDL_Rect img_rect = { .x = 0, .y = 0, .w = 0, .h = 0 };
static SDL_Window* wnd = NULL;
static SDL_Renderer* rend = NULL;

static SDL_Surface* img_orig_surface = NULL;
static SDL_Surface* img_edit_surface = NULL;
static SDL_Texture* img_orig_texture = NULL;
static SDL_Texture* img_edit_texture = NULL;
static TTF_Font* status_bar_font = NULL;

// we'll be using font-config to find fonts for sdl_ttf
FcConfig* font_config = NULL;

// --

void print_usage() {
    printf("usage: 10h0ch\n");
    printf("read an image from stdin, annotate it, write it to stdout as a .png\n");
}

/**
 * initialize SDL, SDL_image and SDL_ttf
 */
void init_sdl() {
    // init core sdl lib (we only care about video and events)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "could not initialize SDL:%s\n", SDL_GetError());
        exit(EXIT_INIT_FAILED);
    }

    // create the sdl window
    uint32_t wnd_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN; 
    wnd = SDL_CreateWindow("10h0ch", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, wnd_rect.w, wnd_rect.h, wnd_flags);
    if (wnd == NULL) {
        fprintf(stderr, "could not create SDL window: %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_INIT_FAILED);
    }

    // create our sdl renderer
    rend = SDL_CreateRenderer(wnd, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    if (rend == NULL) {
        fprintf(stderr, "could not create SDL renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(wnd);
        SDL_Quit();
        exit(EXIT_INIT_FAILED);
    }

    // initialize SDL_image
    uint32_t img_flags = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF;
    if ((IMG_Init(img_flags) & img_flags) != img_flags) {
        fprintf(stderr, "couldnot initialize SDL image: %s\n", IMG_GetError());
        SDL_DestroyWindow(wnd);
        SDL_Quit();
        exit(EXIT_INIT_FAILED);
    }

    // initialize SDL_ttf
    if (TTF_Init() != 0) {
        fprintf(stderr, "could not initialize SDL ttf: %s\n", TTF_GetError());
        SDL_DestroyWindow(wnd);
        SDL_Quit();
        exit(EXIT_INIT_FAILED);
    }
}

/**
 * initializes font-config 
 */
void init_font_config() {
    FcInit();
    font_config = FcInitLoadConfigAndFonts();
}

/**
 * try to find a font file that matches the provided font-config pattern
 *
 * \param pattern a font-config pattern to find a font...
 */
char* find_font_file(const char* pattern) {
    FcPattern* fc_pat = FcNameParse((const FcChar8*)pattern);
    FcConfigSubstitute(font_config, fc_pat, FcMatchPattern);
    FcDefaultSubstitute(fc_pat);

    FcResult font_res;
    FcPattern* fc_font = FcFontMatch(font_config, fc_pat, &font_res);
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

/**
 * init the things needed to render the interface. this includes but is not limited to the
 * font used for the status bar (uses \ref find_font_file to find the "monospace" font)
 */
void init_interface() {
    char* status_bar_font_path = find_font_file("monospace");
    status_bar_font = TTF_OpenFont(status_bar_font_path, 11);
    if (status_bar_font == NULL) {
        fprintf(stderr, "could not load 'monospace' font for status bar: %s\n", TTF_GetError());
        exit(3);
    }
    free(status_bar_font_path);
}

/**
 * read and try to load the image received by stdin
 */
SDL_Surface* read_image_from_stdin() {
    void* tmp_buff = NULL;
    int img_buff_offset = 0;
    size_t img_buff_size = 0x1000; // let's start with a 4Kb buffer. we'll resize it when needed...
    uint8_t* img_buff = (uint8_t*) malloc(img_buff_size * sizeof(uint8_t));

    // TODO: use ioctl to actually check there's stuff to read from stdin and if there isn't any
    //       data make this fail with an appropriate stderr message...

    int c;
    while ((c = getchar()) != EOF) {
        if (img_buff_offset == img_buff_size) {
            // we'll increase img_buff by another 4Kb
            img_buff_size += 0x1000;
            if ((tmp_buff = realloc(img_buff, img_buff_size * sizeof(uint8_t))) != NULL) {
                // resize successfull
                img_buff = (uint8_t*)tmp_buff;
            } else {
                // realloc failed... crash painfully :'(
                fprintf(stderr, "could not resize input image buffer size while reading [exiting]\n");
                free(img_buff);
                exit(EXIT_READ_IMAGE_FAILED); // TODO: instead of exiting over here return NULL or something?
            }
        }
        // read a byte from stdin
        img_buff[img_buff_offset++] = c;
    }

    // we read 0 bytes and hit an EOF...
    if (img_buff_offset == 0) {
        fprintf(stderr, "please provide a valid jpg, png or tif image via stdin\n");
        exit(EXIT_READ_IMAGE_FAILED);
    }

    // img_buff might be too big. realloc one last time so it's just big enough to hold the image data
    if (img_buff != NULL && img_buff_offset < img_buff_size) {
        img_buff = realloc(img_buff, img_buff_offset);
        // TODO probably check if this last realloc was a successs or not...
    }

    // now that we have img_buff we'll make SDL_image try to load that into an SDL_Surface
    // first creating an SDL_RWops which SDL_image can read
    SDL_RWops* sdl_img_buff = SDL_RWFromMem(img_buff, img_buff_offset);
    if (sdl_img_buff == NULL) {
        fprintf(stderr, "could not create SDL_RWops buffer for image: %s\n", SDL_GetError());
        free(img_buff);
        exit(EXIT_READ_IMAGE_FAILED); // TODO: instead of exiting over here return NULL or something?
    }

    // and then actually read the image data...
    SDL_Surface* img_surface = IMG_Load_RW(sdl_img_buff, 0);
    if (img_surface == NULL) {
        fprintf(stderr, "SDL_image could not load the image: %s\n", IMG_GetError());
        SDL_RWclose(sdl_img_buff);
        free(img_buff);
        exit(EXIT_READ_IMAGE_FAILED); // TODO: instead of exiting over here return NULL or something?
    }

    // clean up after ourselves
    SDL_RWclose(sdl_img_buff);
    free(img_buff);

    return img_surface;
}

void render_status_bar() {

    // TODO: probably move this "status_bar_text" thing in its own function...

    // build status bar text buffer
    char status_bar_text[256];
    const char* tool_mode_name = tool_names[tool_mode];
    sprintf(status_bar_text, "[%s]", tool_mode_name);
    //sprintf(status_bar_text, "[%s] | fg=0x%08x", tool_mode_name, 0x00);

    if (tool_mode == TOOL_MODE_PENCIL || tool_mode == TOOL_MODE_ERASER) {
        // when we're using the eraser/pencil we probably are interested in color/pen size
        sprintf((status_bar_text + strlen(status_bar_text)), " | fg=0x%08x", 0x00);
        sprintf((status_bar_text + strlen(status_bar_text)), " | size=%d", pencil_width);
    } else if (tool_mode == TOOL_MODE_TEXT_EDIT || tool_mode == TOOL_MODE_INPUT_COLOR) {
        // when we're in one of the text input modes we render the input prompt
        sprintf((status_bar_text + strlen(status_bar_text)), " | input: %s_", input_buffer);
    }

    // --

    // render the status bar background rect
    SDL_Rect status_bar_rect = { .x = 0, .y = wnd_rect.h - STATUS_BAR_HEIGHT, .w = wnd_rect.w, .h = STATUS_BAR_HEIGHT };
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 0xff);
    SDL_RenderFillRect(rend, &status_bar_rect);

    // render the status bar

    SDL_Color text_fg = { .r = 0xff, .g = 0xff, .b = 0xff, .a = 0xff };
    SDL_Surface* status_bar_text_surface = TTF_RenderText_Solid(status_bar_font, status_bar_text, text_fg); 
    SDL_Texture* status_bar_text_texture = SDL_CreateTextureFromSurface(rend, status_bar_text_surface);

    SDL_Rect status_bar_text_dest_rect = {
        .x = status_bar_rect.x + 5,
        .y = status_bar_rect.y + (status_bar_rect.h>>1) - (status_bar_text_surface->h>>1),
        .w = status_bar_text_surface->w,
        .h = status_bar_text_surface->h
    };
    SDL_RenderCopy(rend, status_bar_text_texture, &status_bar_text_surface->clip_rect, &status_bar_text_dest_rect);

    // clean up...
    SDL_DestroyTexture(status_bar_text_texture);
    SDL_FreeSurface(status_bar_text_surface);
}

/**
 * render our actual screen
 */
void render_screen() {
    // wipe our screen...
    SDL_SetRenderDrawColor(rend, 0xc7, 0xc7, 0xc7, 0xff);
    SDL_RenderClear(rend);
    SDL_RenderCopy(rend, img_orig_texture, &img_orig_surface->clip_rect, &img_rect);
    SDL_RenderCopy(rend, img_edit_texture, &img_orig_surface->clip_rect, &img_rect);

    render_status_bar();

    SDL_RenderPresent(rend);
}

/**
 * wipe the current contents of the input buffer
 */
void wipe_input_buffer() {
    memset(input_buffer, '\0', INPUT_BUFFER_SIZE);
    input_buffer_offset = input_buffer;
}



/**
 * SIGINT handler...
 */
void handle_sigint() {
    should_be_running = 0;
}

/**
 */
void handle_window_resized() {
    // update our wnd size values
    SDL_GetWindowSize(wnd, &wnd_rect.w, &wnd_rect.h);

    // update the img location and size values
    img_rect.x = (wnd_rect.w>>1) - (img_rect.w>>1);
    img_rect.y = (wnd_rect.h>>1) - (img_rect.h>>1);
    img_rect.w = img_orig_surface->w;
    img_rect.h = img_orig_surface->h;
}

/**
 */
void handle_mouse_moved(SDL_MouseMotionEvent* evt) {
    // TODO track mouse positions so we can get relative movements without using SDL's relative mouse mode since that
    //      one hides the cursor (which we dont want...)

    if (tool_mode == TOOL_MODE_PENCIL || tool_mode == TOOL_MODE_ERASER) {
        uint32_t tool_color = tool_mode == TOOL_MODE_PENCIL ? pencil_color : 0x00;
        if ((evt->state & SDL_BUTTON_LMASK) == SDL_BUTTON_LMASK) {
            // get "img_texture local" coords
            int64_t edit_x = evt->x - img_rect.x;
            int64_t edit_y = evt->y - img_rect.y;

            if (edit_x < 0 || edit_x >= img_rect.w) return; // out of bounds
            if (edit_y < 0 || edit_y >= img_rect.h) return; // out of bounds


            if(SDL_SetRenderTarget(rend, img_edit_texture) != 0) {
                fprintf(stderr, "could not set img_edit_texture as render target: %s\n", SDL_GetError());
                exit(5);
            }

            SDL_SetRenderDrawColor(rend, tool_color>>24, tool_color>>16, tool_color>>8, tool_color&0xff);

            SDL_Rect point_rect = { .x = edit_x-(pencil_width>>1), .y = edit_y-(pencil_width>>1), .w = pencil_width, .h = pencil_width };
            SDL_RenderFillRect(rend, &point_rect);
            SDL_SetRenderTarget(rend, NULL);

            return;
        }
    }
}

/**
 */
void handle_key_down(SDL_KeyboardEvent* evt) {
    if (tool_mode == TOOL_MODE_PENCIL || tool_mode == TOOL_MODE_ERASER) {
        if (evt->keysym.sym == SDLK_LEFTBRACKET) {
            // reduce pencil width and clip the minimum size to 1
            pencil_width = pencil_width > 1 ? pencil_width - 1 : pencil_width;
            return;

        } else if(evt->keysym.sym == SDLK_RIGHTBRACKET) {
            // increase pencil size and clip the maximum size to 255
            pencil_width = pencil_width <= 0xff ? pencil_width +  1 : pencil_width;
            return;
        }
    } else if (tool_mode == TOOL_MODE_TEXT_EDIT || tool_mode == TOOL_MODE_INPUT_COLOR) {
        if (evt->keysym.sym == SDLK_ESCAPE) {
            // if we press escape in any of the text input modes we "cancel"
            tool_mode = TOOL_MODE_NORMAL;
        }  else if (evt->keysym.sym == SDLK_RETURN || evt->keysym.sym == SDLK_RETURN2) {
            // if we press return in any of the text input modes we "confirm"
            // TODO: implement confirm_input
        } else if (evt->keysym.sym == SDLK_BACKSPACE || evt->keysym.sym == SDLK_KP_BACKSPACE) {
            // if we press escape we remove a letter from out input_buffer
            if (input_buffer_offset != input_buffer) *(--input_buffer_offset) = '\0';
        } 
        else if ((evt->keysym.sym >= 'a' && evt->keysym.sym <= 'z') || (evt->keysym.sym >= '0' && evt->keysym.sym <= '9') ) {
            // if we press one of the alphanumeric keys we'll add those to the input buffer
            if (input_buffer_offset != &input_buffer[INPUT_BUFFER_SIZE - 1]) *(input_buffer_offset++) = evt->keysym.sym;
        }
        return;
    } 

    switch(evt->keysym.sym) {
        // write image to stdout and bail!
        case SDLK_q:
            // TODO: implement write_png_to_stdout()
            should_be_running = 0;
            break;

        case SDLK_p:
            tool_mode = TOOL_MODE_PENCIL;
            break;

        case SDLK_d:
        case SDLK_e:
            tool_mode = TOOL_MODE_ERASER;
            break;

        case SDLK_c:
            wipe_input_buffer();
            tool_mode = TOOL_MODE_INPUT_COLOR;
            break;

        case SDLK_ESCAPE:
            tool_mode = TOOL_MODE_NORMAL;
            break;

    }
}

// -------

int main(int argc, char* argv[]) {
    // main flow
    // 1. read image from stdin
    // 2. main annotation loop
    // 3. write image back to stdout (assume png for now)

    // minimal set of required annotation tools
    //  * text
    //  * auto increment "bubbles"
    //  * arrows
    //  * pen drawings
    //  * eraser
    //  * color selection
    //  * primitives like circles and rectangles...

    // possible required features:
    //  * image panning inside the 10h0ch window if the image is waaaaaay too big
    //
    //
    //  -----------------------------------------------------------------------------

    // initialize some things
    init_sdl();
    init_font_config();
    init_interface();

    // read the image we'll be annotating from stdin (note that this will exit 10h0ch when an error occures)
    img_orig_surface = read_image_from_stdin();

    // this recalculates the img position n stuff...
    handle_window_resized();

    // create an sdl texture we can draw later
    img_orig_texture = SDL_CreateTextureFromSurface(rend, img_orig_surface);
    if (img_orig_texture == NULL) {
        SDL_FreeSurface(img_orig_surface);
        fprintf(stderr, "SDL could not create an SDL_Texture from the loaded image: %s\n", SDL_GetError());
        exit(1);
    }

    // now that we have our image we can create our annations layer SDL_Texture 
    //img_edit_surface = SDL_CreateRGBSurface(0, img_orig_surface->w, img_orig_surface->h, img_orig_surface->format->BitsPerPixel, img_orig_surface->format->Rmask, img_orig_surface->format->Gmask, img_orig_surface->format->Bmask, img_orig_surface->format->Amask);
    img_edit_texture = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, img_orig_surface->w, img_orig_surface->h);
    SDL_SetTextureBlendMode(img_edit_texture, SDL_BLENDMODE_BLEND);


    // start of the rendering our screen...
    render_screen();

    // fire up the main loop
    SDL_Event evt;
    should_be_running = 1;
    while(SDL_WaitEvent(&evt) && should_be_running) {

        // break out of our main loop if we're supposed to exit...
        if (evt.type == SDL_QUIT) break;

        switch(evt.type) {
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEMOTION:
                handle_mouse_moved(&evt.motion);
                break;
            case SDL_KEYDOWN:
                handle_key_down(&evt.key);
                break;
            case SDL_WINDOWEVENT:
                if (evt.window.event == SDL_WINDOWEVENT_RESIZED) {
                    handle_window_resized();
                }
                break;
        }

        render_screen();
    }

    // clean up after ourselves... (not that it should really be necesarry since we're about to die?)
    SDL_DestroyTexture(img_edit_texture);
    SDL_DestroyTexture(img_orig_texture);

    //SDL_FreeSurface(img_edit_surface);
    
    SDL_FreeSurface(img_orig_surface);

    SDL_Quit();

    return 0;
}


