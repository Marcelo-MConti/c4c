#ifndef _UTIL_H_
#define _UTIL_H_

#include <stddef.h>
#include <curses.h>

#include <err.h>

#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define ARR_SIZE(x) sizeof((x)) / sizeof((x)[0])

#define _STR(x) #x
#define STR(x) _STR(x)

#define MIN_LINES 30
#define MIN_COLS 80

size_t utf8len(const char *str);

void fill(WINDOW *win, int from, int to, int ch);
void print_truncate(WINDOW *win, char *str, int len, int trunc);

uint32_t randinho(void);

#endif /* _UTIL_H_ */
