#ifndef _10h0ch_hex_h
#define _10h0ch_hex_h

#include <stdint.h>

/**
 * parse a non-prefixed/suffixed and non-delimited hex string into its number
 * representation.
 *
 * again, this function expects no pressence of "0x" prefixes or "h"
 * suffixes and no delimiters between the separate bytes. just one big string
 * of valid hex characters, e.g. f28d9fdd7c07ff05.
 *
 * If the hex string is not a multipe of 2 this function will as
 * return 0 as that is considered an "invalid hex string".
 *
 * If the hex string contains more than 8 bytes (16 chars) this function will
 * completely ignore the extra hex string byte values as we're only doing
 * uint64_t over here...
 *
 * note that the hex string is interpreted as Big Endian...
 */
uint64_t hex_parse_hex_str(const char* hexstr);

/**
 * encode \p num into its hex string representation. the generated hex strings
 * will not have any "0x" prefix or "h" suffix and no delimiters between the
 * separate bytes. 
 *
 * note that the generated hex string will be encoded as Big Endian...
 */
char* hex_encode_hex_str(const uint64_t num);

#endif
