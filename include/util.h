#ifndef _UTIL_H_
#define _UTIL_H_

#include <stddef.h>
#include <curses.h>

#include <err.h>

#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define ARR_SIZE(x) sizeof((x)) / sizeof((x)[0])

#define MIN_LINES 20
#define MIN_COLS 50

size_t utf8len(const char *str);

void fill(WINDOW *win, int from, int to, int ch);
void print_truncate(WINDOW *win, char *str, int len, int trunc);

#endif /* _UTIL_H_ */
