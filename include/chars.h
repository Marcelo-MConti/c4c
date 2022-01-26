#ifndef _CHARS_H_
#define _CHARS_H_

#include "game.h"

#ifdef C4C_ASCII
const static char *arrow = "\\/";

const static char *checkers[] = {
    [NONE] = " ", "#", "O"
};
#else
/* Right block diagonal (U+1FB65) + Left block diagonal (U+1FB5A) */
const static char *arrow = "\360\237\255\245\360\237\255\232";

/* Fisheye character */
const static char *checkers[] = {
    [NONE] = " ", "\342\227\211", "\342\227\211"
};
#endif

#endif
