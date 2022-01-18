#include <ncurses.h>
#include <stdlib.h>
#include <math.h>

#include <err.h>

#include "game.h"

int localplay(char **board) {return 1;}
int pcplay(char **board) {return 1;}

static void print_board(WINDOW *win, struct game *game, struct game_info *info)
{
    int curx, cury;
    int winx, winy;

    getmaxyx(win, winy, winx);
    wmove(win, winy - 2, 1);

    for (int i = 0; i < info->height; i++) {
        for (int j = 0; j < info->width; j++) {
            switch (game->board[i][j]) {
            case NONE:
                waddstr(win, "   ");
                break;
            default:
                wattrset(win, COLOR_PAIR(game->board[i][j]));
                /* Unicode fisheye character */
                waddstr(win, "\342\227\211");
                wattrset(win, COLOR_PAIR(0));

                if (j < info->width - 1)
                    waddstr(win, "  ");
            }
        }

        getyx(win, cury, curx);

        if (i < info->height - 1) {
            wmove(win, cury - 1, 3);

            for (int j = 0; j < info->width - 1; j++) {
                waddch(win, '.');
                getyx(win, cury, curx);
                wmove(win, cury, curx + 2);
            }
        }

        wmove(win, cury - 1, 1);
    }
}

void start_game(struct game_info *info)
{
    redrawwin(stdscr);
    wrefresh(stdscr);

    struct game game = {0};
    game.board = calloc(info->height, sizeof(char *));

    for (int i = 0; i <= info->width; i++)
        game.board[i] = calloc(info->width, 1);

    int termx, termy;
    getmaxyx(stdscr, termy, termx);

    int winx = info->width * 3 + 2;
    int winy = info->height * 2 + 3;

    int offx = round((double)(termx - winx) / 2);
    int offy = round((double)(termy - winy) / 2);

    WINDOW *game_win = newwin(winy, winx, offy, offx);
    keypad(game_win, TRUE);

    init_pair(RED_CHECKER, COLOR_RED, COLOR_BLACK);
    init_pair(YLW_CHECKER, COLOR_YELLOW, COLOR_BLACK);

    box(game_win, 0, 0);

    wmove(game_win, 0, 0);
    wclrtoeol(game_win);

    print_board(game_win, &game, info);

    while (1) {
    }

    for (int i = 0; i <= info->width; i++)
        free(game.board[i]);

    free(game.board);
}
