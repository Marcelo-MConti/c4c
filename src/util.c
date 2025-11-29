#include "util.h"
#include <stdlib.h>
#include <time.h>

size_t utf8len(const char *str)
{
    size_t len;

    for (len = 0; *str; str++)
        if ((*str & 0xc0) != 0x80) len++;

    return len;
}

void fill(WINDOW *win, int from, int to, int ch)
{
    int cury, curx;
    getyx(win, cury, curx);

    wmove(win, cury, from);
    for (int i = from; i <= to; i++)
        waddch(win, ch);
    wmove(win, cury, from);
}

void print_truncate(WINDOW *win, char *str, int len, int trunc)
{
    if (len > trunc) {
#ifdef C4C_ASCII
        waddnstr(win, str, trunc - 3);
        waddstr(win, "...");
#else
        waddnstr(win, str, trunc - 1);
        waddstr(win, u8"…");
#endif
    } else {
        waddnstr(win, str, len);
    }
}

__attribute__((constructor))
static void srand32()
{
    srand(time(NULL));
}

uint32_t rand32()
{
    static const int n_bits = __builtin_clz(RAND_MAX);

    uint32_t result = 0;

    for (int shifted = 0; shifted < 16; shifted += n_bits)
        result = (result << n_bits) + rand();

    return result;
}
