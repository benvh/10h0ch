#include "image.h"
#include "rendering.h"
#include <SDL2/SDL_image.h>

SDL_Surface* img_orig_surface = NULL;
SDL_Texture* img_orig_texture = NULL;
SDL_Texture* img_edit_texture = NULL;

void image_init() { 
    // initialize SDL_image
    uint32_t img_flags = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF;
    if ((IMG_Init(img_flags) & img_flags) != img_flags) {
        fprintf(stderr, "couldnot initialize SDL image: %s\n", IMG_GetError());
        SDL_DestroyWindow(wnd);
        SDL_Quit();
        exit(-1);
    }
}

void image_deinit() {
    IMG_Quit();
}

SDL_Surface* image_read_img_from_stdin() {
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
                exit(-1); // TODO: instead of exiting over here return NULL or something?
            }
        }
        // read a byte from stdin
        img_buff[img_buff_offset++] = c;
    }

    // we read 0 bytes and hit an EOF...
    if (img_buff_offset == 0) {
        fprintf(stderr, "please provide a valid jpg, png or tif image via stdin\n");
        exit(-1);
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
        exit(-1); // TODO: instead of exiting over here return NULL or something?
    }

    // and then actually read the image data...
    SDL_Surface* img_surface = IMG_Load_RW(sdl_img_buff, 0);
    if (img_surface == NULL) {
        fprintf(stderr, "SDL_image could not load the image: %s\n", IMG_GetError());
        SDL_RWclose(sdl_img_buff);
        free(img_buff);
        exit(-1); // TODO: instead of exiting over here return NULL or something?
    }

    // clean up after ourselves
    SDL_RWclose(sdl_img_buff);
    free(img_buff);

    // initialize img_orig_... and img_edit_... vars
    img_orig_surface = img_surface;
    img_orig_texture = SDL_CreateTextureFromSurface(rend, img_orig_surface);
    img_edit_texture = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, img_orig_surface->w, img_orig_surface->h);
    SDL_SetTextureBlendMode(img_edit_texture, SDL_BLENDMODE_BLEND);

    return img_surface;
}

void image_write_img_to_stdout() {
}

void image_render_img() {
    SDL_Rect img_dest_rect = image_get_offset_rect();
    SDL_RenderCopy(rend, img_orig_texture, &(img_orig_surface->clip_rect), &img_dest_rect);
    SDL_RenderCopy(rend, img_edit_texture, &(img_orig_surface->clip_rect), &img_dest_rect);
}

SDL_Rect image_get_offset_rect() {
    return (SDL_Rect) {
        .x = (wnd_rect.w>>1) - (img_orig_surface->clip_rect.w>>1),
        .y = (wnd_rect.h>>1) - (img_orig_surface->clip_rect.h>>1),
        .w = img_orig_surface->clip_rect.w,
        .h = img_orig_surface->clip_rect.h,
    };
}
