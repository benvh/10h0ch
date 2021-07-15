#include "tool.h"

static char* status_bar_text = "[normal]";

char* tools_mode_none_provide_status_bar_text() {
    return status_bar_text;
}

tool_t tools_tool_none = {
    .status_bar_text_provider = tools_mode_none_provide_status_bar_text
};

