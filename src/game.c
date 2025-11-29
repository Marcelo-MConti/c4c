#include <curses.h>
#include <string.h>
#include <stdlib.h>

#include "game.h"
#include "game/local.h"

#include "chars.h"


#define ERROR_COLOR RED_CHECKER
#define WARN_COLOR  YLW_CHECKER

#define PLAYER_TO_CHECKER(p) ((p) + 1)


enum player {
    PLAYER_RED,
    PLAYER_YLW
};


/**
 * Esse array é ordenado de forma que as posições `i` e `i + 4`
 * sejam inversas uma da outra. Isso simplifica a lógica da função
 * `check_win`.
 */
static const struct position neighbour_pos[] = {
    { 0,  1 }, {  1,  1 }, {  1,  0 }, {  1, -1 },
    { 0, -1 }, { -1, -1 }, { -1,  0 }, { -1,  1 }
};

static const char *end_messages[][2] = {
    [PLAY_LOCAL] = { "Red won!", "Yellow won!" },
    [PLAY_LOCAL_PC] = { "You won.", "The computer won." },
    [PLAY_NET] = { "You won.", "You lost." }
};


static inline bool is_valid_nonempty_pos(struct game *game, struct position *pos)
{
    if ((pos->x < 0 || pos->y < 0) || (pos->x >= game->width || pos->y >= game->height))
        return false;

    enum tile (*board)[game->width] = game->board;
    return board[pos->y][pos->x] != NONE;
}

static uint8_t (*check_win(struct game *game, struct position *pos))[4]
{
    enum tile (*board)[game->width] = game->board;

    if (board[pos->y][pos->x] == NONE)
        return false;

    static uint8_t same_neighbours[4];
    memset(same_neighbours, 0, sizeof same_neighbours);

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
            return &same_neighbours;
    }

    return NULL;
}

static void print_board(WINDOW *win, struct game *game)
{
    int curx, cury;
    int winx, winy;

    getmaxyx(win, winy, winx);
    wmove(win, winy - 2, 1);

    enum tile (*board)[game->width] = game->board;
    uint8_t (*blink)[game->width] = game->blink;

    for (int i = 0; i <game->height; i++) {
        for (int j = 0; j < game->width; j++) {
            wattrset(win, COLOR_PAIR(board[i][j]) | A_BLINK * blink[i][j]);
            waddstr(win, checkers[board[i][j]]);
            wattrset(win, COLOR_PAIR(0) | A_NORMAL);

            if (j <= game->height - 1)
                waddstr(win, "  ");
        }

        getyx(win, cury, curx);

        if (i < game->height - 1) {
            wmove(win, cury - 1, 3);

            for (int j = 0; j < game->width - 1; j++) {
#ifdef C4C_COLOR
                waddch(win, '+');
#else
                waddch(win, '.');
#endif
                getyx(win, cury, curx);
                wmove(win, cury, curx + 2);
            }
        }

        wmove(win, cury - 1, 1);
    }
}

static void mark_winning_tiles(WINDOW *win, struct game *game, struct position *pos, int winning_axis)
{
    enum tile (*board)[game->width] = game->board;
    uint8_t (*blink)[game->width] = game->blink;

    wattrset(win, COLOR_PAIR(board[pos->y][pos->x] | A_BLINK));

    struct position chk = { pos->x, pos->y };

    while (is_valid_nonempty_pos(game, &chk) &&
            board[pos->y][pos->x] == board[chk.y][chk.x]) {
        blink[chk.y][chk.x] = 1;

        chk.x += neighbour_pos[winning_axis].x;
        chk.y += neighbour_pos[winning_axis].y;
    }
    
    wattrset(win, COLOR_PAIR(0));
}

void start_game(int width, int height, enum play_mode mode, void (*on_redraw)(WINDOW *, void *ctx), void *ctx)
{
    redrawwin(stdscr);
    wrefresh(stdscr);

    struct game game = {
        .width = width, .height = height,
        .blink = calloc(1, sizeof(uint8_t[height][width])),
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

    print_board(game_win, &game);

    int cur_player = 0;
    enum tile (*board)[game.width] = game.board;

    while (true) {
        struct position *pos = local_play(game_win, &game, on_redraw, ctx);

        board[pos->y][pos->x] = PLAYER_TO_CHECKER(cur_player);
        print_board(game_win, &game);

        uint8_t (*winning_axes)[4] = check_win(&game, pos);

        if (winning_axes) {
            for (int i = 0; i < 4; i++) {
                if ((*winning_axes)[i] >= 3) {
                    mark_winning_tiles(game_win, &game, pos, i);
                    mark_winning_tiles(game_win, &game, pos, i + 4);
                }
            }

            print_board(game_win, &game);

            wrefresh(game_win);

            y_offset = (LINES - 3) / 2 - 10;
            x_offset = (COLS - 20) / 2;

            mvwaddstr(stdscr, LINES - 3, 1, end_messages[mode][cur_player]);
            wgetch(stdscr);

            break;
        }

        cur_player = !cur_player;
    }

    delwin(game_win);

    free(game.board);
}
