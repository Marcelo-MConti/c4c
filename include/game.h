#ifndef _GAME_H_
#define _GAME_H_

enum playmode {
    PLAY_PLPL,
    PLAY_PLPC,
    PLAY_PCPC,
    PLAY_NET
};

struct game_info {
    short play_mode;
    int width, height;
};

struct game {
    char **board;
    int (*play_func[2])(char **board);
    unsigned char cur_player;
};

enum position {
    NONE,
    RED_CHECKER,
    YLW_CHECKER
};


void start_game(struct game_info *info);

#endif /* _GAME_H_ */
