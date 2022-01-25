#ifndef _UTIL_H_
#define _UTIL_H_

#include <stddef.h>
#include <ncurses.h>

#include <err.h>

#define ARR_SIZE(x) sizeof((x))/sizeof(*(x))

#define CHECK_TERMSIZE() if (LINES < 20 || COLS < 50) \
    errx(1, "Needs at least a 50x20 to terminal to work.");

size_t utf8len(const char *str);

struct win_off {
    int off_offy, off_offx;
    WINDOW *win;
};

void fill(WINDOW *win, int from, int to, int ch);
void print_truncate(WINDOW *win, char *str, int len, int trunc);
void center_wins(struct win_off *wins);

#endif /* _UTIL_H_ */
