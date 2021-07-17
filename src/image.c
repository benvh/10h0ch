#include "image.h"
#include "rendering.h"

#include <png.h>
#include <SDL2/SDL_image.h>
#include <stdlib.h>

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
    SDL_DestroyTexture(img_edit_texture);
    SDL_DestroyTexture(img_orig_texture);
    SDL_FreeSurface(img_orig_surface);
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
    SDL_SetTextureBlendMode(img_orig_texture, SDL_BLENDMODE_NONE);

    img_edit_texture = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, img_orig_surface->w, img_orig_surface->h);
    SDL_SetTextureBlendMode(img_edit_texture, SDL_BLENDMODE_NONE);
    SDL_SetRenderTarget(rend, img_edit_texture);
    SDL_RenderCopy(rend, img_orig_texture, NULL, NULL);

    SDL_SetRenderTarget(rend, NULL);

    return img_surface;
}


void image_write_img_to_stdout() {
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        fprintf(stderr, "could not create png_write_struct [exiting]\n");
        exit(-1);
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        fprintf(stderr, "could not create png_info_struct [exiting]\n");
        exit(-1);
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "png error [exiting]\n");
        exit(-1);
    }

    // allocate memory to hold our pixel data and read our modified image from screen
    uint8_t* pixel_data = (uint8_t*)malloc(img_orig_surface->w * img_orig_surface->h * 4);
    uint32_t pixel_row_stride = img_orig_surface->w * 4;
    SDL_Rect img_offset_rect = image_get_offset_rect();
    SDL_SetRenderTarget(rend, NULL); // make sure our window is our render target so we read whatever is visible on screen...
    SDL_RenderReadPixels(rend, &img_offset_rect, SDL_PIXELFORMAT_RGBA32, pixel_data, pixel_row_stride);

    // build the png rows array (libpng wants an array of pointers to the first pixel of every row)
    uint8_t** png_row_ptrs = (uint8_t**)malloc(img_orig_surface->h * sizeof(uint8_t*));
    for(int i = 0; i < img_orig_surface->h; i++) {
        png_row_ptrs[i] = (pixel_data + pixel_row_stride * i);
    }

    // write the png to stdout
    png_init_io(png_ptr, stdout);
    png_set_IHDR(png_ptr, info_ptr, img_orig_surface->w, img_orig_surface->h, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_set_rows(png_ptr, info_ptr, png_row_ptrs);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    // clean up after ourselves
    free(png_row_ptrs);
    free(pixel_data);
    png_destroy_write_struct(&png_ptr, &info_ptr);
}


void image_render_img() {
    SDL_Rect img_dest_rect = image_get_offset_rect();
    SDL_SetRenderTarget(rend, NULL);
    SDL_RenderCopy(rend, img_orig_texture, NULL, &img_dest_rect);
    SDL_RenderCopy(rend, img_edit_texture, NULL, &img_dest_rect);
}

SDL_Rect image_get_offset_rect() {
    return (SDL_Rect) {
        .x = (wnd_rect.w>>1) - (img_orig_surface->clip_rect.w>>1),
        .y = (wnd_rect.h>>1) - (img_orig_surface->clip_rect.h>>1),
        .w = img_orig_surface->clip_rect.w,
        .h = img_orig_surface->clip_rect.h,
    };
}
