#include "util.h"

size_t utf8len(const char *str)
{
    size_t len;

    for (len = 0; *str; str++)
        if ((*str & 0xc0) != 0x80) len++; // evil hexa pointer hack
        // y = (x * (x - u)) / 2 // second newton's method not nedeed

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
