#ifndef _GAME_COMMON_H_
#define _GAME_COMMON_H_

#include <curses.h>

#include "game.h"

#define PLAYER_TO_CHECKER(p) ((p) + 1)

enum player {
    PLAYER_RED,
    PLAYER_YLW
};

extern volatile bool player_wants_to_quit;

int col_is_not_full(struct game *game, int column);
void print_arrow(WINDOW *win, int idx);
void print_hud(struct game *game);

#endif /* _GAME_COMMON_H_ */
