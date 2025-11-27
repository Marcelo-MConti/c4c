#ifndef _GAME_COMMON_H_
#define _GAME_COMMON_H_

#include <curses.h>

int col_is_not_full(struct game *game, int column);
void print_arrow(WINDOW *win, int idx);

#endif /* _GAME_COMMON_H_ */
