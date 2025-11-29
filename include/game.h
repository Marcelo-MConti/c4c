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

void start_game(int width, int height, enum play_mode, void (*on_redraw)(WINDOW *, void *ctx), void *ctx);

#endif /* _GAME_H_ */
