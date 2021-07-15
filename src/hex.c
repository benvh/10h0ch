#include <string.h>

#include "hex.h"

uint8_t ctoi(const char c) {
    if (c >= 'a' && c <= 'z') {
        return 0x0a + (c - 'a');
    } else if (c >= 'A' && c <= 'Z') {
        return 0x0a + (c - 'A');
    } else if (c >= '0' && c <= '9') {
        return 0x00 + (c - '0');
    }
    return 0;
}

char itoc(const uint8_t i) {
    if (i >= 0 && i <= 9) {
        return '0' + i;
    }
    else if (i >= 10 && i <= 16) {
        return 'A' + (i - 10);
    }
    return '0';
}

uint64_t hex_parse_hex_str(const char* hexstr) {
    size_t hexstr_length = strlen(hexstr);
    if ((hexstr_length & 1) != 0) {
        return 0x00;
    }

    uint8_t ms_nibble = 0x00;
    uint8_t ls_nibble = 0x00;
    uint64_t out = 0x00;

    // we cap at 16 chars as we are limited to one uint64_t
    for(int i = 0; i < hexstr_length && i < 16; i+=2) {
        ms_nibble = ctoi(hexstr[i])<<4;
        ls_nibble = ctoi(hexstr[i+1]);

        // shift 'out' left to make room for our next byte and dump it in...
        out = (out<<8) | (ms_nibble | ls_nibble);
    }

    return out;
}

char* hex_encode_hex_str(const uint64_t num) {
    return NULL;
}
