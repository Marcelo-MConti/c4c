#include <ncurses.h>
#include <stdlib.h>

#include "util.h"
#include "game.h"

/* the other 4 complimentary neighbours are just negated versions of these */
const static struct move neighbour_pos[] = {
    { 0, 1 }, { 1, 1 }, { 1, 0 }, { 1, -1 }
};

static int col_is_not_full(char **board, struct game_info *info, int column)
{
    for (int i = 0; i < info->y; i++) {
        if (board[i][column] == NONE)
            return i;
    }

    return -1;
}

static void print_arrow(WINDOW *win, int ind)
{
    int curx, cury;
    getyx(win, cury, curx);

    wclrtoeol(win);

    /* Unicode right block diagonal (U+1FB65) */
    mvwaddstr(win, cury, ind * 3 + 1, "\360\237\255\245");
    /* Unicode left  block diagonal (U+1FB5A) */
    waddstr(win, "\360\237\255\232");
}

static inline int is_nonempty_pos(struct game *game, struct game_info *info, struct move *mv)
{
    if (mv->x < 0 || mv->y < 0)
        return 0;
    if (mv->x >= info->x || mv->y >= info->y)
        return 0;

    if (game->board[mv->y][mv->x] != NONE)
        return 1;
    else
        return 0;
}

static int check_win(struct game *game, struct game_info *info, struct move *mv)
{
    if (game->board[mv->y][mv->x] == NONE)
        return 0;

    char same_neighbours[4] = {0};

    for (int i = 0; i < ARR_SIZE(neighbour_pos); i++) {
        struct move chk_pos = { mv->x, mv->y };

        for (int j = 0; j < 4; j++) {
            chk_pos.x += neighbour_pos[i].x;
            chk_pos.y += neighbour_pos[i].y;

            if (!is_nonempty_pos(game, info, &chk_pos))
                break;

            if (game->board[mv->y][mv->x] == game->board[chk_pos.y][chk_pos.x])
                same_neighbours[i]++;
            else
                break;
        }
    }

    for (int i = 0; i < ARR_SIZE(neighbour_pos); i++) {
        struct move chk_pos = { mv->x, mv->y };

        for (int j = 0; j < 4; j++) {
            chk_pos.x -= neighbour_pos[i].x;
            chk_pos.y -= neighbour_pos[i].y;

            if (!is_nonempty_pos(game, info, &chk_pos))
                break;

            if (game->board[mv->y][mv->x] == game->board[chk_pos.y][chk_pos.x])
                same_neighbours[i]++;
            else
                break;
        }
    }

    for (int i = 0; i < 4; i++) {
        if (same_neighbours[i] >= 3)
            return 1;
    }

    return 0;
}

struct move *local_make_move(WINDOW *win, struct game *game, struct game_info *info)
{
    static struct move mv = {0};

    int ind = 0, ch, tmp;

    wmove(win, 0, 1);
    print_arrow(win, ind);

    while ((ch = wgetch(win))) {
        switch (ch) {
        case KEY_LEFT:
            if (ind != 0) {
                ind--;

                wmove(win, 0, 1);
                print_arrow(win, ind);
            }

            break;
        case KEY_RIGHT:
            if (ind < info->x - 1) {
                ind++;

                wmove(win, 0, 1);
                print_arrow(win, ind);
            }

            break;
        case '\n':
        case ' ':
            if ((tmp = col_is_not_full(game->board, info, ind)) != -1) {
                mv.x  = ind;
                mv.y = tmp;

                wmove(win, 0, 1);
                wclrtoeol(win);

                return &mv;
            } else {
                // show error message
            }

            break;
        case KEY_RESIZE:;
            CHECK_TERMSIZE();

            struct win_off wins[2] = {0};
            wins[0].win = win;

            center_wins(wins);
        }
    }
}

static void print_board(WINDOW *win, struct game *game, struct game_info *info)
{
    int curx, cury;
    int winx, winy;

    getmaxyx(win, winy, winx);
    wmove(win, winy - 2, 1);

    for (int i = 0; i < info->y; i++) {
        for (int j = 0; j < info->x; j++) {
            if (game->board[i][j] == NONE) {
                waddstr(win, "   ");
            } else {
                wattrset(win, COLOR_PAIR(game->board[i][j]));
                /* Unicode fisheye character */
                waddstr(win, "\342\227\211");
                wattrset(win, COLOR_PAIR(0));

                if (j < info->x - 1)
                    waddstr(win, "  ");
            }
        }

        getyx(win, cury, curx);

        if (i < info->y - 1) {
            wmove(win, cury - 1, 3);

            for (int j = 0; j < info->x - 1; j++) {
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
    game.board = calloc(info->y, sizeof(char *));

    for (int i = 0; i < info->y; i++)
        game.board[i] = calloc(info->x, 1);

    int winx = info->x * 3 + 2;
    int winy = info->y * 2 + 3;

    int offx = (COLS - winx) / 2;
    int offy = (LINES - winy) / 2;

    WINDOW *game_win = newwin(winy, winx, offy, offx);
    keypad(game_win, TRUE);

    init_pair(RED_CHECKER, COLOR_RED, COLOR_BLACK);
    init_pair(YLW_CHECKER, COLOR_YELLOW, COLOR_BLACK);

    box(game_win, 0, 0);

    wmove(game_win, 0, 0);
    wclrtoeol(game_win);

    switch (info->play_mode) {
    case PLAY_PLPL:
        game.make_move[0] = local_make_move;
        game.make_move[1] = local_make_move;
    }

    print_board(game_win, &game, info);

    int curplayer = 0;
    while (1) {
        struct move *mv = game.make_move[curplayer](game_win, &game, info);

        if (curplayer == 0)
            game.board[mv->y][mv->x] = RED_CHECKER;
        else
            game.board[mv->y][mv->x] = YLW_CHECKER;

        print_board(game_win, &game, info);

        if (check_win(&game, info, mv)) {
            offy = (LINES - 3) / 2;
            offx = (COLS - 20) / 2;

            WINDOW *msg_win = newwin(3, 20, offy, offx);

            box(msg_win, 0, 0);

            mvwprintw(msg_win, 1, 4, "Player %d won.", curplayer + 1);
            wgetch(msg_win);

            delwin(msg_win);

            break;
        }

        curplayer = !curplayer;
    }

    for (int i = 0; i < info->y; i++)
        free(game.board[i]);

    free(game.board);
}
