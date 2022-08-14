#ifndef _UTIL_H_
#define _UTIL_H_

#include <stddef.h>
#include <curses.h>

#include <err.h>

#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define ARR_SIZE(x) sizeof((x))/sizeof((x)[0])

#define CHECK_TERMSIZE() if (LINES < 20 || COLS < 50) \
    errx(1, "Needs at least a 50x20 to terminal to work.");

size_t utf8len(const char *str);

void fill(WINDOW *win, int from, int to, int ch);
void print_truncate(WINDOW *win, char *str, int len, int trunc);

#endif /* _UTIL_H_ */
