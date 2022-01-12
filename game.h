#ifndef _GAME_H_
#define _GAME_H_

enum playmode {
    PLAY_PLPL,
    PLAY_PLPC,
    PLAY_PCPC,
};

struct gameinfo {
    short playmode;
    int width, height;
};

struct game {
    char **board;
    int (*playfunc[2])(char **board);
    unsigned char curplayer;
};

enum position {
    NONE,
    RED_CHECKER,
    YLW_CHECKER
};


void startgame(struct gameinfo *info);

#endif /* _GAME_H_ */
