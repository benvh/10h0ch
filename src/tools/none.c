#include "tool.h"
#include "image.h"

//static char* status_bar_text = "[normal]";
static char status_bar_buff[64];

char* tools_mode_none_provide_status_bar_text() {
    sprintf(status_bar_buff, "[normal] | (%d x %d)", img_orig_surface->w, img_orig_surface->h);
    return status_bar_buff;
}

tool_t tools_tool_none = {
    .status_bar_text_provider = tools_mode_none_provide_status_bar_text
};

