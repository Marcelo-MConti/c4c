#ifndef _CHARS_H_
#define _CHARS_H_

#include "game/common.h"

/*
 O arquivo define os caracteres usados para exibir elementos gráficos
 do jogo. A escolha entre símbolos ASCII ou Unicode
 
  arrow:
    - Representa a seta indicadora do jogador atual.
    - ASCII: "\/"
    - Unicode: "🭥🭚"
 
  checkers[]:
    - Vetor de strings que representa cada tipo de peça no tabuleiro.
    - Indexado por constantes como TILE_NONE, TILE_RED_CHECKER e TILE_YLW_CHECKER.
    - ASCII:
 *       TILE_NONE -> " "
 *       TILE_RED_CHECKER -> "#"
 *       TILE_YLW_CHECKER -> "O"
    - Unicode:
 *       TILE_NONE -> " "
 *       TILE_RED_CHECKER -> "◉"
 *       TILE_YLW_CHECKER -> "◉"
 */

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
