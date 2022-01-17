#include "util.h"

size_t utf8len(const char *str)
{
    size_t len;

    for (len = 0; *str; str++)
        if ((*str & 0xc0) != 0x80) len++;

    return len;
}
