#include <curses.h>

#include "chars.h"
#include "game.h"
#include "game/common.h"

int col_is_not_full(struct game *game, int column)
{
    enum tile (*board)[game->width] = game->board;

    for (int i = 0; i < game->height; i++) {
        if (board[i][column] == NONE)
            return i;
    }

    return -1;
}

void print_arrow(WINDOW *win, int ind)
{
    int curx, cury;
    getyx(win, cury, curx);

    wclrtoeol(win);
    mvwaddstr(win, cury, ind * 3 + 1, arrow);
}
