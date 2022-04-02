#ifndef _GAME_H_
#define _GAME_H_

#include <ncurses.h>

enum playmode {
    PLAY_PLPL,
    PLAY_PLPC,
    PLAY_PCPC,
    PLAY_NET
};

struct game {
    unsigned char **board;
    struct move *(*make_move[2])(WINDOW *, struct game *);
    unsigned char cur_player;
    unsigned int x, y;
};

struct move {
    int x, y;
};

enum position {
    NONE = 0,
    RED_CHECKER,
    YLW_CHECKER
};

void start_game(int x, int y, enum playmode);

#endif /* _GAME_H_ */
