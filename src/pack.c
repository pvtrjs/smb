#include <string.h>
#include <stdlib.h>
#include "pack.h"

/*[2 bytes len][n bytes payload]*/

uint8_t unpack_u8(const uint8_t **buff) {
    uint8_t value = **buff;
    (*buff)++;
    return value;
}

uint16_t unpack_u16(const uint8_t **buf) {
    uint16_t val = ((uint16_t)(*buf)[0] << 8) | (*buf)[1];
    *buf += 2;
    return val;
}

uint32_t unpack_u32(const uint8_t **buf) {
    uint32_t val = ((uint32_t)(*buf)[0] << 24) | ((uint32_t)(*buf)[1] << 16) | ((uint32_t)(*buf)[2] << 8) | (*buf)[3];
    *buf += 4;
    return val;
}

uint8_t *unpack_bytes(const uint8_t **buf, size_t len, uint8_t *dest) {
    if (dest == NULL) {
        dest = malloc(len + 1);
        if (dest == NULL) {
            return NULL; // Memory allocation failed
        }
    }
    memcpy(dest, *buf, len);
    dest[len] = '\0'; // Ensure null-termination
    *buf += len;
    return dest;
}

uint16_t unpack_string16(uint8_t **buff, uint8_t **dest) {
    uint16_t len = unpack_u16((const uint8_t **)buff);
    if (len == 0) {
        *dest = NULL;
        return 0;
    }
    *dest = unpack_bytes(buff, len, NULL);
    if (*dest == NULL) {
        return 0; // Memory allocation failed
    }
    return len;
}
