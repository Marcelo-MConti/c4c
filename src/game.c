#include <ncurses.h>
#include <stdlib.h>
#include <math.h>

#include <err.h>

#include "game.h"

int localplay(char **board) {return 1;}
int pcplay(char **board) {return 1;}

static void print_board(char **board)
{

}

void start_game(struct game_info *info)
{
    struct game game = {0};
    game.board = calloc(info->width, info->height);


    if (!game.board)
        errx(1, "failed to malloc()");

    int winx, winy;
    getmaxyx(stdscr, winy, winx);

    int offx = round((double)(winx - info->height + 6) / 2);
    int offy = round((double)(winy - info->width + 2) / 2);

    WINDOW *gamewin = newwin(info->height + 6, info->width + 2, offy, offx);


    switch (info->play_mode) {
    case PLAY_PLPL:
        game.play_func[0] = localplay;
        game.play_func[1] = localplay;

        break;
    case PLAY_PLPC:
        game.play_func[0] = localplay;
        game.play_func[1] = pcplay;

        break;
    case PLAY_PCPC:
        game.play_func[0] = pcplay;
        game.play_func[1] = pcplay;

        break;
    }

    print_board(game.board);

    while (1) {
#if 0
        game.play_func[game.curplayer](game.board);

        printboard(game.board);

        game.curplayer = !game.curplayer;
#else
        wgetch(gamewin);
#endif
    }
}
