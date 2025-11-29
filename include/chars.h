#ifndef _CHARS_H_
#define _CHARS_H_

#include "game.h"

#ifdef C4C_ASCII
const static char *arrow = "\\/";
const static char *checkers[] = {
    [TILE_NONE] = " ", [TILE_RED_CHECKER] = "#", [TILE_YLW_CHECKER] = "O"
};
#else
const static char *arrow = u8"🭥🭚";
const static char *checkers[] = {
    [TILE_NONE] = " ", [TILE_RED_CHECKER] = u8"◉", [TILE_YLW_CHECKER] = u8"◉"
};
#endif

#endif
