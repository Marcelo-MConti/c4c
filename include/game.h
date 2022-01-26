#ifndef _GAME_H_
#define _GAME_H_

#include <ncurses.h>

enum playmode {
    PLAY_PLPL,
    PLAY_PLPC,
    PLAY_PCPC,
    PLAY_NET
};

struct game_info {
    short play_mode;
    int x, y;
};

struct game {
    unsigned char **board;
    struct move *(*make_move[2])(WINDOW *, struct game *, struct game_info *);
    unsigned char cur_player;
};

struct move {
    int x, y;
};

enum position {
    NONE,
    RED_CHECKER,
    YLW_CHECKER
};

void start_game(struct game_info *info);

#endif /* _GAME_H_ */
