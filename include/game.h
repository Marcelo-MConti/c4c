#ifndef _GAME_H_
#define _GAME_H_

#include <stdint.h>
#include <curses.h>

#define DEFAULT_PORT 36540

#define DEFAULT_WIDTH 7
#define DEFAULT_HEIGHT 6

enum play_mode {
    PLAY_LOCAL,
    PLAY_LOCAL_PC,
    PLAY_NET,
    PLAY_LAST
};

enum tile {
    TILE_NONE = 0,
    TILE_RED_CHECKER,
    TILE_YLW_CHECKER
};

struct game {
    enum tile (*board)[];
    uint8_t (*blink)[];
    uint8_t cur_player;
    int width, height;
};

typedef int32_t move_t;

struct position {
    int x, y;
};

void start_game(int width, int height, enum play_mode, void (*on_redraw)(WINDOW *, void *ctx), void *ctx);

#endif /* _GAME_H_ */
