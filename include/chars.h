#ifndef _CHARS_H_
#define _CHARS_H_

#include "game.h"

#ifdef C4C_ASCII
const static char *arrow = "\\/";
const static char *checkers[] = {
    [NONE] = " ", "#", "O"
};
#else
const static char *arrow = u8"🭥🭚";
const static char *checkers[] = {
    [NONE] = " ", u8"◉", u8"◉"
};
#endif

#endif
