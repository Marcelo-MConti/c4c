#include <string.h>
#include <curses.h>

#include "chars.h"
#include "game.h"
#include "game/common.h"

int col_is_not_full(struct game *game, int column)
{
    enum tile (*board)[game->width] = game->board;

    for (int i = 0; i < game->height; i++) {
        if (board[i][column] == TILE_NONE)
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

void print_hud(struct game *game)
{
    static const char *hud_msg = "(^C) Quit     (<-/->/Home/End) Move arrow     (Enter) Play tile";

    wmove(stdscr, LINES - 1, 1);

    waddch(stdscr, game->cur_player == PLAYER_RED ? '[' : ' ');
    
    wattrset(stdscr, COLOR_PAIR(TILE_RED_CHECKER));
    waddstr(stdscr, checkers[TILE_RED_CHECKER]);
    wattrset(stdscr, COLOR_PAIR(0));
    
    waddch(stdscr, game->cur_player == PLAYER_RED ? ']' : ' ');

    waddch(stdscr, game->cur_player == PLAYER_YLW ? '[' : ' ' | COLOR_PAIR(TILE_YLW_CHECKER));

    wattrset(stdscr, COLOR_PAIR(TILE_YLW_CHECKER));
    waddstr(stdscr, checkers[TILE_YLW_CHECKER]);
    wattrset(stdscr, COLOR_PAIR(0));

    waddch(stdscr, game->cur_player == PLAYER_YLW ? ']' : ' ' | COLOR_PAIR(0));

    mvwaddstr(stdscr, LINES - 1, COLS - strlen(hud_msg) - 1, hud_msg);

    wnoutrefresh(stdscr);
} 
