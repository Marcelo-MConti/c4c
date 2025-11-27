#ifndef _GAME_LOCAL_H_
#define _GAME_LOCAL_H_

#include <curses.h>

#include "game.h"

struct position *local_play(WINDOW *win, struct game *game, void (*on_redraw)(WINDOW *, void *ctx), void *ctx);

#endif /* _GAME_LOCAL_H_ */
