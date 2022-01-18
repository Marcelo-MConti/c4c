#ifndef _UTIL_H_
#define _UTIL_H_

#include <stddef.h>
#include <ncurses.h>

#define ARR_SIZE(x) sizeof((x))/sizeof(*(x))

size_t utf8len(const char *str);

struct win_off {
    int off_offy, off_offx;
    WINDOW *win;
};

void fill(WINDOW *win, int from, int to, int ch);
void print_truncate(WINDOW *win, char *str, int len, int trunc);
void center_wins(struct win_off *wins, int termy, int termx);

#endif /* _UTIL_H_ */
