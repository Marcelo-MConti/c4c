#ifndef _GAME_H_
#define _GAME_H_

#include <stdint.h>
#include <curses.h>

#define DEFAULT_PORT 36540

enum play_mode {
    PLAY_PLPL,
    PLAY_PLPC,
    PLAY_PCPC,
    PLAY_NET
};

enum tile {
    NONE = 0,
    RED_CHECKER,
    YLW_CHECKER
};

struct game {
    enum tile (*board)[];
    uint8_t cur_player;
    int width, height;
};

typedef int32_t move_t;

struct position {
    int x, y;
};

void start_game(int width, int height, enum play_mode, void (*on_redraw)(WINDOW *, void *ctx), void *ctx);

#endif /* _GAME_H_ */
