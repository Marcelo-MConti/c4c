#include <curses.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "game.h"
#include "chars.h"

const static struct position neighbour_pos[] = {
    { 0,  1 }, {  1,  1 }, {  1,  0 }, {  1, -1 },
    { 0, -1 }, { -1, -1 }, { -1,  0 }, { -1,  1 }
};

static int col_is_not_full(struct game *game, int column)
{
    enum tile (*board)[game->width] = game->board;

    for (int i = 0; i < game->height; i++) {
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
    mvwaddstr(win, cury, ind * 3 + 1, arrow);
}

static inline bool is_valid_nonempty_pos(struct game *game, struct position *pos)
{
    if ((pos->x < 0 || pos->y < 0) || (pos->x >= game->width || pos->y >= game->height))
        return false;

    enum tile (*board)[game->width] = game->board;
    return board[pos->y][pos->x] != NONE;
}

static bool check_win(struct game *game, struct position *pos)
{
    enum tile (*board)[game->width] = game->board;

    if (board[pos->y][pos->x] == NONE)
        return false;

    char same_neighbours[4] = {0};

    for (int i = 0; i < 8; i++) {
        struct position chk_pos = { pos->x, pos->y };

        for (int j = 0; j < 4; j++) {
            chk_pos.x += neighbour_pos[i].x;
            chk_pos.y += neighbour_pos[i].y;

            if (is_valid_nonempty_pos(game, &chk_pos) &&
                    board[pos->y][pos->x] == board[chk_pos.y][chk_pos.x])
                same_neighbours[i % 4]++;
            else
                break;
        }
    }

    for (int i = 0; i < 4; i++) {
        if (same_neighbours[i] >= 3)
            return true;
    }

    return false;
}

struct position *local_make_move(WINDOW *win, struct game *game)
{
    static struct position pos = {0};
    int idx = pos.x, ch, tmp;

    wmove(win, 0, 1);
    print_arrow(win, idx);

    while (true) {
        ch = wgetch(win);
        
        switch (ch) {
            case KEY_LEFT:
                if (idx != 0) {
                    idx--;

                    wmove(win, 0, 1);
                    print_arrow(win, idx);
                }

                break;
            case KEY_RIGHT:
                if (idx != game->width - 1) {
                    idx++;

                    wmove(win, 0, 1);
                    print_arrow(win, idx);
                }

                break;
            case KEY_HOME:
                idx = 0;

                wmove(win, 0, 1);
                print_arrow(win, idx);

                break;
            case KEY_END:
                idx = game->width - 1;

                wmove(win, 0, 1);
                print_arrow(win, idx);

                break;
            case '\n':
            case ' ':
                if ((tmp = col_is_not_full(game, idx)) != -1) {
                    pos.x = idx;
                    pos.y = tmp;

                    wmove(win, 0, 1);
                    wclrtoeol(win);

                    return &pos;
                } else {
                    // XXX: show error message
                }

                break;
            case KEY_RESIZE:
                // XXX: check terminal size

                break;
            case '1' ... '7':
                idx = ch - '1';

                wmove(win, 0, 1);
                print_arrow(win, idx);
            }
    }
}

static void print_board(WINDOW *win, struct game *game)
{
    int curx, cury;
    int winx, winy;

    getmaxyx(win, winy, winx);
    wmove(win, winy - 2, 1);

    enum tile (*board)[game->width] = game->board;

    for (int i = 0; i < game->height; i++) {
        for (int j = 0; j < game->width; j++) {
            wattrset(win, COLOR_PAIR(board[i][j]));
            waddstr(win, checkers[board[i][j]]);
            wattrset(win, COLOR_PAIR(0));

            if (j < game->height - 1)
                waddstr(win, "  ");
        }

        getyx(win, cury, curx);

        if (i < game->height - 1) {
            wmove(win, cury - 1, 3);

            for (int j = 0; j < game->width - 1; j++) {
                waddch(win, '.');
                getyx(win, cury, curx);
                wmove(win, cury, curx + 2);
            }
        }

        wmove(win, cury - 1, 1);
    }
}

void start_game(int width, int height, enum play_mode mode)
{
    redrawwin(stdscr);
    wrefresh(stdscr);

    struct game game = {
        .width = width, .height = height,
        .board = calloc(1, sizeof(enum tile[height][width]))
    };

    int win_width = game.width * 3 + 2;
    int win_height = game.height * 2 + 3;

    int x_offset = (COLS - win_width) / 2;
    int y_offset = (LINES - win_height) / 2;

    WINDOW *game_win = newwin(win_height, win_width, y_offset, x_offset);
    keypad(game_win, TRUE);

    init_pair(RED_CHECKER, COLOR_RED, COLOR_BLACK);
    init_pair(YLW_CHECKER, COLOR_YELLOW, COLOR_BLACK);

    box(game_win, 0, 0);

    wmove(game_win, 0, 0);
    wclrtoeol(game_win);

    // switch (mode) {
    // case PLAY_PLPL:
    //     game.make_move[0] = local_make_move;
    //     game.make_move[1] = local_make_move;
    // }

    print_board(game_win, &game);

    enum tile (*board)[game.width] = game.board;
    int curplayer = 0;
    while (1) {
        struct position *mv = local_make_move(game_win, &game);

        board[mv->y][mv->x] = (curplayer == 0) ?
            RED_CHECKER : YLW_CHECKER;

        print_board(game_win, &game);

        if (check_win(&game, mv)) {
            wrefresh(game_win);

            y_offset = (LINES - 3) / 2 - 10;
            x_offset = (COLS - 20) / 2;

            WINDOW *msg_win = newwin(3, 20, y_offset, x_offset);

            box(msg_win, 0, 0);

            mvwprintw(msg_win, 1, 4, "Player %d won.", curplayer + 1);
            wgetch(msg_win);

            delwin(msg_win);

            break;
        }

        curplayer = !curplayer;
    }

    delwin(game_win);

    free(game.board);
}
